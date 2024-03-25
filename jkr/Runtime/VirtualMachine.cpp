#include "jkr/CodeFile/OpCodes.h"
#include "jkr/CodeFile/Type.h"
#include "jkr/Runtime/VirtualMachine.h"
#include "jkr/Error.h"
#include "jkr/String.h"
#include <stdio.h>

using Float4 = Float[4];
using Int4 = Int[4];
using UInt4 = UInt[4];
using Byte32 = Byte[32];

union VectorRegister{
    Float4 FV;
    Int4 IV;
    UInt4 UV;
};

thread_local runtime::Value Registers[16] = {};
thread_local VectorRegister VR[16] = {};

#define ZERO_FLAG 0x01
#define SIGN_FLAG 0x02
thread_local UInt CMP = 0;

#define CHECK_ARRAY_INDEX(IndexExpr) \
    if((IndexExpr) >= array->Size) {\
        RuntimeError(false, "Index out of range");\
    }

#define GET_AND_INC(Type, Dest) \
    Dest = *(Type*)ip;\
    ip += sizeof(Type);

#define HANDLE_MATH(Case, Op, Field) \
    case codefile::OpCode::##Case:\
    GET_AND_INC(UInt16, word);\
    Registers[MATH_DEST(word)].##Field = Registers[MATH_SRC1(word)].##Field Op Registers[MATH_SRC2(word)].##Field;\
    break;\

#define HANDLE_MATH8(Case, Op, Field, Type) \
    case codefile::OpCode::##Case:\
    util = *ip++;\
    util2 = *ip++;\
    Registers[INST_ARG1(util)].##Field = Registers[INST_ARG2(util)].##Field Op Type(util2);\
    break;\

#define HANDLE_MATH16(Case, Op, Field, Type) \
    case codefile::OpCode::##Case:\
    util = *ip++;\
    GET_AND_INC(UInt16, word);\
    Registers[INST_ARG1(util)].##Field = Registers[INST_ARG2(util)].##Field Op Type(word);\
    break;\

