#include "jkr/Runtime/ThreadExec.h"
#include "jkr/Lib/Debug.h"

thread_local runtime::Value Registers[7] = {};
thread_local runtime::Value CMPFlags = {};

#define ZERO_BIT 0x1
#define EQUAL_BIT 0x2
#define LESS_BIT 0x4
#define GREATER_BIT 0x8

namespace runtime {

static constexpr void Push(StackFrame& Frame, UInt Val) {
    Frame.SP->Unsigned = Val;
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
    frame.FP = SP - fn.Header.LocalCount;
    frame.SP = frame.FP;
    frame.Globals = Self.Asm.Globals.Data;

    while (true) {
        codefile::OpCode opcode = IntCast<codefile::OpCode>(fn.Code.Data[ip++]);
        switch (opcode) {
        case codefile::OpCode::StackPrefix:
            Self.ExecStack(
                fn,
                IntCast<codefile::OpCodeStack>(fn.Code.Data[ip++]),
                frame,
                ip
            );
            break;
        default:
            Self.ExecCommon(fn, opcode, frame, ip);
            if (opcode >= codefile::OpCode::Ret && opcode <= codefile::OpCode::GRet) {
                return frame.Result;
            }
            break;
        }
    }
}

void ThreadExecution::ExecCommon(this ThreadExecution& Self, const Function& Fn, codefile::OpCode C, 
                                 StackFrame& Frame, UInt& IP) {
    switch (C) {
    case codefile::OpCode::Brk:
        debug::Break();
        break;
    case codefile::OpCode::Mov:
    {
        codefile::LI& li = *(codefile::LI*)(Fn.Code.Data + IP);
        IP += sizeof(codefile::LI);
        Registers[li.Dest] = Registers[li.Src];
    }
        break;
    case codefile::OpCode::Mov8:
    {
        Byte reg = Fn.Code.Data[IP++];
        Registers[reg].Unsigned = Fn.Code.Data[IP++];
    }
        break;
    case codefile::OpCode::Mov16:
    {
        Byte reg = Fn.Code.Data[IP++];
        Registers[reg].Unsigned = *(Uint16*)(Fn.Code.Data + IP);
        IP += 2;
    }
        break;
    case codefile::OpCode::Mov32:
    {
        Byte reg = Fn.Code.Data[IP++];
        Registers[reg].Unsigned = *(Uint32*)(Fn.Code.Data + IP);
        IP += 4;
    }
        break;
    case codefile::OpCode::Mov64:
    {
        Byte reg = Fn.Code.Data[IP++];
        Registers[reg].Unsigned = *(Uint64*)(Fn.Code.Data + IP);
        IP += 8;
    }
        break;
    case codefile::OpCode::RMov:
    {
        Registers[Fn.Code.Data[IP++]] = Frame.Result;
    }
    break;
    case codefile::OpCode::LocalSet:
    {
        codefile::LIL& lil = *(codefile::LIL*)(Fn.Code.Data + IP);
        IP += sizeof(codefile::LIL);
        Frame.FP[lil.Index()] = Registers[lil.SrcDest];
    }
    break;

    case codefile::OpCode::LocalGet:
    {
        codefile::LIL& lil = *(codefile::LIL*)(Fn.Code.Data + IP);
        IP += sizeof(codefile::LIL);
        Registers[lil.SrcDest] = Frame.FP[lil.Index()];
    }
    break;
    case codefile::OpCode::GlobalSet:
        break;
    case codefile::OpCode::GlobalGet:
        break;
    case codefile::OpCode::Add:
    {
        codefile::CIL& cil = *(codefile::CIL*)(Fn.Code.Data + IP);
        IP += sizeof(codefile::CIL);
        Registers[cil.Dest].Unsigned = Registers[cil.Src1].Unsigned + Registers[cil.Src2].Unsigned;
    }
    break;
    case codefile::OpCode::IAdd:
    {
        codefile::CIL& cil = *(codefile::CIL*)(Fn.Code.Data + IP);
        IP += sizeof(codefile::CIL);
        Registers[cil.Dest].Signed = Registers[cil.Src1].Signed + Registers[cil.Src2].Signed;
    }
    break;
    case codefile::OpCode::FAdd:
    {
        codefile::CIL& cil = *(codefile::CIL*)(Fn.Code.Data + IP);
        IP += sizeof(codefile::CIL);
        Registers[cil.Dest].Real = Registers[cil.Src1].Real + Registers[cil.Src2].Real;
    }
    break;
    case codefile::OpCode::Sub:
    {
        codefile::CIL& cil = *(codefile::CIL*)(Fn.Code.Data + IP);
        IP += sizeof(codefile::CIL);
        Registers[cil.Dest].Unsigned = Registers[cil.Src1].Unsigned - Registers[cil.Src2].Unsigned;
    }
    break;
    case codefile::OpCode::ISub:
    {
        codefile::CIL& cil = *(codefile::CIL*)(Fn.Code.Data + IP);
        IP += sizeof(codefile::CIL);
        Registers[cil.Dest].Signed = Registers[cil.Src1].Signed - Registers[cil.Src2].Signed;
    }
    break;
    case codefile::OpCode::FSub:
    {
        codefile::CIL& cil = *(codefile::CIL*)(Fn.Code.Data + IP);
        IP += sizeof(codefile::CIL);
        Registers[cil.Dest].Real = Registers[cil.Src1].Real - Registers[cil.Src2].Real;
    }
    break;
    case codefile::OpCode::Mul:
    {
        codefile::CIL& cil = *(codefile::CIL*)(Fn.Code.Data + IP);
        IP += sizeof(codefile::CIL);
        Registers[cil.Dest].Unsigned = Registers[cil.Src1].Unsigned * Registers[cil.Src2].Unsigned;
    }
    break;
    case codefile::OpCode::IMul:
    {
        codefile::CIL& cil = *(codefile::CIL*)(Fn.Code.Data + IP);
        IP += sizeof(codefile::CIL);
        Registers[cil.Dest].Signed = Registers[cil.Src1].Signed * Registers[cil.Src2].Signed;
    }
    break;
    case codefile::OpCode::FMul:
    {
        codefile::CIL& cil = *(codefile::CIL*)(Fn.Code.Data + IP);
        IP += sizeof(codefile::CIL);
        Registers[cil.Dest].Real = Registers[cil.Src1].Real * Registers[cil.Src2].Real;
    }
    break;
    case codefile::OpCode::Div:
    {
        codefile::CIL& cil = *(codefile::CIL*)(Fn.Code.Data + IP);
        IP += sizeof(codefile::CIL);
        Registers[cil.Dest].Unsigned = Registers[cil.Src1].Unsigned / Registers[cil.Src2].Unsigned;
    }
    break;
    case codefile::OpCode::IDiv:
    {
        codefile::CIL& cil = *(codefile::CIL*)(Fn.Code.Data + IP);
        IP += sizeof(codefile::CIL);
        Registers[cil.Dest].Signed = Registers[cil.Src1].Signed / Registers[cil.Src2].Signed;
    }
    break;
    case codefile::OpCode::FDiv:
    {
        codefile::CIL& cil = *(codefile::CIL*)(Fn.Code.Data + IP);
        IP += sizeof(codefile::CIL);
        Registers[cil.Dest].Real = Registers[cil.Src1].Real / Registers[cil.Src2].Real;
    }
    case codefile::OpCode::Cmp:
    {
        codefile::LI& li = *(codefile::LI*)(Fn.Code.Data + IP);
        IP += sizeof(codefile::LI);
        CMPFlags.Unsigned = 0;
        if (Registers[li.Dest].Unsigned == Registers[li.Src].Unsigned) {
            if (Registers[li.Src].Unsigned == 0) {
                CMPFlags.Unsigned |= ZERO_BIT;
            }
            CMPFlags.Unsigned |= EQUAL_BIT;
        }
        if (Registers[li.Dest].Unsigned < Registers[li.Src].Unsigned) {
            CMPFlags.Unsigned |= LESS_BIT;
        }
        if (Registers[li.Dest].Unsigned > Registers[li.Src].Unsigned) {
            CMPFlags.Unsigned |= GREATER_BIT;
        }
    }
    break;
    case codefile::OpCode::ICmp:
    {
        codefile::LI& li = *(codefile::LI*)(Fn.Code.Data + IP);
        IP += sizeof(codefile::LI);
        CMPFlags.Unsigned = 0;
        if (Registers[li.Dest].Signed == Registers[li.Src].Signed) {
            if (Registers[li.Src].Signed == 0) {
                CMPFlags.Unsigned |= ZERO_BIT;
            }
            CMPFlags.Unsigned |= EQUAL_BIT;
        }
        if (Registers[li.Dest].Signed < Registers[li.Src].Signed) {
            CMPFlags.Unsigned |= LESS_BIT;
        }
        if (Registers[li.Dest].Signed > Registers[li.Src].Signed) {
            CMPFlags.Unsigned |= GREATER_BIT;
        }
    }
    break;
    case codefile::OpCode::FCmp:
    {
        codefile::LI& li = *(codefile::LI*)(Fn.Code.Data + IP);
        IP += sizeof(codefile::LI);
        CMPFlags.Unsigned = 0;
        if (Registers[li.Dest].Real == Registers[li.Src].Real) {
            if (Registers[li.Src].Real == 0.0) {
                CMPFlags.Unsigned |= ZERO_BIT;
            }
            CMPFlags.Unsigned |= EQUAL_BIT;
        }
        if (Registers[li.Dest].Real < Registers[li.Src].Real) {
            CMPFlags.Unsigned |= LESS_BIT;
        }
        if (Registers[li.Dest].Real > Registers[li.Src].Real) {
            CMPFlags.Unsigned |= GREATER_BIT;
        }
    }
    break;
    case codefile::OpCode::Jmp:
        IP = *(Uint32*)(Fn.Code.Data + IP);
    break;
    case codefile::OpCode::Jez:
        if (CMPFlags.Unsigned & ZERO_BIT && CMPFlags.Unsigned & EQUAL_BIT) {
            IP = *(Uint32*)(Fn.Code.Data + IP);
        }
        else {
            IP += 4;
        }
        break;
    case codefile::OpCode::Jnz:
        if (CMPFlags.Unsigned & ZERO_BIT && !(CMPFlags.Unsigned & EQUAL_BIT)) {
            IP = *(Uint32*)(Fn.Code.Data + IP);
        }
        else {
            IP += 4;
        }
        break;
    case codefile::OpCode::Je:
        if (CMPFlags.Unsigned & EQUAL_BIT) {
            IP = *(Uint32*)(Fn.Code.Data + IP);
        }
        else {
            IP += 4;
        }
        break;
    case codefile::OpCode::Jne:
        if (!(CMPFlags.Unsigned & EQUAL_BIT)) {
            IP = *(Uint32*)(Fn.Code.Data + IP);
        }
        else {
            IP += 4;
        }
        break;
    case codefile::OpCode::Jl:
        if (CMPFlags.Unsigned & LESS_BIT) {
            IP = *(Uint32*)(Fn.Code.Data + IP);
        }
        else {
            IP += 4;
        }
        break;
    case codefile::OpCode::Jle:
        if (CMPFlags.Unsigned & LESS_BIT && CMPFlags.Unsigned & EQUAL_BIT) {
            IP = *(Uint32*)(Fn.Code.Data + IP);
        }
        else {
            IP += 4;
        }
        break;
    case codefile::OpCode::Jg:
        if (CMPFlags.Unsigned & GREATER_BIT) {
            IP = *(Uint32*)(Fn.Code.Data + IP);
        }
        else {
            IP += 4;
        }
        break;
    case codefile::OpCode::Jge:
        if (CMPFlags.Unsigned & GREATER_BIT && CMPFlags.Unsigned & EQUAL_BIT) {
            IP = *(Uint32*)(Fn.Code.Data + IP);
        }
        else {
            IP += 4;
        }
        break;
    case codefile::OpCode::Call:
    {
        unsigned int idx = *(unsigned int*)(Fn.Code.Data + IP);
        IP += 4;
        Frame.Result = Self.Exec(idx, Frame.SP);
        Frame.SP -= Self.Asm.Functions[idx].Header.Arguments;
    }
    break;
    case codefile::OpCode::Ret:
        break;
    case codefile::OpCode::RRet:
    {
        Byte reg = Fn.Code.Data[IP++];
        Frame.Result = Registers[reg];
    }
    break;
    case codefile::OpCode::LRet:
        break;
    case codefile::OpCode::GRet:
        break;
    default:
        RuntimeError(0, STR("Invalid OpCode"));
        break;
    }
}

void ThreadExecution::ExecStack(this ThreadExecution& Self, const Function& Fn, codefile::OpCodeStack S,
                                StackFrame& Frame, UInt& IP) {
    switch (S) {
    case codefile::OpCodeStack::RPush:
    {
        Byte reg = Fn.Code.Data[IP++];
        Push(Frame, Registers[reg].Unsigned);
    }
        break;
    case codefile::OpCodeStack::LPush:
    {
        unsigned short idx = *(unsigned short*)(Fn.Code.Data + IP);
        IP += 2;
        Push(Frame, (Frame.FP + idx)->Unsigned);
    }
        break;
    case codefile::OpCodeStack::Push8:
    {
        Byte constant = Fn.Code.Data[IP++];
        Push(Frame, IntCast<UInt>(constant));
    }
    break;
    case codefile::OpCodeStack::Push16:
    {
        unsigned short constant = *(unsigned short*)(Fn.Code.Data + IP);
        IP += 2;
        Push(Frame, IntCast<UInt>(constant));
    }
        break;
    case codefile::OpCodeStack::Push32:
    {
        unsigned int constant = *(unsigned int*)(Fn.Code.Data + IP);
        IP += 4;
        Push(Frame, IntCast<UInt>(constant));
    }
        break;
    case codefile::OpCodeStack::Push64:
    {
        UInt constant = *(UInt*)(Fn.Code.Data + IP);
        IP += 8;
        Push(Frame, constant);
    }
        break;
    case codefile::OpCodeStack::PopTop:
        (void)Pop(Frame);
        break;
    case codefile::OpCodeStack::RPop:
    {
        Byte reg = Fn.Code[IP++];
        Registers[reg] = Pop(Frame);
    }
        break;
    default:
        break;
    }
}

void ThreadExecution::ExecMemory(this ThreadExecution& Self, const Function& Fn, codefile::OpCodeMemory M,
                                 StackFrame& Frame, UInt& IP) {
}

void ThreadExecution::Destroy(this ThreadExecution& Self) {
    Self.ThreadStack.Destroy();
}

}
