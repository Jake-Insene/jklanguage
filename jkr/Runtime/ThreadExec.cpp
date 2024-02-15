#include "jkr/Runtime/ThreadExec.h"
#include "jkr/Lib/Debug.h"

thread_local runtime::Value Registers[16] = {};
thread_local runtime::Value CMPFlags = {};

#define ZERO_BIT 0x1
#define EQUAL_BIT 0x2
#define LESS_BIT 0x4
#define GREATER_BIT 0x8

#define DISPATCH(OP) case codefile::OpCode::##OP:
#define DISPATCH_STACK(OP) case codefile::OpCodeStack::##OP:
#define BREAK() break;

namespace runtime {

static constexpr void Push(StackFrame& Frame, Value Val) {
    *Frame.SP = Val;
    Frame.SP++;
}

static constexpr Value Pop(StackFrame& Frame) {
    return *--Frame.SP;
}

ThreadExecution runtime::ThreadExecution::New(USize StackSize, Assembly& Asm) {
    return ThreadExecution{
        .ThreadStack = Stack::New(StackSize),
        .Asm = Asm,
    };
}

Value ThreadExecution::Exec(this ThreadExecution& Self, unsigned int Index, Value* SP) {
    RuntimeError(Index <= Self.Asm.Header.CountOfFunctions,
                 STR("Unknown Function At index {u}"),
                 IntCast<UInt>(Index)
    );

    UInt ip = 0;
    Function& fn = Self.Asm.Functions.Data[Index];

    StackFrame frame = {};
    frame.FP = SP - fn.Header.Arguments;
    frame.SP = SP + fn.Header.LocalCount;
    frame.Globals = Self.Asm.Globals.Data;

    while (true) {
        codefile::OpCode opcode = IntCast<codefile::OpCode>(fn.Code.Data[ip++]);
        switch (opcode) {
            DISPATCH(Brk) {
                debug::Break();
                BREAK(); 
            }
            DISPATCH(Mov) {
                Byte b = fn.Code.Data[ip++];
                Registers[b & 0xF] = Registers[Byte(b >> 4)];
                BREAK();
            }
            DISPATCH(Const4) {
                Byte b = fn.Code.Data[ip++];
                Registers[b & 0xF].Unsigned = Byte(b >> 4);
                BREAK();
            }
            DISPATCH(Const8) {
                Byte reg = fn.Code.Data[ip++];
                Registers[reg].Unsigned = fn.Code.Data[ip++];
                BREAK();
            }
            DISPATCH(Const16) {
                Byte reg = fn.Code.Data[ip++];
                Registers[reg].Unsigned = *(Uint16*)(fn.Code.Data + ip);
                ip += 2;
                BREAK();
            }
            DISPATCH(Const32) {
                Byte reg = fn.Code.Data[ip++];
                Registers[reg].Unsigned = *(Uint32*)(fn.Code.Data + ip);
                ip += 4;
                BREAK();
            }
            DISPATCH(Const64) {
                Byte reg = fn.Code.Data[ip++];
                Registers[reg].Unsigned = *(Uint64*)(fn.Code.Data + ip);
                ip += 8;
                BREAK();
            }
            DISPATCH(MovRes) {
                Registers[fn.Code.Data[ip++]] = frame.Result;
                BREAK();
            }
            DISPATCH(LocalSet4) {
                Byte op = fn.Code[ip++];
                frame.FP[op >> 4] = Registers[op & 0xF];
                BREAK();
            }
            DISPATCH(LocalGet4) {
                Byte op = fn.Code[ip++];
                Registers[op & 0xF] = frame.FP[op >> 4];
                BREAK();
            }
            DISPATCH(LocalSet) {
                codefile::LIL& lil = *(codefile::LIL*)(fn.Code.Data + ip);
                ip += sizeof(codefile::LIL);
                frame.FP[lil.Index] = Registers[lil.SrcDest];
                BREAK();
            }
            DISPATCH(LocalGet) {
                codefile::LIL& lil = *(codefile::LIL*)(fn.Code.Data + ip);
                ip += sizeof(codefile::LIL);
                Registers[lil.SrcDest] = frame.FP[lil.Index];
                BREAK();
            }
            DISPATCH(GlobalSet) {
                BREAK();
            }
            DISPATCH(GlobalGet) {
                BREAK();
            }
            DISPATCH(Inc) {
                Byte reg = fn.Code[ip++];
                Registers[reg].Unsigned++;
                BREAK();
            }
            DISPATCH(IInc) {
                Byte reg = fn.Code[ip++];
                Registers[reg].Signed++;
                BREAK();
            }
            DISPATCH(FInc) {
                Byte reg = fn.Code[ip++];
                Registers[reg].Real++;
                BREAK();
            }
            DISPATCH(Dec) {
                Byte reg = fn.Code[ip++];
                Registers[reg].Unsigned--;
                BREAK();
            }
            DISPATCH(IDec) {
                Byte reg = fn.Code[ip++];
                Registers[reg].Signed--;
                BREAK();
            }
            DISPATCH(FDec) {
                Byte reg = fn.Code[ip++];
                Registers[reg].Real--;
                BREAK();
            }
            DISPATCH(Add) {
                Byte reg = fn.Code[ip++];
                Byte src2 = fn.Code[ip++];
                ip += sizeof(codefile::CIL);
                Registers[reg & 0xF].Unsigned = Registers[(reg >> 4)].Unsigned + Registers[src2].Unsigned;
                BREAK();
            }
            DISPATCH(IAdd) {
                Byte reg = fn.Code[ip++];
                Byte src2 = fn.Code[ip++];
                Registers[reg & 0xF].Signed = Registers[(reg >> 4)].Signed + Registers[src2].Signed;
                BREAK();
            }
            DISPATCH(FAdd) {
                Byte reg = fn.Code[ip++];
                Byte src2 = fn.Code[ip++];
                Registers[reg & 0xF].Real = Registers[(reg >> 4)].Real + Registers[src2].Real;
                BREAK();
            }
            DISPATCH(Sub) {
                Byte reg = fn.Code[ip++];
                Byte src2 = fn.Code[ip++];
                Registers[reg & 0xF].Unsigned = Registers[(reg >> 4)].Unsigned - Registers[src2].Unsigned;
            }
            BREAK();
            DISPATCH(ISub) {
                Byte reg = fn.Code[ip++];
                Byte src2 = fn.Code[ip++];
                Registers[reg & 0xF].Signed = Registers[(reg >> 4)].Signed - Registers[src2].Signed;
            }
            BREAK();
            DISPATCH(FSub) {
                Byte reg = fn.Code[ip++];
                Byte src2 = fn.Code[ip++];
                Registers[reg & 0xF].Real = Registers[(reg >> 4)].Real - Registers[src2].Real;
            }
            BREAK();
            DISPATCH(Mul) {
                Byte reg = fn.Code[ip++];
                Byte src2 = fn.Code[ip++];
                Registers[reg & 0xF].Unsigned = Registers[(reg >> 4)].Unsigned * Registers[src2].Unsigned;
            }
            BREAK();
            DISPATCH(IMul) {
                Byte reg = fn.Code[ip++];
                Byte src2 = fn.Code[ip++];
                Registers[reg & 0xF].Signed = Registers[(reg >> 4)].Signed * Registers[src2].Signed;
            }
            BREAK();
            DISPATCH(FMul) {
                Byte reg = fn.Code[ip++];
                Byte src2 = fn.Code[ip++];
                Registers[reg & 0xF].Real = Registers[(reg >> 4)].Real * Registers[src2].Real;
            }
            BREAK();
            DISPATCH(Div) {
                Byte reg = fn.Code[ip++];
                Byte src2 = fn.Code[ip++];
                Registers[reg & 0xF].Unsigned = Registers[(reg >> 4)].Unsigned / Registers[src2].Unsigned;
            }
            BREAK();
            DISPATCH(IDiv) {
                Byte reg = fn.Code[ip++];
                Byte src2 = fn.Code[ip++];
                Registers[reg & 0xF].Signed = Registers[(reg >> 4)].Signed / Registers[src2].Signed;
            }
            BREAK();
            DISPATCH(FDiv) {
                Byte reg = fn.Code[ip++];
                Byte src2 = fn.Code[ip++];
                Registers[reg & 0xF].Real = Registers[(reg >> 4)].Real / Registers[src2].Real;
            }
            BREAK();
            DISPATCH(Add8) {
                Byte reg = fn.Code[ip++];
                Byte c = fn.Code[ip++];

                Registers[reg & 0xF].Unsigned = Registers[(reg > 4)].Unsigned + c;
            }
            BREAK();
            DISPATCH(Add16) {
                Byte reg = fn.Code[ip++];
                Uint16 c = *(Uint16*)(fn.Code.Data + ip);
                ip += 2;
                Registers[reg & 0xF].Unsigned = Registers[(reg >> 4)].Unsigned + c;
            }
            BREAK();
            DISPATCH(IAdd8) {
                Byte reg = fn.Code[ip++];
                Byte c = fn.Code[ip++];

                Registers[reg & 0xF].Signed = Registers[(reg > 4)].Signed + c;
            }
            BREAK();
            DISPATCH(IAdd16) {
                Byte reg = fn.Code[ip++];
                Uint16 c = *(Uint16*)(fn.Code.Data + ip);
                ip += 2;
                Registers[reg & 0xF].Signed = Registers[(reg >> 4)].Signed + c;
            }
            BREAK();
            DISPATCH(Sub8) {
                Byte reg = fn.Code[ip++];
                Byte c = fn.Code[ip++];

                Registers[reg & 0xF].Unsigned = Registers[(reg > 4)].Unsigned - c;
            }
            BREAK();
            DISPATCH(Sub16) {
                Byte reg = fn.Code[ip++];
                Uint16 c = *(Uint16*)(fn.Code.Data + ip);
                ip += 2;
                Registers[reg & 0xF].Unsigned = Registers[(reg >> 4)].Unsigned - c;
            }
            BREAK();
            DISPATCH(ISub8) {
                Byte reg = fn.Code[ip++];
                Byte c = fn.Code[ip++];

                Registers[reg & 0xF].Signed = Registers[(reg > 4)].Signed - c;
            }
            BREAK();
            DISPATCH(ISub16) {
                Byte reg = fn.Code[ip++];
                Uint16 c = *(Uint16*)(fn.Code.Data + ip);
                ip += 2;
                Registers[reg & 0xF].Signed = Registers[(reg >> 4)].Signed - c;
            }
            BREAK();
            DISPATCH(Mul8) {
                Byte reg = fn.Code[ip++];
                Byte c = fn.Code[ip++];

                Registers[reg & 0xF].Unsigned = Registers[(reg > 4)].Unsigned * c;
            }
            BREAK();
            DISPATCH(Mul16) {
                Byte reg = fn.Code[ip++];
                Uint16 c = *(Uint16*)(fn.Code.Data + ip);
                ip += 2;
                Registers[reg & 0xF].Unsigned = Registers[(reg >> 4)].Unsigned * c;
            }
            BREAK();
            DISPATCH(IMul8) {
                Byte reg = fn.Code[ip++];
                Byte c = fn.Code[ip++];

                Registers[reg & 0xF].Signed = Registers[(reg > 4)].Signed * c;
            }
            BREAK();
            DISPATCH(IMul16) {
                Byte reg = fn.Code[ip++];
                Uint16 c = *(Uint16*)(fn.Code.Data + ip);
                ip += 2;
                Registers[reg & 0xF].Signed = Registers[(reg >> 4)].Signed * c;
            }
            BREAK();
            DISPATCH(Div8) {
                Byte reg = fn.Code[ip++];
                Byte c = fn.Code[ip++];

                Registers[reg & 0xF].Unsigned = Registers[(reg > 4)].Signed / c;
            }
            BREAK();
            DISPATCH(Div16) {
                Byte reg = fn.Code[ip++];
                Uint16 c = *(Uint16*)(fn.Code.Data + ip);
                ip += 2;
                Registers[reg & 0xF].Unsigned = Registers[(reg >> 4)].Unsigned / c;
            }
            BREAK();
            DISPATCH(IDiv8) {
                Byte reg = fn.Code[ip++];
                Byte c = fn.Code[ip++];

                Registers[reg & 0xF].Signed = Registers[(reg > 4)].Signed / c;
            }
            BREAK();
            DISPATCH(IDiv16) {
                Byte reg = fn.Code[ip++];
                Uint16 c = *(Uint16*)(fn.Code.Data + ip);
                ip += 2;
                Registers[reg & 0xF].Signed = Registers[(reg >> 4)].Signed / c;
            }
            BREAK();
            DISPATCH(Cmp) {
                codefile::MIL& bil = *(codefile::MIL*)(fn.Code.Data + ip);
                ip += sizeof(codefile::MIL);
                CMPFlags.Unsigned = 0;
                if (Registers[bil.Dest].Unsigned == Registers[bil.Src].Unsigned) {
                    if (Registers[bil.Src].Unsigned == 0) {
                        CMPFlags.Unsigned |= ZERO_BIT;
                    }
                    CMPFlags.Unsigned |= EQUAL_BIT;
                }
                if (Registers[bil.Dest].Unsigned < Registers[bil.Src].Unsigned) {
                    CMPFlags.Unsigned |= LESS_BIT;
                }
                if (Registers[bil.Dest].Unsigned > Registers[bil.Src].Unsigned) {
                    CMPFlags.Unsigned |= GREATER_BIT;
                }
            }
            BREAK();
            DISPATCH(ICmp) {
                codefile::MIL& bil = *(codefile::MIL*)(fn.Code.Data + ip);
                ip += sizeof(codefile::MIL);
                CMPFlags.Unsigned = 0;
                if (Registers[bil.Dest].Signed == Registers[bil.Src].Signed) {
                    if (Registers[bil.Src].Signed == 0) {
                        CMPFlags.Unsigned |= ZERO_BIT;
                    }
                    CMPFlags.Unsigned |= EQUAL_BIT;
                }
                if (Registers[bil.Dest].Signed < Registers[bil.Src].Signed) {
                    CMPFlags.Unsigned |= LESS_BIT;
                }
                if (Registers[bil.Dest].Signed > Registers[bil.Src].Signed) {
                    CMPFlags.Unsigned |= GREATER_BIT;
                }
            }
            BREAK();
            DISPATCH(FCmp) {
                codefile::MIL& bil = *(codefile::MIL*)(fn.Code.Data + ip);
                ip += sizeof(codefile::MIL);
                CMPFlags.Unsigned = 0;
                if (Registers[bil.Dest].Real == Registers[bil.Src].Real) {
                    if (Registers[bil.Src].Real == 0.0) {
                        CMPFlags.Unsigned |= ZERO_BIT;
                    }
                    CMPFlags.Unsigned |= EQUAL_BIT;
                }
                if (Registers[bil.Dest].Real < Registers[bil.Src].Real) {
                    CMPFlags.Unsigned |= LESS_BIT;
                }
                if (Registers[bil.Dest].Real > Registers[bil.Src].Real) {
                    CMPFlags.Unsigned |= GREATER_BIT;
                }
            }
            BREAK();
            DISPATCH(Jmp)
                ip = *(codefile::AddressType*)(fn.Code.Data + ip);
            BREAK();
            DISPATCH(Jez)
                if (CMPFlags.Unsigned & ZERO_BIT && CMPFlags.Unsigned & EQUAL_BIT) {
                    ip = *(codefile::AddressType*)(fn.Code.Data + ip);
                }
                else {
                    ip += sizeof(codefile::AddressType);
                }
            BREAK();
            DISPATCH(Jnz)
                if (CMPFlags.Unsigned & ZERO_BIT && !(CMPFlags.Unsigned & EQUAL_BIT)) {
                    ip = *(codefile::AddressType*)(fn.Code.Data + ip);
                }
                else {
                    ip += sizeof(codefile::AddressType);
                }
            BREAK();
            DISPATCH(Je)
                if (CMPFlags.Unsigned & EQUAL_BIT) {
                    ip = *(codefile::AddressType*)(fn.Code.Data + ip);
                }
                else {
                    ip += sizeof(codefile::AddressType);
                }
            BREAK();
            DISPATCH(Jne)
                if (!(CMPFlags.Unsigned & EQUAL_BIT)) {
                    ip = *(codefile::AddressType*)(fn.Code.Data + ip);
                }
                else {
                    ip += sizeof(codefile::AddressType);
                }
            BREAK();
            DISPATCH(Jl)
                if (CMPFlags.Unsigned & LESS_BIT) {
                    ip = *(codefile::AddressType*)(fn.Code.Data + ip);
                }
                else {
                    ip += sizeof(codefile::AddressType);
                }
            BREAK();
            DISPATCH(Jle)
                if (CMPFlags.Unsigned & LESS_BIT && CMPFlags.Unsigned & EQUAL_BIT) {
                    ip = *(codefile::AddressType*)(fn.Code.Data + ip);
                }
                else {
                    ip += sizeof(codefile::AddressType);
                }
            BREAK();
            DISPATCH(Jg)
                if (CMPFlags.Unsigned & GREATER_BIT) {
                    ip = *(codefile::AddressType*)(fn.Code.Data + ip);
                }
                else {
                    ip += sizeof(codefile::AddressType);
                }
            BREAK();
            DISPATCH(Jge)
                if (CMPFlags.Unsigned & GREATER_BIT && CMPFlags.Unsigned & EQUAL_BIT) {
                    ip = *(codefile::AddressType*)(fn.Code.Data + ip);
                }
                else {
                    ip += sizeof(codefile::AddressType);
                }
            BREAK();
            DISPATCH(Jmp8)
                ip = fn.Code.Data[ip];
            BREAK();
            DISPATCH(Jez8)
                if (CMPFlags.Unsigned & ZERO_BIT && CMPFlags.Unsigned & EQUAL_BIT)
                    ip = fn.Code.Data[ip];
                else
                    ip++;
            BREAK();
            DISPATCH(Jnz8)
                if (CMPFlags.Unsigned & ZERO_BIT && !(CMPFlags.Unsigned & EQUAL_BIT))
                    ip = fn.Code.Data[ip];
                else
                    ip++;
            BREAK();
            DISPATCH(Je8)
                if (CMPFlags.Unsigned & EQUAL_BIT)
                    ip = fn.Code.Data[ip];
                else
                    ip++;
            BREAK();
            DISPATCH(Jne8)
                if (!(CMPFlags.Unsigned & EQUAL_BIT))
                    ip = fn.Code.Data[ip];
                else
                    ip++;
            BREAK();
            DISPATCH(Jl8)
                if (CMPFlags.Unsigned & LESS_BIT)
                    ip = fn.Code.Data[ip];
                else
                    ip++;
            BREAK();
            DISPATCH(Jle8)
                if (CMPFlags.Unsigned & LESS_BIT && CMPFlags.Unsigned & EQUAL_BIT)
                    ip = fn.Code.Data[ip];
                else
                    ip++;
            BREAK();
            DISPATCH(Jg8)
                if (CMPFlags.Unsigned & GREATER_BIT)
                    ip = fn.Code.Data[ip];
                else
                    ip++;
            BREAK();
            DISPATCH(Jge8)
                if (CMPFlags.Unsigned & GREATER_BIT && CMPFlags.Unsigned & EQUAL_BIT)
                    ip = fn.Code.Data[ip];
                else
                    ip++;
            BREAK();
            DISPATCH(Call) {
                codefile::FunctionType idx = *(codefile::FunctionType*)(fn.Code.Data + ip);
                ip += sizeof(codefile::FunctionType);
                frame.Result = Self.Exec(idx, frame.SP);
                frame.SP -= Self.Asm.Functions[idx].Header.Arguments;
            BREAK();
            }
            DISPATCH(Call8) {
                Byte idx = fn.Code.Data[ip++];
                frame.Result = Self.Exec(idx, frame.SP);
                frame.SP -= Self.Asm.Functions[idx].Header.Arguments;
            BREAK();
            }
            DISPATCH(RetVoid)
                return Registers[0];
            DISPATCH(Ret) {
                Byte reg = fn.Code.Data[ip];
                return Registers[reg];
            }
            DISPATCH(Ret8) {
                Byte c = fn.Code.Data[ip++];
                return { c };
            }
            DISPATCH(Ret16) {
                Uint16 c = *(Uint16*)(fn.Code.Data + ip);
                return { c };
            }
            DISPATCH(RetLocal) {
                codefile::LocalType idx = *(codefile::LocalType*)(fn.Code.Data + ip);
                ip += sizeof(codefile::LocalType);
                return frame.FP[idx];
            }
            DISPATCH(RetGlobal) {
                codefile::GlobalType idx = *(codefile::GlobalType*)(fn.Code.Data + ip);
                ip += sizeof(codefile::GlobalType);
                return Self.Asm.Globals[idx];
            }
            DISPATCH(StackPrefix) {
                codefile::OpCodeStack s = IntCast<codefile::OpCodeStack>(fn.Code.Data[ip++]);
                switch (s) {
                    DISPATCH_STACK(Push) {
                        Byte reg = fn.Code.Data[ip++];
                        Push(frame, Registers[reg]);
                        BREAK();
                    }
                    DISPATCH_STACK(LocalPush) {
                        codefile::LocalType idx = *(codefile::LocalType*)(fn.Code.Data + ip);
                        ip += sizeof(codefile::LocalType);
                        Push(frame, frame.FP[idx]);
                        BREAK();
                    }
                    DISPATCH_STACK(GlobalPush) {
                        codefile::GlobalType idx = *(codefile::GlobalType*)(fn.Code.Data + ip);
                        ip += sizeof(codefile::LocalType);
                        Push(frame, Self.Asm.Globals[idx]);
                        BREAK();
                    }
                    DISPATCH_STACK(Push8) {
                        Byte constant = fn.Code.Data[ip++];
                        Push(
                            frame,
                            Value{
                            .Unsigned = constant,
                            }
                        );
                        BREAK();
                    }
                    DISPATCH_STACK(Push16) {
                        Uint16 constant = *(Uint16*)(fn.Code.Data + ip);
                        ip += 2;
                        Push(
                            frame,
                            Value{
                            .Unsigned = constant,
                            }
                        );
                        BREAK();
                    }
                    DISPATCH_STACK(Push32) {
                        Uint32 constant = *(Uint32*)(fn.Code.Data + ip);
                        ip += 4;
                        Push(
                            frame,
                            Value{
                            .Unsigned = constant,
                            }
                        );
                        BREAK();
                    }
                    DISPATCH_STACK(Push64) {
                        Uint64 constant = *(Uint64*)(fn.Code.Data + ip);
                        ip += 8;
                        Push(
                            frame,
                            Value{
                            .Unsigned = constant,
                            }
                        );
                        BREAK();
                    }
                    DISPATCH_STACK(PopTop)
                        (void)Pop(frame);
                    BREAK();
                    DISPATCH_STACK(Pop) {
                        Byte reg = fn.Code[ip++];
                        Registers[reg] = Pop(frame);
                        BREAK();
                    }
                default:
                    RuntimeError(0, STR("Invalid OpCode 0x{xb:0}"), (Byte)opcode);
                    BREAK();
                }
                BREAK();
            }
        default:
            RuntimeError(0, STR("Invalid OpCode 0x{xb:0}"), (Byte)opcode);
            BREAK();
        }
    }
}

void ThreadExecution::Destroy(this ThreadExecution& Self) {
    Self.ThreadStack.Destroy();
}

}