namespace runtime {

// Utility

void MakeComparisionInteger(UInt A, UInt B) {
    UInt diff = A - B;
    UInt sign = (diff >> 63) & 0x1;
    CMP |= sign ? SIGN_FLAG : 0;
    CMP |=
        ((diff ^ sign) - sign) ? 
        0 :
        ZERO_FLAG;
}

void MakeComparisionFloat(Float A, Float B) {
    if (A < B) {
        CMP |= SIGN_FLAG;
    }
    if (A > B) {
        CMP |= SIGN_FLAG;
    }
}

static constexpr void Push(StackFrame& Frame, Value Arg) {
    *Frame.SP++ = Arg;
}

static constexpr Value Pop(StackFrame& Frame) {
    return *--Frame.SP;
}

VirtualMachine::VirtualMachine(USize StackSize, Assembly* Asm) :
    VMStack(StackSize), Asm(Asm), Err(VMSuccess), LinkageResolved(false)
{}

VirtualMachine::~VirtualMachine() {}

Int VirtualMachine::ExecMain() {
    if (Asm->Err != AsmOk) return {};
    if (!LinkageResolved) {
        Err = VMLinkageError;
        return {};
    }
    
    Function& fn = Asm->CodeSection[Asm->EntryPoint];

    StackFrame frame = {
        .SP = VMStack.Start,
        .FP = VMStack.Start,
    };
    UInt status = MainLoop(fn, frame);
    (void)status;
    return Registers[0].Signed;
}

void VirtualMachine::ResolveExtern() {
    for (auto& fn : Asm->CodeSection) {
        if (fn.Flags & codefile::FunctionNative) {
            USize lib = {};
            if (!TryLoad(Asm->STSection[fn.SizeOfCode], lib)) {
                Err = VMLinkageError;
                return;
            }

            UInt entryAddress = UInt(fn.StackArguments | (fn.LocalReserve << 16));
            Str entry = Str(Asm->STSection[entryAddress].Bytes);
            fn.Native = (Value(*)(...))Libraries[lib].Get(entry);

            if (!fn.Native) {
                Err = VMLinkageError;
                return;
            }
        }
    }

    LinkageResolved = true;
}

bool VirtualMachine::TryLoad(Array& LibName, USize& Lib) {
    USize i = 0;
    for (auto& lib : Libraries) {
        USize len = lib.FilePath.length();
        if (len != LibName.Size) {
            continue;
        }

        if (memcmp((void*)lib.FilePath.data(), LibName.Bytes, len) == 0) {
            Lib = i;
            return true;
        }
        i++;
    }

    char buff[512] = {};
#ifdef _WIN32
    sprintf_s(buff, 512, "%s.dll", (char*)LibName.Bytes);
#else
    sprintf(buff, "lib%s.so", (char*)LibName.Items);
#endif

    auto& newLib = Libraries.emplace_back((Char*)buff);
    Lib = Libraries.size() - 1;
    if (!newLib.Handle)
        return false;

    return true;
}

UInt VirtualMachine::ProcessCall(StackFrame& Frame, Function& Fn) {
    if (Fn.Native) {
        Registers[0] = Fn.Native(
            Registers[1], Registers[2], Registers[3], Registers[4], Registers[5],
            Registers[6], Registers[7], Registers[8], Registers[9], Registers[10]
        );
        return 0;
    }

    StackFrame newFrame = {
        .SP = Frame.SP + Fn.LocalReserve - Fn.StackArguments,
        .FP = Frame.SP - Fn.StackArguments,
    };

    return MainLoop(Fn, newFrame);
}

UInt VirtualMachine::MainLoop(Function& Fn, StackFrame& Frame) {
    Byte* ip = &Fn.Code[0];

    Value constant = {};
    Byte util = 0;
    Byte util2 = 0;
    union {
        UInt16 word;
        UInt32 dword;
        UInt qword = 0;
    };
    Array* array = nullptr;

    while (true) {
        codefile::OpCode opcode = codefile::OpCode(*ip++);

        switch (opcode) {
        case codefile::OpCode::Brk:
            Break();
            break;
        case codefile::OpCode::Mov:
            util = *ip++;
            Registers[INST_ARG1(util)] = Registers[INST_ARG2(util)];
            break;
        case codefile::OpCode::Mov4:
            util = *ip++;
            Registers[INST_ARG1(util)].Unsigned = INST_ARG2(util);
            break;
        case codefile::OpCode::Mov8:
            util = *ip++;
            util2 = *ip++;
            Registers[INST_ARG1(util)].Unsigned = util2;
            break;
        case codefile::OpCode::Mov16:
            util = *ip++;
            GET_AND_INC(UInt16, word);
            Registers[INST_ARG1(util)].Unsigned = word;
            break;
        case codefile::OpCode::Mov32:
            util = *ip++;
            GET_AND_INC(UInt32, dword);
            Registers[INST_ARG1(util)].Unsigned = dword;
            break;
        case codefile::OpCode::Mov64:
            util = *ip++;
            GET_AND_INC(UInt64, qword);
            Registers[INST_ARG1(util)].Unsigned = qword;
            break;
        case codefile::OpCode::Ldstr:
            util = *ip++;
            GET_AND_INC(UInt32, dword);
            Registers[INST_ARG1(util)].Ptr = &Asm->STSection[dword];
            break;
        case codefile::OpCode::Ldr:
            util = *ip++;
            GET_AND_INC(UInt16, word);
            if (INST_ARG2(util) == codefile::BaseSP) {
                Registers[INST_ARG1(util)] = Frame.SP[word];
            }
            else  if (INST_ARG2(util) == codefile::BaseFP) {
                Registers[INST_ARG1(util)] = Frame.FP[word];
            }
            else  if (INST_ARG2(util) == codefile::BaseCS) {
                Registers[INST_ARG1(util)].Unsigned = Asm->DataSection[word].Value.Unsigned;
            }
            break;
        case codefile::OpCode::Str:
            util = *ip++;
            if (INST_ARG2(util) == codefile::BaseSP) {
                Frame.SP[word] = Registers[INST_ARG1(util)];
            }
            else  if (INST_ARG2(util) == codefile::BaseFP) {
                Frame.FP[word] = Registers[INST_ARG1(util)];
            }
            else  if (INST_ARG2(util) == codefile::BaseCS) {
                Asm->DataSection[word].Value.Unsigned = Registers[INST_ARG1(util)].Unsigned;
            }
            break;
        case codefile::OpCode::Cmp:
            util = *ip++;
            MakeComparisionInteger(INST_ARG1(util), INST_ARG2(util));
            break;
        case codefile::OpCode::FCmp:
        {
            union {
                UInt op1_;
                Float op1;
            };

            union {
                UInt op2_;
                Float op2;
            };

            util = *ip++;
            op1_ = INST_ARG1(util);
            op2_ = INST_ARG2(util);
            MakeComparisionFloat(op1, op2);
        }
        break;
        case codefile::OpCode::TestZ:
            util = *ip++;
            if (!Registers[INST_ARG1(util)].Unsigned) {
                CMP |= ZERO_FLAG;
            }
            break;
        case codefile::OpCode::Jmp:
            GET_AND_INC(UInt16, word);
            ip = ip + word;
            break;
        case codefile::OpCode::Je:
            GET_AND_INC(UInt16, word);
            if (CMP & ZERO_FLAG) {
                ip = ip + word;
            }
            break;
        case codefile::OpCode::Jne:
            GET_AND_INC(UInt16, word);
            if (!(CMP & ZERO_FLAG)) {
                ip = ip + word;
            }
            break;
        case codefile::OpCode::Jl:
            GET_AND_INC(UInt16, word);
            if (CMP & SIGN_FLAG) {
                ip = ip + word;
            }
            break;
        case codefile::OpCode::Jle:
            GET_AND_INC(UInt16, word);
            if (CMP & ZERO_FLAG || CMP & SIGN_FLAG) {
                ip = ip + word;
            }
            break;
        case codefile::OpCode::Jg:
            GET_AND_INC(UInt16, word);
            if (!(CMP & ZERO_FLAG) && !(CMP & SIGN_FLAG)) {
                ip = ip + word;
            }
            break;
        case codefile::OpCode::Jge:
            GET_AND_INC(UInt16, word);
            if (!(CMP & SIGN_FLAG)) {
                ip = ip + word;
            }
            break;
        case codefile::OpCode::Call:
        {
            GET_AND_INC(UInt32, dword);
            RuntimeError(qword < Asm->FunctionSize, "Invalid function address %08X", dword);
            Function& target = Asm->CodeSection[dword];
            UInt status = ProcessCall(Frame, target);
            (void)status;
        }
        break;
        case codefile::OpCode::Ret:
            return 0;
        case codefile::OpCode::RetC:
            GET_AND_INC(UInt32, dword);
            return dword;
        case codefile::OpCode::Inc:
            util = *ip++;
            Registers[INST_ARG1(util)].Unsigned++;
            break;
        case codefile::OpCode::IInc:
            util = *ip++;
            Registers[INST_ARG1(util)].Signed++;
            break;
        case codefile::OpCode::FInc:
            util = *ip++;
            Registers[INST_ARG1(util)].Real++;
            break;
        case codefile::OpCode::Dec:
            util = *ip++;
            Registers[INST_ARG1(util)].Unsigned--;
            break;
        case codefile::OpCode::IDec:
            util = *ip++;
            Registers[INST_ARG1(util)].Signed--;
            break;
        case codefile::OpCode::FDec:
            util = *ip++;
            Registers[INST_ARG1(util)].Real--;
            break;
            {
                HANDLE_MATH(Add, +, Unsigned);
                HANDLE_MATH(Sub, -, Unsigned);
                HANDLE_MATH(Mul, *, Unsigned);
                HANDLE_MATH(Div, /, Unsigned);
                HANDLE_MATH(IAdd, +, Signed);
                HANDLE_MATH(ISub, -, Signed);
                HANDLE_MATH(IMul, *, Signed);
                HANDLE_MATH(IDiv, /, Signed);
                HANDLE_MATH(FAdd, +, Real);
                HANDLE_MATH(FSub, -, Real);
                HANDLE_MATH(FMul, *, Real);
                HANDLE_MATH(FDiv, /, Real);

                HANDLE_MATH8(Add8, +, Unsigned, Byte);
                HANDLE_MATH8(Sub8, -, Unsigned, Byte);
                HANDLE_MATH8(Mul8, *, Unsigned, Byte);
                HANDLE_MATH8(Div8, / , Unsigned, Byte);
                HANDLE_MATH8(IAdd8, +, Signed, Int8);
                HANDLE_MATH8(ISub8, -, Signed, Int8);
                HANDLE_MATH8(IMul8, *, Signed, Int8);
                HANDLE_MATH8(IDiv8, / , Signed, Int8);

                HANDLE_MATH16(Add16, +, Unsigned, UInt16);
                HANDLE_MATH16(Sub16, -, Unsigned, UInt16);
                HANDLE_MATH16(Mul16, *, Unsigned, UInt16);
                HANDLE_MATH16(Div16, / , Unsigned, UInt16);
                HANDLE_MATH16(IAdd16, +, Signed, Int16);
                HANDLE_MATH16(ISub16, -, Signed, Int16);
                HANDLE_MATH16(IMul16, *, Signed, Int16);
                HANDLE_MATH16(IDiv16, / , Signed, Int16);

                HANDLE_MATH(Or, |, Unsigned);
                HANDLE_MATH(And, &, Unsigned);
                HANDLE_MATH(XOr, ^, Unsigned);
                HANDLE_MATH(Shl, <<, Unsigned);
                HANDLE_MATH(Shr, >>, Unsigned);

                HANDLE_MATH8(Or8, | , Unsigned, Byte);
                HANDLE_MATH8(And8, &, Unsigned, Byte);
                HANDLE_MATH8(XOr8, ^, Unsigned, Byte);
                HANDLE_MATH8(Shl8, << , Unsigned, Byte);
                HANDLE_MATH8(Shr8, >> , Unsigned, Byte);

                HANDLE_MATH16(Or16, | , Unsigned, UInt16);
                HANDLE_MATH16(And16, &, Unsigned, UInt16);
                HANDLE_MATH16(XOr16, ^, Unsigned, UInt16);
            }
        case codefile::OpCode::Not:
            Registers[INST_ARG1(util)].Unsigned = ~Registers[INST_ARG1(util)].Unsigned;
            break;
        case codefile::OpCode::Neg:
            Registers[INST_ARG1(util)].Signed = -Registers[INST_ARG1(util)].Signed;
            break;
        case codefile::OpCode::Push8:
            Push(Frame, { .Unsigned = *ip++ });
            break;
        case codefile::OpCode::Push16:
            GET_AND_INC(UInt16, word);
            Push(Frame, { .Unsigned = word });
            break;
        case codefile::OpCode::Push32:
            GET_AND_INC(UInt32, dword);
            Push(Frame, { .Unsigned = dword });
            break;
        case codefile::OpCode::Push64:
            GET_AND_INC(UInt32, qword);
            Push(Frame, { .Unsigned = qword });
            break;
        case codefile::OpCode::Popd:
            (void)Pop(Frame);
            break;
        case codefile::OpCode::Push:
            util = *ip++;
            Push(Frame, Registers[INST_ARG1(util)]);
            break;
        case codefile::OpCode::Pop:
            util = *ip++;
            Registers[INST_ARG1(util)] = Pop(Frame);
            break;
        case codefile::OpCode::ArrayNew:
            util = *ip++;
            qword = Registers[INST_ARG1(util)].Unsigned;
            util2 = Byte(Registers[INST_ARG2(util)].Unsigned);

            Registers[INST_ARG1(util)].ArrayRef = new Array(qword, codefile::ArrayElement(util2));
            break;
        case codefile::OpCode::ArrayL:
            util = *ip++;
            array = Registers[INST_ARG1(util)].ArrayRef;
            Registers[INST_ARG2(util)].Unsigned = array->Size;
            break;
        case codefile::OpCode::ArrayLoad:
            util = *ip++;
            util2 = *ip++;
            array = Registers[INST_ARG1(util)].ArrayRef;
            qword = Registers[INST_ARG2(util)].Unsigned;
            CHECK_ARRAY_INDEX(qword);

            if (array->ElementSize == 1) {
                Registers[INST_ARG1(util2)].Unsigned = array->GetByte(qword);
            }
            else if (array->ElementSize == 8) {
                Registers[INST_ARG1(util2)].Unsigned = array->GetUInt(qword);
            }
            break;
        case codefile::OpCode::ArrayStore:
            util = *ip++;
            util2 = *ip++;
            array = Registers[INST_ARG1(util)].ArrayRef;
            qword = Registers[INST_ARG2(util)].Unsigned;
            CHECK_ARRAY_INDEX(qword);

            if (array->ElementSize == 1) {
                array->GetByte(qword) = Byte(Registers[INST_ARG1(util2)].Unsigned);
            }
            else if (array->ElementSize == 8) {
                array->GetUInt(qword) = Registers[INST_ARG1(util2)].Unsigned;
            }

            break;
        case codefile::OpCode::ArrayDestroy:
            util = *ip++;
            array = Registers[INST_ARG1(util)].ArrayRef;
            delete array;
            array = nullptr;
            break;
        default:
            RuntimeError(0, "Invalid OpCode %X\n", Byte(opcode));
            break;
        }
    }
}

}
