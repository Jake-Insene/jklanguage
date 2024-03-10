#include "jkr/CodeFile/OpCodes.h"
#include "jkr/CodeFile/Type.h"
#include "jkr/NI/ThreadState.h"
#include "jkr/Runtime/VirtualMachine.h"
#include <stdjk/Debug.h>
#include <stdjk/Mem/Allocator.h>
#include <stdio.h>

#include <unordered_map>

thread_local runtime::Value Registers[16] = {};
thread_local UInt CMPFlags = {};

#define ZERO_BIT 0x1
#define EQUAL_BIT 0x2
#define LESS_BIT 0x4
#define GREATER_BIT 0x8
#define CARRY_BIT 0x10
#define SIGN_BIT 0x20

#define DISPATCH(OP) case codefile::OpCode::##OP:
#define BREAK() break;

#define CHECK_ARRAY_INDEX(IndexExpr) \
    if((IndexExpr) >= array->Size) {\
        RuntimeError(false, STR("Index out of range"));\
    }

namespace runtime {

static constexpr void Push(StackFrame& Frame, Value Arg) {
    *Frame.SP = Arg;
    Frame.SP++;
}

static constexpr Value Pop(StackFrame& Frame) {
    return *--Frame.SP;
}

VirtualMachine* VirtualMachine::New(USize StackSize, UInt LocalSize, Assembly* Asm) {
    VirtualMachine* vm = Cast<VirtualMachine*>(
        mem::Allocate(sizeof(VirtualMachine))
    );

    vm->VMStack = Stack::New(StackSize);
    vm->Locals = LocalStack::New(LocalSize);
    vm->Asm = Asm;
    vm->Libraries = List<Library>::New(0);
    vm->LinkageResolved = false;

    return vm;
}

Value VirtualMachine::ExecMain(this VirtualMachine& Self) {
    if (Self.Asm->Err != AsmOk) return {};
    if (!Self.LinkageResolved) {
        Self.Err = VMLinkageError;
        return {};
    }
    Function* fn = &Self.Asm->CodeSection->Functions[Self.Asm->EntryPoint];

    StackFrame frame = {
        .SP = Self.VMStack.Start,
        .FP = Self.Locals.Data,
        .DataElements = Self.Asm->DataSection ? Self.Asm->DataSection->Data.Data : nullptr,
    };
    return Self.MainLoop(fn, frame);
}

void VirtualMachine::ResolveExtern(this VirtualMachine& Self) {
    for (auto& fn : Self.Asm->CodeSection->Functions) {
        if (fn.Flags & codefile::FunctionNative) {
            Library lib = {};
            if (!Self.TryLoad(Self.Asm->STSection->Strings[fn.SizeOfCode], lib)) {
                Self.Err = VMLinkageError;
                return;
            }

            Str entry = Cast<Str>(Self.Asm->STSection->Strings[fn.LocalReserve].Items);
            fn.Native = (Value(*)(ThreadState*, ...))lib.Get(entry);
            if (!fn.Native) {
                Self.Err = VMLinkageError;
                return;
            }
        }
    }

    Self.LinkageResolved = true;
}

bool VirtualMachine::TryLoad(this VirtualMachine& Self, Array& LibName, Library& Lib) {
    for (auto& lib : Self.Libraries) {
        USize len = Strlen(lib.FilePath);
        if (len != LibName.Size) {
            continue;
        }

        if (mem::Cmp((void*)lib.FilePath, LibName.Items, len)) {
            Lib = lib;
            return true;
        }
    }
    
    Char buff[1024] = {};
    if constexpr (Platform == Windows) {
        sprintf_s((char*)buff, 1024, "%s.dll", (char*)LibName.Items);
    }
    else {
        sprintf((char*)buff, "lib%s.so", (char*)LibName.Items);
    }

    Lib = Self.Libraries.Push((Char*)buff);
    if (!Lib.Handle)
        return false;

    return true;
}

Value VirtualMachine::ProcessCall(this VirtualMachine& Self, StackFrame& Frame, Function* Fn) {
    RuntimeError(Fn != nullptr,
                 STR("Invalid function")
    );

    if (Fn->Native) {
        ThreadState state = {
            .VM = &Self,
        };

        return Fn->Native(
            &state,
            Registers[1], Registers[2], Registers[3], Registers[4], Registers[5],
            Registers[6], Registers[7], Registers[8], Registers[9], Registers[10]
        );
    }

    StackFrame newFrame = {
        .SP = Frame.SP,
        .FP = Frame.FP + Fn->LocalReserve,
        .DataElements = Frame.DataElements,
    };

    return Self.MainLoop(Fn, newFrame);
}

Value VirtualMachine::MainLoop(this VirtualMachine& Self, Function* Fn, StackFrame& Frame) {
    RuntimeError(Fn != nullptr,
                 STR("Invalid function")
    );

    Byte* ip = &Fn->Code[0];

    Value constant = {};
    Byte reg = 0;
    Byte reg2 = 0;
    UInt qword = {};
    Array* array = nullptr;

    while (true) {
        codefile::OpCode opcode = IntCast<codefile::OpCode>(*ip++);
        switch (opcode) {
            DISPATCH(Brk) {
                debug::Break();
                BREAK();
            }
            DISPATCH(Mov) {
                reg = *ip++;
                Registers[reg & 0xF] = Registers[Byte(reg >> 4)];
                BREAK();
            }
            DISPATCH(Mov4) {
                reg = *ip++;
                Registers[reg & 0xF].Unsigned = Byte(reg >> 4);
                BREAK();
            }
            DISPATCH(Mov8) {
                reg = *ip++;
                Registers[reg].Unsigned = *ip++;
                BREAK();
            }
            DISPATCH(Mov16) {
                reg = *ip++;
                Registers[reg].Unsigned = *(UInt16*)ip;
                ip += 2;
                BREAK();
            }
            DISPATCH(Mov32) {
                reg = *ip++;
                Registers[reg].Unsigned = *(UInt32*)ip;
                ip += 4;
                BREAK();
            }
            DISPATCH(Mov64) {
                reg = (*ip++);
                Registers[reg].Unsigned = *(UInt64*)ip;
                ip += 8;
                BREAK();
            }
            DISPATCH(MovRes) {
                Registers[(*ip++) & 0xF] = Frame.Result;
                BREAK();
            }
            DISPATCH(LocalSet4) {
                reg = (*ip++);
                Frame.FP[reg >> 4] = Registers[reg & 0xF];
                BREAK();
            }
            DISPATCH(LocalGet4) {
                reg = (*ip++);
                Registers[reg & 0xF] = Frame.FP[reg >> 4];
                BREAK();
            }
            DISPATCH(LocalSet) {
                reg = (*ip++);
                qword = (*ip++);
                Frame.FP[qword] = Registers[reg];
                BREAK();
            }
            DISPATCH(LocalGet) {
                reg = (*ip++);
                qword = (*ip++);
                Registers[reg] = Frame.FP[qword];
                BREAK();
            }
            DISPATCH(GlobalSet) {
                qword = *(UInt32*)ip;
                ip += 4;
                Frame.DataElements[qword >> 4].Contant = Registers[qword & 0xF];
                BREAK();
            }
            DISPATCH(GlobalGet) {
                qword = *(UInt32*)ip;
                ip += 4;
                Registers[qword & 0xF] = Frame.DataElements[qword >> 4].Contant;
                BREAK();
            }
            DISPATCH(Inc) {
                reg = (*ip++);
                reg2 = Byte(reg >> 4);
                if (reg == 0) {
                    Registers[reg & 0xF].Signed++;
                }
                else if (reg == 1) {
                    Registers[reg & 0xF].Unsigned++;
                }
                else if (reg == 2) {
                    Registers[reg & 0xF].Real++;
                }
            }
            BREAK();
            DISPATCH(Dec) {
                reg = (*ip++);
                reg2 = Byte(reg >> 4);
                if (reg == 0) {
                    Registers[reg & 0xF].Signed--;
                }
                else if (reg == 1) {
                    Registers[reg & 0xF].Unsigned--;
                }
                else if (reg == 2) {
                    Registers[reg & 0xF].Real--;
                }
            }
            BREAK();
            DISPATCH(Add) {
                reg = (*ip++);
                reg2 = (*ip++);
                Registers[reg & 0xF].Unsigned = Registers[(reg >> 4)].Unsigned + Registers[reg2 & 0xF].Unsigned;
            }
            BREAK();
            DISPATCH(IAdd) {
                reg = (*ip++);
                reg2 = (*ip++);
                Registers[reg & 0xF].Signed = Registers[(reg >> 4)].Signed + Registers[reg2 & 0xF].Signed;
            }
            BREAK();
            DISPATCH(FAdd) {
                reg = (*ip++);
                reg2 = (*ip++);
                Registers[reg & 0xF].Real = Registers[(reg >> 4)].Real + Registers[reg2 & 0xF].Real;
            }
            BREAK();
            DISPATCH(Sub) {
                reg = (*ip++);
                reg2 = (*ip++);
                Registers[reg & 0xF].Unsigned = Registers[(reg >> 4)].Unsigned - Registers[reg2 & 0xF].Unsigned;
            }
            BREAK();
            DISPATCH(ISub) {
                reg = (*ip++);
                reg2 = (*ip++);
                Registers[reg & 0xF].Signed = Registers[(reg >> 4)].Signed - Registers[reg2 & 0xF].Signed;
            }
            BREAK();
            DISPATCH(FSub) {
                reg = (*ip++);
                reg2 = (*ip++);
                Registers[reg & 0xF].Real = Registers[(reg >> 4)].Real - Registers[reg2 & 0xF].Real;
            }
            BREAK();
            DISPATCH(Mul) {
                reg = (*ip++);
                reg2 = (*ip++);
                Registers[reg & 0xF].Unsigned = Registers[(reg >> 4)].Unsigned * Registers[reg2 & 0xF].Unsigned;
            }
            BREAK();
            DISPATCH(IMul) {
                reg = (*ip++);
                reg2 = (*ip++);
                Registers[reg & 0xF].Signed = Registers[(reg >> 4)].Signed * Registers[reg2 & 0xF].Signed;
            }
            BREAK();
            DISPATCH(FMul) {
                reg = (*ip++);
                reg2 = (*ip++);
                Registers[reg & 0xF].Real = Registers[(reg >> 4)].Real * Registers[reg2 & 0xF].Real;
            }
            BREAK();
            DISPATCH(Div) {
                reg = (*ip++);
                reg2 = (*ip++);
                Registers[reg & 0xF].Unsigned = Registers[(reg >> 4)].Unsigned / Registers[reg2 & 0xF].Unsigned;
            }
            BREAK();
            DISPATCH(IDiv) {
                reg = (*ip++);
                reg2 = (*ip++);
                Registers[reg & 0xF].Signed = Registers[(reg >> 4)].Signed / Registers[reg2 & 0xF].Signed;
            }
            BREAK();
            DISPATCH(FDiv) {
                reg = (*ip++);
                reg2 = (*ip++);
                Registers[reg & 0xF].Real = Registers[(reg >> 4)].Real / Registers[reg2 & 0xF].Real;
            }
            BREAK();
            DISPATCH(Add8) {
                reg = (*ip++);
                constant.Unsigned = (*ip++);
                Registers[reg & 0xF].Unsigned = Registers[(reg > 4)].Unsigned + constant.Unsigned;
            }
            BREAK();
            DISPATCH(IAdd8) {
                reg = (*ip++);
                constant.Unsigned = (*ip++);
                Registers[reg & 0xF].Signed = Registers[(reg > 4)].Signed + constant.Signed;
            }
            BREAK();
            DISPATCH(Sub8) {
                reg = (*ip++);
                constant.Unsigned = (*ip++);
                Registers[reg & 0xF].Unsigned = Registers[(reg > 4)].Unsigned - constant.Unsigned;
            }
            BREAK();
            DISPATCH(ISub8) {
                reg = (*ip++);
                constant.Unsigned = (*ip++);
                Registers[reg & 0xF].Signed = Registers[(reg > 4)].Signed - constant.Signed;
            }
            BREAK();
            DISPATCH(Mul8) {
                reg = (*ip++);
                constant.Unsigned = (*ip++);
                Registers[reg & 0xF].Unsigned = Registers[(reg > 4)].Unsigned * constant.Unsigned;
            }
            BREAK();
            DISPATCH(IMul8) {
                reg = (*ip++);
                constant.Unsigned = (*ip++);
                Registers[reg & 0xF].Signed = Registers[(reg > 4)].Signed * constant.Signed;
            }
            BREAK();
            DISPATCH(Div8) {
                reg = (*ip++);
                constant.Unsigned = (*ip++);
                Registers[reg & 0xF].Unsigned = Registers[(reg > 4)].Unsigned / constant.Unsigned;
            }
            BREAK();
            DISPATCH(IDiv8) {
                reg = (*ip++);
                constant.Unsigned = (*ip++);
                Registers[reg & 0xF].Signed = Registers[(reg > 4)].Signed / constant.Signed;
            }
            BREAK();
            DISPATCH(Add16) {
                reg = (*ip++);
                constant.Unsigned = *(UInt16*)ip;
                Registers[reg & 0xF].Unsigned = Registers[(reg >> 4)].Unsigned + constant.Unsigned;
                ip += 2;
            }
            BREAK();
            DISPATCH(IAdd16) {
                reg = (*ip++);
                constant.Unsigned = *(UInt16*)ip;
                ip += 2;
                Registers[reg & 0xF].Signed = Registers[(reg >> 4)].Signed + constant.Signed;
            }
            BREAK();
            DISPATCH(Sub16) {
                reg = (*ip++);
                constant.Unsigned = *(UInt16*)ip;
                ip += 2;
                Registers[reg & 0xF].Unsigned = Registers[(reg >> 4)].Unsigned - constant.Unsigned;
            }
            BREAK();
            DISPATCH(ISub16) {
                reg = (*ip++);
                constant.Unsigned = *(UInt16*)ip;
                ip += 2;
                Registers[reg & 0xF].Signed = Registers[(reg >> 4)].Signed - constant.Signed;
            }
            BREAK();
            DISPATCH(Mul16) {
                reg = (*ip++);
                constant.Unsigned = *(UInt16*)ip;
                ip += 2;
                Registers[reg & 0xF].Unsigned = Registers[(reg >> 4)].Unsigned * constant.Unsigned;
            }
            BREAK();
            DISPATCH(IMul16) {
                reg = (*ip++);
                constant.Unsigned = *(UInt16*)ip;
                ip += 2;
                Registers[reg & 0xF].Signed = Registers[(reg >> 4)].Signed * constant.Signed;
            }
            BREAK();
            DISPATCH(Div16) {
                reg = (*ip++);
                constant.Unsigned = *(UInt16*)ip;
                ip += 2;
                Registers[reg & 0xF].Unsigned = Registers[(reg >> 4)].Unsigned / constant.Unsigned;
            }
            BREAK();
            DISPATCH(IDiv16) {
                reg = (*ip++);
                constant.Unsigned = *(UInt16*)ip;
                ip += 2;
                Registers[reg & 0xF].Signed = Registers[(reg >> 4)].Signed / constant.Signed;
            }
            BREAK();
            DISPATCH(Cmp) {
                reg = (*ip++);
                CMPFlags = 0;
                if (Registers[reg & 0xF].Unsigned == Registers[(reg >> 4)].Unsigned) {
                    if (Registers[(reg >> 4)].Unsigned == 0) {
                        CMPFlags |= ZERO_BIT;
                    }
                    CMPFlags |= EQUAL_BIT;
                }
                if (Registers[reg & 0xF].Unsigned < Registers[(reg >> 4)].Unsigned) {
                    CMPFlags |= LESS_BIT;
                }
                if (Registers[reg & 0xF].Unsigned > Registers[(reg >> 4)].Unsigned) {
                    CMPFlags |= GREATER_BIT;
                }
            }
            BREAK();
            DISPATCH(ICmp) {
                reg = (*ip++);
                CMPFlags = 0;
                if (Registers[reg & 0xF].Signed == Registers[(reg >> 4)].Signed) {
                    if (Registers[(reg >> 4)].Signed == 0) {
                        CMPFlags |= ZERO_BIT;
                    }
                    CMPFlags |= EQUAL_BIT;
                }
                if (Registers[reg & 0xF].Signed < Registers[(reg >> 4)].Signed) {
                    CMPFlags |= LESS_BIT;
                }
                if (Registers[reg & 0xF].Signed > Registers[(reg >> 4)].Signed) {
                    CMPFlags |= GREATER_BIT;
                }
            }
            BREAK();
            DISPATCH(FCmp) {
                reg = (*ip++);
                CMPFlags = 0;
                if (Registers[reg & 0xF].Real == Registers[(reg >> 4)].Real) {
                    if (Registers[(reg >> 4)].Unsigned == 0) {
                        CMPFlags |= ZERO_BIT;
                    }
                    CMPFlags |= EQUAL_BIT;
                }
                if (Registers[reg & 0xF].Real < Registers[(reg >> 4)].Real) {
                    CMPFlags |= LESS_BIT;
                }
                if (Registers[reg & 0xF].Real > Registers[(reg >> 4)].Real) {
                    CMPFlags |= GREATER_BIT;
                }
            }
            BREAK();
            DISPATCH(TestZ) {
                reg = *ip++;
                CMPFlags = 0;
                if (Registers[reg & 0xF].Unsigned == 0) {
                    CMPFlags |= EQUAL_BIT;
                    CMPFlags |= ZERO_BIT;
                }
            }
            BREAK();
            DISPATCH(Jmp) {
                ip += *(UInt16*)ip;
            }
            BREAK();
            DISPATCH(Jez) {
                if (CMPFlags & ZERO_BIT && CMPFlags & EQUAL_BIT)
                    ip += *(UInt16*)ip;
                else
                    ip += 2;
            }
            BREAK();
            DISPATCH(Jnz) {
                if (CMPFlags & ZERO_BIT && !(CMPFlags & EQUAL_BIT))
                    ip += *(UInt16*)ip;
                else
                    ip += 2;
            }
            BREAK();
            DISPATCH(Je) {
                if (CMPFlags & EQUAL_BIT)
                    ip += *(UInt16*)ip;
                else
                    ip += 2;
                BREAK();
            }
            DISPATCH(Jne) {
                if (!(CMPFlags & EQUAL_BIT))
                    ip += *(UInt16*)ip;
                else
                    ip += 2;
            }
            BREAK();
            DISPATCH(Jl) {
                if (CMPFlags & LESS_BIT)
                    ip += *(UInt16*)ip;
                else
                    ip += 2;
            }
            BREAK();
            DISPATCH(Jle) {
                if (CMPFlags & LESS_BIT && CMPFlags & EQUAL_BIT)
                    ip += *(UInt16*)ip;
                else
                    ip += 2;
            }
            BREAK();
            DISPATCH(Jg) {
                if (CMPFlags & GREATER_BIT)
                    ip += *(UInt16*)ip;
                else
                    ip += 2;
            }
            BREAK();
            DISPATCH(Jge) {
                if (CMPFlags & GREATER_BIT && CMPFlags & EQUAL_BIT)
                    ip += *(UInt16*)ip;
                else
                    ip += 2;
            }
            BREAK();
            DISPATCH(Jmp8) {
                ip += *ip;
            }
            BREAK();
            DISPATCH(Jez8) {
                if (CMPFlags & ZERO_BIT && CMPFlags & EQUAL_BIT)
                    ip += *ip;
                else
                    ip++;
            }
            BREAK();
            DISPATCH(Jnz8) {
                if (CMPFlags & ZERO_BIT && !(CMPFlags & EQUAL_BIT))
                    ip += *ip;
                else
                    ip++;
            }
            BREAK();
            DISPATCH(Je8) {
                if (CMPFlags & EQUAL_BIT)
                    ip += *ip;
                else
                    ip++;
            }
            BREAK();
            DISPATCH(Jne8) {
                if (!(CMPFlags & EQUAL_BIT))
                    ip += *ip;
                else
                    ip++;
            }
            BREAK();
            DISPATCH(Jl8) {
                if (CMPFlags & LESS_BIT)
                    ip += *ip;
                else
                    ip++;
            }
            BREAK();
            DISPATCH(Jle8) {
                if (CMPFlags & LESS_BIT && CMPFlags & EQUAL_BIT)
                    ip += *ip;
                else
                    ip++;
            }
            BREAK();
            DISPATCH(Jg8) {
                if (CMPFlags & GREATER_BIT)
                    ip += *ip;
                else
                    ip++;
            }
            BREAK();
            DISPATCH(Jge8) {
                if (CMPFlags & GREATER_BIT && CMPFlags & EQUAL_BIT)
                    ip += *ip;
                else
                    ip++;
            }
            BREAK();
            DISPATCH(Call8) {
                qword = *ip++;
                Function* called = &Fn->Asm->CodeSection->Functions[qword];
                Frame.Result = Self.ProcessCall(Frame, called);
            }
            BREAK();
            DISPATCH(Call) {
                qword = *(UInt32*)ip;
                ip += 4;
                Function* called = &Fn->Asm->CodeSection->Functions[qword];
                Frame.Result = Self.ProcessCall(Frame, called);
            }
            BREAK();
            DISPATCH(Ret) {
                return Registers[0];
            }
            BREAK();
            DISPATCH(ObjectNew) {

            }
            BREAK();
            DISPATCH(FieldSet4) {

            }
            BREAK();
            DISPATCH(FieldGet4) {

            }
            BREAK();
            DISPATCH(FieldSet) {

            }
            BREAK();
            DISPATCH(FieldGet) {

            }
            BREAK();
            DISPATCH(ObjectDestroy) {

            }
            BREAK();
            DISPATCH(ArrayNew) {
                reg = *ip++; // Dest
                reg2 = *ip++;
                array = Cast<Array*>(
                    mem::Allocate(sizeof(Array))
                );
                array->ItemType = codefile::PrimitiveType(reg2);

                if (reg2 == codefile::PrimitiveByte) {
                    *array = Array::New(Registers[Byte(reg >> 4)].Unsigned, 1);
                    Registers[reg & 0xF].ArrayRef = array;
                }
                else if (reg2 >= codefile::PrimitiveInt
                         || reg <= codefile::PrimitiveAny) {
                    *array = Array::New(Registers[Byte(reg >> 4)].Unsigned, 8);
                    Registers[reg & 0xF].ArrayRef = array;
                }
            }
            BREAK();
            DISPATCH(ArrayObjectNew) {

            }
            BREAK();
            DISPATCH(ArrayLen) {
                reg = *ip++;
                array = Registers[reg & 0xF].ArrayRef;
                Registers[reg & 0xF].Unsigned = array->Size;
            }
            BREAK();
            DISPATCH(ArraySet) {
                reg = *ip++;
                reg2 = *ip++;
                array = Registers[reg & 0xF].ArrayRef;
                CHECK_ARRAY_INDEX(Registers[reg >> 4].Unsigned);

                if (array->ItemType == codefile::PrimitiveByte) {
                    ((Byte*)array->Items)[Registers[reg >> 4].Unsigned] = Byte(Registers[reg2 & 0xF].Unsigned);
                }
                else if (array->ItemType >= codefile::PrimitiveInt
                         || reg <= codefile::PrimitiveAny) {
                    ((UInt*)array->Items)[Registers[reg >> 4].Unsigned] = Registers[reg2 & 0xF].Unsigned;
                }
            }
            BREAK();
            DISPATCH(ArrayGet) {
                reg = *ip++;
                reg2 = *ip++;
                array = Registers[reg & 0xF].ArrayRef;
                CHECK_ARRAY_INDEX(Registers[reg >> 4].Unsigned);

                if (array->ItemType == codefile::PrimitiveByte) {
                    Registers[reg2 & 0xF].Unsigned = ((Byte*)array->Items)[Registers[reg >> 4].Unsigned];
                }
                else if (array->ItemType >= codefile::PrimitiveInt
                         || reg <= codefile::PrimitiveAny) {
                    Registers[reg2 & 0xF].Unsigned = ((UInt*)array->Items)[Registers[reg >> 4].Unsigned];
                }
            }
            BREAK();
            DISPATCH(ArrayDestroy) {
                reg = *ip++;
                array = Registers[reg & 0xF].ArrayRef;
                array->Destroy();
                mem::Deallocate(array);
            }
            BREAK();
            DISPATCH(StringGet4) {
                reg = (*ip++);
                Registers[reg & 0xF].ArrayRef = &Fn->Asm->STSection->Strings[Byte(reg >> 4)];
            }
            BREAK();
            DISPATCH(StringGet) {
                qword = *(UInt32*)ip;
                ip += 4;
                Registers[(qword & 0xF)].ArrayRef = &Fn->Asm->STSection->Strings[(qword >> 4)];
            }
            BREAK();
            DISPATCH(Push8) {
                constant.Unsigned = (*ip++);
                Push(Frame, constant);
            }
            BREAK();
            DISPATCH(Push16) {
                constant.Unsigned = *(UInt16*)ip;
                ip += 2;
                Push(Frame, constant);
            }
            BREAK();
            DISPATCH(Push32) {
                constant.Unsigned = *(UInt32*)ip;
                ip += 4;
                Push(Frame, constant);
            }
            BREAK();
            DISPATCH(Push64) {
                constant.Unsigned = *(UInt64*)ip;
                ip += 8;
                Push(Frame, constant);
            }
            BREAK();
            DISPATCH(PopTop)
                (void)Pop(Frame);
            BREAK();
            DISPATCH(PushR0)
            DISPATCH(PushR1)
            DISPATCH(PushR2)
            DISPATCH(PushR3)
            DISPATCH(PushR4)
            DISPATCH(PushR5)
            DISPATCH(PushR6)
            DISPATCH(PushR7)
            DISPATCH(PushR8)
            DISPATCH(PushR9)
            DISPATCH(PushR10)
            DISPATCH(PushR11)
            DISPATCH(PushR12)
            DISPATCH(PushR13)
            DISPATCH(PushR14)
            DISPATCH(PushR15) {
                reg = Byte(opcode);
                Push(
                    Frame,
                    Registers[reg & 0xF]
                );
            }
            BREAK();
            DISPATCH(PopR0)
            DISPATCH(PopR1)
            DISPATCH(PopR2)
            DISPATCH(PopR3)
            DISPATCH(PopR4)
            DISPATCH(PopR5)
            DISPATCH(PopR6)
            DISPATCH(PopR7)
            DISPATCH(PopR8)
            DISPATCH(PopR9)
            DISPATCH(PopR10)
            DISPATCH(PopR11)
            DISPATCH(PopR12)
            DISPATCH(PopR13)
            DISPATCH(PopR14)
            DISPATCH(PopR15) {
                reg = Byte(opcode);
                Registers[reg & 0xF] = Pop(Frame);
            }
            BREAK();
            DISPATCH(PushLocal) {
                reg = *ip++;
                Push(Frame, Frame.FP[reg]);
            }
            BREAK();
            DISPATCH(PopLocal) {
                reg = *ip++;
                Frame.FP[reg] = Pop(Frame);
            }
            BREAK();
        default:
            RuntimeError(0, STR("Invalid OpCode 0x{xb:0}"), (Byte)opcode);
            BREAK();
        }
    }
}

void VirtualMachine::Destroy(this VirtualMachine& Self) {
    Self.Libraries.Destroy();
    mem::Deallocate(&Self);
}

}
