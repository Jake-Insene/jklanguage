#include "jkr/Runtime/ThreadExec.h"
#include "jkr/Runtime/VirtualMachine.h"
#include "jkr/NI/ThreadState.h"
#include <stdjk/Debug.h>
#include <stdjk/Error.h>

thread_local runtime::Value Registers[16] = {};
thread_local runtime::Value CMPFlags = {};

#define ZERO_BIT 0x1
#define EQUAL_BIT 0x2
#define LESS_BIT 0x4
#define GREATER_BIT 0x8

#define DISPATCH(OP) case codefile::OpCode::##OP:
#define BREAK() break;

namespace runtime {

static constexpr void Push(StackFrame& Frame, Value Val) {
    *Frame.SP = Val;
    Frame.SP++;
}

static constexpr Value Pop(StackFrame& Frame) {
    return *--Frame.SP;
}

ThreadExecution runtime::ThreadExecution::New(VirtualMachine* VM) {
    return ThreadExecution{
        .VM = VM,
    };
}

Value ThreadExecution::Exec(this ThreadExecution& Self, Function* Fn, Value* SP) {
    RuntimeError(Fn != nullptr,
                 STR("Invalid function")
    );

    UInt ip = 0;

    StackFrame frame = {};
    frame.FP = SP - Fn->NonNative.Arguments;
    frame.SP = SP + Fn->NonNative.LocalCount;
    frame.Globals = Fn->Asm->Globals.Data;

    Value constant = {};
    Byte reg = 0;
    Byte reg2 = 0;
    codefile::LocalType local = 0;
    codefile::GlobalType global = 0;
    codefile::StringType string = 0;

    while (true) {
        codefile::OpCode opcode = IntCast<codefile::OpCode>(Fn->Code[ip++]);
        switch (opcode) {
            DISPATCH(Brk) {
                debug::Break();
                BREAK();
            }
            DISPATCH(Mov) {
                reg = Fn->Code[ip++];
                Registers[reg & 0xF] = Registers[Byte(reg >> 4)];
                BREAK();
            }
            DISPATCH(Mov4) {
                reg = Fn->Code[ip++];
                Registers[reg & 0xF].Unsigned = Byte(reg >> 4);
                BREAK();
            }
            DISPATCH(Mov8) {
                reg = Fn->Code[ip++];
                Registers[reg].Unsigned = Fn->Code[ip++];
                BREAK();
            }
            DISPATCH(Mov16) {
                reg = Fn->Code[ip++];
                Registers[reg].Unsigned = *(UInt16*)(Fn->Code.Data + ip);
                ip += 2;
                BREAK();
            }
            DISPATCH(Mov32) {
                reg = Fn->Code[ip++];
                Registers[reg].Unsigned = *(UInt32*)(Fn->Code.Data + ip);
                ip += 4;
                BREAK();
            }
            DISPATCH(Mov64) {
                reg = Fn->Code[ip++];
                Registers[reg].Unsigned = *(UInt64*)(Fn->Code.Data + ip);
                ip += 8;
                BREAK();
            }
            DISPATCH(MovRes) {
                Registers[Fn->Code[ip++] & 0xF] = frame.Result;
                BREAK();
            }
            DISPATCH(LocalSet4) {
                reg = Fn->Code[ip++];
                frame.FP[reg >> 4] = Registers[reg & 0xF];
                BREAK();
            }
            DISPATCH(LocalGet4) {
                reg = Fn->Code[ip++];
                Registers[reg & 0xF] = frame.FP[reg >> 4];
                BREAK();
            }
            DISPATCH(LocalSet) {
                reg = Fn->Code[ip++];
                local = Fn->Code[ip++];
                frame.FP[local] = Registers[reg];
                BREAK();
            }
            DISPATCH(LocalGet) {
                reg = Fn->Code[ip++];
                local = Fn->Code[ip++];
                Registers[reg] = frame.FP[local];
                BREAK();
            }
            DISPATCH(GlobalSet) {
                reg = Fn->Code[ip++];
                global = *(codefile::GlobalType*)(Fn->Code.Data + ip);
                ip += sizeof(codefile::GlobalType);
                frame.Globals[global].Contant = Registers[reg];
                BREAK();
            }
            DISPATCH(GlobalGet) {
                reg = Fn->Code[ip++];
                global = *(codefile::GlobalType*)(Fn->Code.Data + ip);
                ip += sizeof(codefile::GlobalType);
                Registers[reg] = frame.Globals[global].Contant;
                BREAK();
            }
            DISPATCH(Inc) {
                reg = Fn->Code[ip++];
                Registers[reg & 0xF].Unsigned++;
                BREAK();
            }
            DISPATCH(IInc) {
                reg = Fn->Code[ip++];
                Registers[reg&0xF].Signed++;
                BREAK();
            }
            DISPATCH(FInc) {
                reg = Fn->Code[ip++];
                Registers[reg & 0xF].Real++;
                BREAK();
            }
            DISPATCH(Dec) {
                reg = Fn->Code[ip++];
                Registers[reg & 0xF].Unsigned--;
                BREAK();
            }
            DISPATCH(IDec) {
                reg = Fn->Code[ip++];
                Registers[reg & 0xF].Signed--;
                BREAK();
            }
            DISPATCH(FDec) {
                reg = Fn->Code[ip++];
                Registers[reg & 0xF].Real--;
                BREAK();
            }
            DISPATCH(Add) {
                reg = Fn->Code[ip++];
                reg2 = Fn->Code[ip++];
                ip += sizeof(codefile::CIL);
                Registers[reg & 0xF].Unsigned = Registers[(reg >> 4)].Unsigned + Registers[reg2&0xF].Unsigned;
                BREAK();
            }
            DISPATCH(IAdd) {
                reg = Fn->Code[ip++];
                reg2 = Fn->Code[ip++];
                Registers[reg & 0xF].Signed = Registers[(reg >> 4)].Signed + Registers[reg2&0xF].Signed;
                BREAK();
            }
            DISPATCH(FAdd) {
                reg = Fn->Code[ip++];
                reg2 = Fn->Code[ip++];
                Registers[reg & 0xF].Real = Registers[(reg >> 4)].Real + Registers[reg2&0xF].Real;
                BREAK();
            }
            DISPATCH(Sub) {
                reg = Fn->Code[ip++];
                reg2 = Fn->Code[ip++];
                Registers[reg & 0xF].Unsigned = Registers[(reg >> 4)].Unsigned - Registers[reg2&0xF].Unsigned;
            }
            BREAK();
            DISPATCH(ISub) {
                reg = Fn->Code[ip++];
                reg2 = Fn->Code[ip++];
                Registers[reg & 0xF].Signed = Registers[(reg >> 4)].Signed - Registers[reg2&0xF].Signed;
            }
            BREAK();
            DISPATCH(FSub) {
                reg = Fn->Code[ip++];
                reg2 = Fn->Code[ip++];
                Registers[reg & 0xF].Real = Registers[(reg >> 4)].Real - Registers[reg2&0xF].Real;
            }
            BREAK();
            DISPATCH(Mul) {
                reg = Fn->Code[ip++];
                reg2 = Fn->Code[ip++];
                Registers[reg & 0xF].Unsigned = Registers[(reg >> 4)].Unsigned * Registers[reg2&0xF].Unsigned;
            }
            BREAK();
            DISPATCH(IMul) {
                reg = Fn->Code[ip++];
                reg2 = Fn->Code[ip++];
                Registers[reg & 0xF].Signed = Registers[(reg >> 4)].Signed * Registers[reg2&0xF].Signed;
            }
            BREAK();
            DISPATCH(FMul) {
                reg = Fn->Code[ip++];
                reg2 = Fn->Code[ip++];
                Registers[reg & 0xF].Real = Registers[(reg >> 4)].Real * Registers[reg2&0xF].Real;
            }
            BREAK();
            DISPATCH(Div) {
                reg = Fn->Code[ip++];
                reg2 = Fn->Code[ip++];
                Registers[reg & 0xF].Unsigned = Registers[(reg >> 4)].Unsigned / Registers[reg2&0xF].Unsigned;
            }
            BREAK();
            DISPATCH(IDiv) {
                reg = Fn->Code[ip++];
                reg2 = Fn->Code[ip++];
                Registers[reg & 0xF].Signed = Registers[(reg >> 4)].Signed / Registers[reg2&0xF].Signed;
            }
            BREAK();
            DISPATCH(FDiv) {
                reg = Fn->Code[ip++];
                reg2 = Fn->Code[ip++];
                Registers[reg & 0xF].Real = Registers[(reg >> 4)].Real / Registers[reg2&0xF].Real;
            }
            BREAK();
            DISPATCH(Add8) {
                reg = Fn->Code[ip++];
                constant.Unsigned = Fn->Code[ip++];
                Registers[reg & 0xF].Unsigned = Registers[(reg > 4)].Unsigned + constant.Unsigned;
            }
            BREAK();
            DISPATCH(Add16) {
                reg = Fn->Code[ip++];
                constant.Unsigned = *(UInt16*)(Fn->Code.Data + ip);
                Registers[reg & 0xF].Unsigned = Registers[(reg >> 4)].Unsigned + constant.Unsigned;
                ip += 2;
            }
            BREAK();
            DISPATCH(IAdd8) {
                reg = Fn->Code[ip++];
                constant.Unsigned = Fn->Code[ip++];
                Registers[reg & 0xF].Signed = Registers[(reg > 4)].Signed + constant.Signed;
            }
            BREAK();
            DISPATCH(IAdd16) {
                reg = Fn->Code[ip++];
                constant.Unsigned = *(UInt16*)(Fn->Code.Data + ip);
                ip += 2;
                Registers[reg & 0xF].Signed = Registers[(reg >> 4)].Signed + constant.Signed;
            }
            BREAK();
            DISPATCH(Sub8) {
                reg = Fn->Code[ip++];
                constant.Unsigned = Fn->Code[ip++];
                Registers[reg & 0xF].Unsigned = Registers[(reg > 4)].Unsigned - constant.Unsigned;
            }
            BREAK();
            DISPATCH(Sub16) {
                reg = Fn->Code[ip++];
                constant.Unsigned = *(UInt16*)(Fn->Code.Data + ip);
                ip += 2;
                Registers[reg & 0xF].Unsigned = Registers[(reg >> 4)].Unsigned - constant.Unsigned;
            }
            BREAK();
            DISPATCH(ISub8) {
                reg = Fn->Code[ip++];
                constant.Unsigned = Fn->Code[ip++];
                Registers[reg & 0xF].Signed = Registers[(reg > 4)].Signed - constant.Signed;
            }
            BREAK();
            DISPATCH(ISub16) {
                reg = Fn->Code[ip++];
                constant.Unsigned = *(UInt16*)(Fn->Code.Data + ip);
                ip += 2;
                Registers[reg & 0xF].Signed = Registers[(reg >> 4)].Signed - constant.Signed;
            }
            BREAK();
            DISPATCH(Mul8) {
                reg = Fn->Code[ip++];
                constant.Unsigned = Fn->Code[ip++];
                Registers[reg & 0xF].Unsigned = Registers[(reg > 4)].Unsigned * constant.Unsigned;
            }
            BREAK();
            DISPATCH(Mul16) {
                reg = Fn->Code[ip++];
                constant.Unsigned = *(UInt16*)(Fn->Code.Data + ip);
                ip += 2;
                Registers[reg & 0xF].Unsigned = Registers[(reg >> 4)].Unsigned * constant.Unsigned;
            }
            BREAK();
            DISPATCH(IMul8) {
                reg = Fn->Code[ip++];
                constant.Unsigned = Fn->Code[ip++];
                Registers[reg & 0xF].Signed = Registers[(reg > 4)].Signed * constant.Signed;
            }
            BREAK();
            DISPATCH(IMul16) {
                reg = Fn->Code[ip++];
                constant.Unsigned = *(UInt16*)(Fn->Code.Data + ip);
                ip += 2;
                Registers[reg & 0xF].Signed = Registers[(reg >> 4)].Signed * constant.Signed;
            }
            BREAK();
            DISPATCH(Div8) {
                reg = Fn->Code[ip++];
                constant.Unsigned = Fn->Code[ip++];
                Registers[reg & 0xF].Unsigned = Registers[(reg > 4)].Unsigned / constant.Unsigned;
            }
            BREAK();
            DISPATCH(Div16) {
                reg = Fn->Code[ip++];
                constant.Unsigned = *(UInt16*)(Fn->Code.Data + ip);
                ip += 2;
                Registers[reg & 0xF].Unsigned = Registers[(reg >> 4)].Unsigned / constant.Unsigned;
            }
            BREAK();
            DISPATCH(IDiv8) {
                reg = Fn->Code[ip++];
                constant.Unsigned = Fn->Code[ip++];
                Registers[reg & 0xF].Signed = Registers[(reg > 4)].Signed / constant.Signed;
            }
            BREAK();
            DISPATCH(IDiv16) {
                reg = Fn->Code[ip++];
                constant.Unsigned = *(UInt16*)(Fn->Code.Data + ip);
                ip += 2;
                Registers[reg & 0xF].Signed = Registers[(reg >> 4)].Signed / constant.Signed;
            }
            BREAK();
            DISPATCH(Cmp) {
                reg = Fn->Code[ip++];
                CMPFlags.Unsigned = 0;
                if (Registers[reg & 0xF].Unsigned == Registers[(reg >> 4)].Unsigned) {
                    if (Registers[(reg >> 4)].Unsigned == 0) {
                        CMPFlags.Unsigned |= ZERO_BIT;
                    }
                    CMPFlags.Unsigned |= EQUAL_BIT;
                }
                if (Registers[reg & 0xF].Unsigned < Registers[(reg >> 4)].Unsigned) {
                    CMPFlags.Unsigned |= LESS_BIT;
                }
                if (Registers[reg & 0xF].Unsigned > Registers[(reg >> 4)].Unsigned) {
                    CMPFlags.Unsigned |= GREATER_BIT;
                }
            }
            BREAK();
            DISPATCH(ICmp) {
                reg = Fn->Code[ip++];
                CMPFlags.Unsigned = 0;
                if (Registers[reg & 0xF].Signed == Registers[(reg >> 4)].Signed) {
                    if (Registers[(reg >> 4)].Signed == 0) {
                        CMPFlags.Unsigned |= ZERO_BIT;
                    }
                    CMPFlags.Unsigned |= EQUAL_BIT;
                }
                if (Registers[reg & 0xF].Signed < Registers[(reg >> 4)].Signed) {
                    CMPFlags.Unsigned |= LESS_BIT;
                }
                if (Registers[reg & 0xF].Signed > Registers[(reg >> 4)].Signed) {
                    CMPFlags.Unsigned |= GREATER_BIT;
                }
            }
            BREAK();
            DISPATCH(FCmp) {
                reg = Fn->Code[ip++];
                CMPFlags.Unsigned = 0;
                if (Registers[reg & 0xF].Real == Registers[(reg >> 4)].Real) {
                    if (Registers[(reg >> 4)].Real == 0.0) {
                        CMPFlags.Unsigned |= ZERO_BIT;
                    }
                    CMPFlags.Unsigned |= EQUAL_BIT;
                }
                if (Registers[reg & 0xF].Real < Registers[(reg >> 4)].Real) {
                    CMPFlags.Unsigned |= LESS_BIT;
                }
                if (Registers[reg & 0xF].Real > Registers[(reg >> 4)].Real) {
                    CMPFlags.Unsigned |= GREATER_BIT;
                }
            }
            BREAK();
            DISPATCH(Jmp)
                ip = *(codefile::AddressType*)(Fn->Code.Data + ip);
            BREAK();
            DISPATCH(Jez)
                if (CMPFlags.Unsigned & ZERO_BIT && CMPFlags.Unsigned & EQUAL_BIT) {
                    ip = *(codefile::AddressType*)(Fn->Code.Data + ip);
                }
                else {
                    ip += sizeof(codefile::AddressType);
                }
            BREAK();
            DISPATCH(Jnz)
                if (CMPFlags.Unsigned & ZERO_BIT && !(CMPFlags.Unsigned & EQUAL_BIT)) {
                    ip = *(codefile::AddressType*)(Fn->Code.Data + ip);
                }
                else {
                    ip += sizeof(codefile::AddressType);
                }
            BREAK();
            DISPATCH(Je)
                if (CMPFlags.Unsigned & EQUAL_BIT) {
                    ip = *(codefile::AddressType*)(Fn->Code.Data + ip);
                }
                else {
                    ip += sizeof(codefile::AddressType);
                }
            BREAK();
            DISPATCH(Jne)
                if (!(CMPFlags.Unsigned & EQUAL_BIT)) {
                    ip = *(codefile::AddressType*)(Fn->Code.Data + ip);
                }
                else {
                    ip += sizeof(codefile::AddressType);
                }
            BREAK();
            DISPATCH(Jl)
                if (CMPFlags.Unsigned & LESS_BIT) {
                    ip = *(codefile::AddressType*)(Fn->Code.Data + ip);
                }
                else {
                    ip += sizeof(codefile::AddressType);
                }
            BREAK();
            DISPATCH(Jle)
                if (CMPFlags.Unsigned & LESS_BIT && CMPFlags.Unsigned & EQUAL_BIT) {
                    ip = *(codefile::AddressType*)(Fn->Code.Data + ip);
                }
                else {
                    ip += sizeof(codefile::AddressType);
                }
            BREAK();
            DISPATCH(Jg)
                if (CMPFlags.Unsigned & GREATER_BIT) {
                    ip = *(codefile::AddressType*)(Fn->Code.Data + ip);
                }
                else {
                    ip += sizeof(codefile::AddressType);
                }
            BREAK();
            DISPATCH(Jge)
                if (CMPFlags.Unsigned & GREATER_BIT && CMPFlags.Unsigned & EQUAL_BIT) {
                    ip = *(codefile::AddressType*)(Fn->Code.Data + ip);
                }
                else {
                    ip += sizeof(codefile::AddressType);
                }
            BREAK();
            DISPATCH(Jmp8)
                ip = Fn->Code[ip];
            BREAK();
            DISPATCH(Jez8)
                if (CMPFlags.Unsigned & ZERO_BIT && CMPFlags.Unsigned & EQUAL_BIT)
                    ip = Fn->Code[ip];
                else
                    ip++;
            BREAK();
            DISPATCH(Jnz8)
                if (CMPFlags.Unsigned & ZERO_BIT && !(CMPFlags.Unsigned & EQUAL_BIT))
                    ip = Fn->Code[ip];
                else
                    ip++;
            BREAK();
            DISPATCH(Je8)
                if (CMPFlags.Unsigned & EQUAL_BIT)
                    ip = Fn->Code[ip];
                else
                    ip++;
            BREAK();
            DISPATCH(Jne8)
                if (!(CMPFlags.Unsigned & EQUAL_BIT))
                    ip = Fn->Code[ip];
                else
                    ip++;
            BREAK();
            DISPATCH(Jl8)
                if (CMPFlags.Unsigned & LESS_BIT)
                    ip = Fn->Code[ip];
                else
                    ip++;
            BREAK();
            DISPATCH(Jle8)
                if (CMPFlags.Unsigned & LESS_BIT && CMPFlags.Unsigned & EQUAL_BIT)
                    ip = Fn->Code[ip];
                else
                    ip++;
            BREAK();
            DISPATCH(Jg8)
                if (CMPFlags.Unsigned & GREATER_BIT)
                    ip = Fn->Code[ip];
                else
                    ip++;
            BREAK();
            DISPATCH(Jge8)
                if (CMPFlags.Unsigned & GREATER_BIT && CMPFlags.Unsigned & EQUAL_BIT)
                    ip = Fn->Code[ip];
                else
                    ip++;
            BREAK();
            DISPATCH(Call8) {
                codefile::FunctionType idx = (codefile::FunctionType)Fn->Code[ip++];
                Function* called = &Fn->Asm->Functions[idx];
                frame.Result = Self.ProcessCall(frame, called);
            }
            BREAK();
            DISPATCH(Call) {
                codefile::FunctionType idx = *(codefile::FunctionType*)(Fn->Code.Data + ip);
                ip += sizeof(codefile::FunctionType);
                Function* called = &Fn->Asm->Functions[idx];
                frame.Result = Self.ProcessCall(frame, called);
            }
            BREAK();
            DISPATCH(RetVoid)
                return Registers[0];
            DISPATCH(Ret) {
                reg = Fn->Code[ip];
                return Registers[reg&0xF];
            }
            DISPATCH(Ret8) {
                constant.Unsigned = Fn->Code[ip++];
                return constant;
            }
            DISPATCH(Ret16) {
                constant.Unsigned = *(UInt16*)(Fn->Code.Data + ip);
                return constant;
            }
            DISPATCH(RetLocal) {
                local = *(codefile::LocalType*)(Fn->Code.Data + ip);
                ip += sizeof(codefile::LocalType);
                return frame.FP[local];
            }
            DISPATCH(RetGlobal) {
                global = *(codefile::GlobalType*)(Fn->Code.Data + ip);
                ip += sizeof(codefile::GlobalType);
                return Fn->Asm->Globals[global].Contant;
            }
            DISPATCH(NewObject) {

            }
            BREAK();
            DISPATCH(DestroyObject) {

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
            DISPATCH(NewArray) {

            }
            BREAK();
            DISPATCH(NewArrayObject) {

            }
            BREAK();
            DISPATCH(ArrayLen) {

            }
            BREAK();
            DISPATCH(ArraySet) {

            }
            BREAK();
            DISPATCH(ArrayGet) {

            }
            BREAK();
            DISPATCH(StringGet4) {
                reg = Fn->Code[ip++];
                Registers[reg & 0xF].Ptr = &Fn->Asm->Strings[Byte(reg >> 4)];
            }
            BREAK();
            DISPATCH(StringGet) {
                reg = Fn->Code[ip++];
                string = *(codefile::StringType*)(Fn->Code.Data + ip);
                Registers[reg & 0xF].Ptr = &Fn->Asm->Strings[Byte(reg >> 4)];
            }
            BREAK();
            DISPATCH(Push) {
                reg = Fn->Code[ip++];
                Push(frame, Registers[reg&0xF]);
            }
            BREAK();
            DISPATCH(LocalPush) {
                local = *(codefile::LocalType*)(Fn->Code.Data + ip);
                ip += sizeof(codefile::LocalType);
                Push(frame, frame.FP[local]);
            }
            BREAK();
            DISPATCH(GlobalPush) {
                global = *(codefile::GlobalType*)(Fn->Code.Data + ip);
                ip += sizeof(codefile::LocalType);
                Push(frame, frame.Globals[global].Contant);
            }
            BREAK();
            DISPATCH(Push8) {
                constant.Unsigned = Fn->Code[ip++];
                Push(
                    frame,
                    constant
                );
            }
            BREAK();
            DISPATCH(Push16) {
                constant.Unsigned = *(UInt16*)(Fn->Code.Data + ip);
                ip += 2;
                Push(
                    frame,
                    constant
                );
            }
            BREAK();
            DISPATCH(Push32) {
                constant.Unsigned = *(UInt32*)(Fn->Code.Data + ip);
                ip += 4;
                Push(
                    frame,
                    constant
                );
            }
            BREAK();
            DISPATCH(Push64) {
                constant.Unsigned = *(UInt64*)(Fn->Code.Data + ip);
                ip += 8;
                Push(
                    frame,
                    constant
                );
            }
            BREAK();
            DISPATCH(PopTop)
                (void)Pop(frame);
            BREAK();
            DISPATCH(Pop) {
                reg = Fn->Code[ip++];
                Registers[reg&0xF] = Pop(frame);
            }
            BREAK();
        default:
            RuntimeError(0, STR("Invalid OpCode 0x{xb:0}"), (Byte)opcode);
            BREAK();
        }
    }
}

Value ThreadExecution::ProcessCall(this ThreadExecution& Self, StackFrame& Frame, Function* Fn) {
    RuntimeError(Fn != nullptr,
                 STR("Invalid function")
    );

    ThreadState state = {
        .VM = Self.VM,
    };

    Value result = {};

    if (Fn->Native) {
        result = Fn->Native(&state,
                           Registers[1], Registers[2], Registers[3], Registers[4], Registers[5],
                           Registers[6], Registers[7], Registers[8], Registers[9], Registers[10]
        );
    }
    else {
        result = Self.Exec(Fn, Frame.SP);
        Frame.SP -= Fn->NonNative.Arguments;
    }

    return result;
}

void ThreadExecution::Destroy(this ThreadExecution& /*Self*/) {}

}
