#pragma once
#include "jkc/Utility/Memory.h"
#include "jkc/Utility/Slice.h"
#include "jkc/AST/Type.h"
#include "jkc/CodeGen/SymbolTable.h"
#include <jkr/CodeFile/OpCodes.h>
#include <jkr/CodeFile/Type.h>

namespace CodeGen {

struct Constant {
    AST::TypeDecl Type = {};
    union {
        Int Signed = 0;
        UInt Unsigned;
        Float Real;
        void* Array;
    };
};

struct [[nodiscard]] Local {
    Str Name = STR("");
    AST::TypeDecl Type = {};
    union {
        Byte Index = 0;
        Byte Reg;
    };
    bool IsInitialized = false;
    bool IsRegister = false;
};

struct [[nodiscard]] Global {
    Str Name = STR("");
    AST::TypeDecl Type = {};
    UInt32 Index = 0;
    Constant Value;
};

enum class CallConv {
    Stack,
    // The maximum size of arguments is 10 and there are put bettwen the r1-r10 register
    // And the return value if put in the r0 register
    Register,
    // The maximum size of arguments is 32, the first 10 arguments are putted bettwen the r1-r10 register
    // and the rest are putted in the stack from right to left
    // And the return value if put in the r0 register
    RegS,
};

struct AddressToResolve {
    // Pointer in code to the address to resolve,
    // for now used for resolve jumps.
    UInt32 IP; 
};

struct [[nodiscard]] Function {
    Str Name = STR("");

    AST::TypeDecl Type = {};
    MemoryBuffer Code = {};

    List<AddressToResolve, false> ResolveReturns;
    SymbolTable<Local> Locals = {};
    Byte CountOfArguments = 0;
    Byte RegisterArguments = 0;
    Byte StackArguments = 0;
    Byte CountOfStackLocals = 0;
    UInt32 Address = 0;

    UInt32 LibraryAddress = 0;
    UInt32 EntryAddress = 0;

    bool IsDefined = false;
    bool IsNative = false;
    bool HasMultiReturn = false;
    CallConv CC = CallConv::Stack;

    [[nodiscard]] constexpr bool IsRegisterBased() const {
        return CC == CallConv::Register || CC == CallConv::RegS;
    }
};

struct Assembler {
    Assembler() {}
    ~Assembler() {}

    constexpr void Brk(this Assembler& /*Self*/, Function& Fn) { Fn.Code << (Byte)codefile::OpCode::Brk; }

    constexpr void CIL(this Assembler& /*Self*/, Function& Fn, Byte Dest, Byte Src1, Byte Src2, Byte Extra = 0) {
        Fn.Code << UInt16(Dest | (Src1 << 4) | (Src2 << 8) | (Extra<<12));
    }

    constexpr void LocalSet4(this Assembler& /*Self*/, Function& Fn, Byte Src, Byte Idx) {
        Fn.Code << (Byte)codefile::OpCode::LocalSet4;
        Fn.Code << Byte(Src | (Idx << 4));
    }

    constexpr void LocalGet4(this Assembler& /*Self*/, Function& Fn, Byte Dest, Byte Idx) {
        Fn.Code << (Byte)codefile::OpCode::LocalGet4;
        Fn.Code << Byte(Dest | (Idx << 4));
    }

    constexpr void LocalSet(this Assembler& /*Self*/, Function& Fn, Byte Src, Byte Idx) {
        Fn.Code << (Byte)codefile::OpCode::LocalSet;
        Fn.Code << Src;
        Fn.Code << Idx;
    }
    
    constexpr void LocalGet(this Assembler& /*Self*/, Function& Fn, Byte Dest, Byte Idx) {
        Fn.Code << (Byte)codefile::OpCode::LocalGet;
        Fn.Code << Dest;
        Fn.Code << Idx;
    }

    constexpr void GlobalSet(this Assembler& /*Self*/, Function& Fn, Byte Src, UInt32 Idx) {
        Fn.Code << (Byte)codefile::OpCode::GlobalSet;
        Fn.Code << Byte(Src | (Idx << 4));
    }

    constexpr void GlobalGet(this Assembler& /*Self*/, Function& Fn, Byte Dest, UInt32 Idx) {
        Fn.Code << (Byte)codefile::OpCode::GlobalGet;
        Fn.Code << Byte(Dest | (Idx << 4));
    }
    
    constexpr void Mov(this Assembler& /*Self*/, Function& Fn, Byte Dest, Byte Src) {
        Fn.Code << (Byte)codefile::OpCode::Mov;
        Fn.Code << Byte(Dest | (Src<<4));
    }

    constexpr void Const4(this Assembler& /*Self*/, Function& Fn, Byte Dest, Byte Const) {
        Fn.Code << (Byte)codefile::OpCode::Mov4;
        Fn.Code << (Byte)(Dest | (Const << 4));
    }

    constexpr void Const8(this Assembler& /*Self*/, Function& Fn, Byte Dest, Byte Const) {
        Fn.Code << (Byte)codefile::OpCode::Mov8;
        Fn.Code << Dest;
        Fn.Code << Const;
    }

    constexpr void Const16(this Assembler& /*Self*/, Function& Fn, Byte Dest, UInt16 Const) {
        Fn.Code << (Byte)codefile::OpCode::Mov16;
        Fn.Code << Dest;
        Fn.Code << Const;
    }

    constexpr void Const32(this Assembler& /*Self*/, Function& Fn, Byte Dest, UInt32 Const) {
        Fn.Code << (Byte)codefile::OpCode::Mov32;
        Fn.Code << Dest;
        Fn.Code << Const;
    }

    constexpr void Const64(this Assembler& /*Self*/, Function& Fn, Byte Dest, UInt64 Const) {
        Fn.Code << (Byte)codefile::OpCode::Mov64;
        Fn.Code << Dest;
        Fn.Code << Const;
    }

    constexpr void MovRes(this Assembler& /*Self*/, Function& Fn, Byte Dest) {
        Fn.Code << (Byte)codefile::OpCode::MovRes;
        Fn.Code << Dest;
    }

    constexpr void Inc(this Assembler& /*Self*/, Function& Fn, Byte Reg) {
        Fn.Code << (Byte)codefile::OpCode::Inc;
        Fn.Code << Byte(Reg | 0b00010000);
    }

    constexpr void IInc(this Assembler& /*Self*/, Function& Fn, Byte Reg) {
        Fn.Code << (Byte)codefile::OpCode::Inc;
        Fn.Code << Reg;
    }

    constexpr void FInc(this Assembler& /*Self*/, Function& Fn, Byte Reg) {
        Fn.Code << (Byte)codefile::OpCode::Inc;
        Fn.Code << Byte(Reg | 0b00100000);
    }

    constexpr void Dec(this Assembler& /*Self*/, Function& Fn, Byte Reg) {
        Fn.Code << (Byte)codefile::OpCode::Dec;
        Fn.Code << Byte(Reg | 0b00010000);
    }

    constexpr void IDec(this Assembler& /*Self*/, Function& Fn, Byte Reg) {
        Fn.Code << (Byte)codefile::OpCode::Dec;
        Fn.Code << Reg;
    }

    constexpr void FDec(this Assembler& /*Self*/, Function& Fn, Byte Reg) {
        Fn.Code << (Byte)codefile::OpCode::Dec;
        Fn.Code << Byte(Reg | 0b00100000);
    }

    constexpr void Add(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << (Byte)codefile::OpCode::Add;
        Self.CIL(Fn, Dest, Src1, Src2);
    }

    constexpr void Add8(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, Byte Constant) {
        Fn.Code << (Byte)codefile::OpCode::Add8;
        Self.CIL(Fn, Dest, Src1, Constant & 0xF, Constant >> 4);
    }

    constexpr void Add16(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, UInt16 Constant) {
        Fn.Code << (Byte)codefile::OpCode::Add16;
        Self.CIL(Fn, Dest, Src1, Byte(Constant & 0xF), Byte(Constant >> 4));
        Fn.Code << Byte(Constant >> 8);
    }

    constexpr void IAdd(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << (Byte)codefile::OpCode::IAdd;
        Self.CIL(Fn, Dest, Src1, Src2);
    }

    constexpr void IAdd8(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, Byte Constant) {
        Fn.Code << (Byte)codefile::OpCode::IAdd8;
        Self.CIL(Fn, Dest, Src1, Constant & 0xF, Constant >> 4);
    }

    constexpr void IAdd16(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, UInt16 Constant) {
        Fn.Code << (Byte)codefile::OpCode::IAdd16;
        Self.CIL(Fn, Dest, Src1, Byte(Constant & 0xF), Byte(Constant >> 4));
        Fn.Code << Byte(Constant >> 8);
    }

    constexpr void FAdd(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << (Byte)codefile::OpCode::FAdd;
        Self.CIL(Fn, Dest, Src1, Src2);
    }

    constexpr void Sub(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << (Byte)codefile::OpCode::Sub;
        Self.CIL(Fn, Dest, Src1, Src2);
    }

    constexpr void Sub8(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, Byte Constant) {
        Fn.Code << (Byte)codefile::OpCode::Sub8;
        Self.CIL(Fn, Dest, Src1, Constant & 0xF, Constant >> 4);
    }

    constexpr void Sub16(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, UInt16 Constant) {
        Fn.Code << (Byte)codefile::OpCode::Sub16;
        Self.CIL(Fn, Dest, Src1, Byte(Constant & 0xF), Byte(Constant >> 4));
        Fn.Code << Byte(Constant >> 8);
    }

    constexpr void ISub(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << (Byte)codefile::OpCode::ISub;
        Self.CIL(Fn, Dest, Src1, Src2);
    }

    constexpr void ISub8(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, Byte Constant) {
        Fn.Code << (Byte)codefile::OpCode::ISub8;
        Self.CIL(Fn, Dest, Src1, Constant & 0xF, Constant >> 4);
    }

    constexpr void ISub16(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, UInt16 Constant) {
        Fn.Code << (Byte)codefile::OpCode::ISub16;
        Self.CIL(Fn, Dest, Src1, Byte(Constant & 0xF), Byte(Constant >> 4));
        Fn.Code << Byte(Constant >> 8);
    }

    constexpr void FSub(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << (Byte)codefile::OpCode::FSub;
        Self.CIL(Fn, Dest, Src1, Src2);
    }

    constexpr void Mul(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << (Byte)codefile::OpCode::Mul;
        Self.CIL(Fn, Dest, Src1, Src2);
    }

    constexpr void Mul8(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, Byte Constant) {
        Fn.Code << (Byte)codefile::OpCode::Mul8;
        Self.CIL(Fn, Dest, Src1, Constant & 0xF, Constant >> 4);
    }

    constexpr void Mul16(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, UInt16 Constant) {
        Fn.Code << (Byte)codefile::OpCode::Mul16;
        Self.CIL(Fn, Dest, Src1, Byte(Constant & 0xF), Byte(Constant >> 4));
        Fn.Code << Byte(Constant >> 8);
    }

    constexpr void IMul(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << (Byte)codefile::OpCode::IMul;
        Self.CIL(Fn, Dest, Src1, Src2);
    }

    constexpr void IMul8(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, Byte Constant) {
        Fn.Code << (Byte)codefile::OpCode::IMul8;
        Self.CIL(Fn, Dest, Src1, Constant & 0xF, Constant >> 4);
    }

    constexpr void IMul16(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, UInt16 Constant) {
        Fn.Code << (Byte)codefile::OpCode::IMul16;
        Self.CIL(Fn, Dest, Src1, Byte(Constant & 0xF), Byte(Constant >> 4));
        Fn.Code << Byte(Constant >> 8);
    }

    constexpr void FMul(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << (Byte)codefile::OpCode::FMul;
        Self.CIL(Fn, Dest, Src1, Src2);
    }

    constexpr void Div(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << (Byte)codefile::OpCode::Div;
        Self.CIL(Fn, Dest, Src1, Src2);
    }

    constexpr void Div8(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, Byte Constant) {
        Fn.Code << (Byte)codefile::OpCode::Div8;
        Self.CIL(Fn, Dest, Src1, Constant & 0xF, Constant >> 4);
    }

    constexpr void Div16(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, UInt16 Constant) {
        Fn.Code << (Byte)codefile::OpCode::Div16;
        Self.CIL(Fn, Dest, Src1, Byte(Constant & 0xF), Byte(Constant >> 4));
        Fn.Code << Byte(Constant >> 8);
    }

    constexpr void IDiv(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << (Byte)codefile::OpCode::IDiv;
        Self.CIL(Fn, Dest, Src1, Src2);
    }

    constexpr void IDiv8(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, Byte Constant) {
        Fn.Code << (Byte)codefile::OpCode::IDiv16;
        Self.CIL(Fn, Dest, Src1, Constant & 0xF, Constant >> 4);
    }

    constexpr void IDiv16(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, UInt16 Constant) {
        Fn.Code << (Byte)codefile::OpCode::IDiv16;
        Self.CIL(Fn, Dest, Src1, Byte(Constant & 0xF), Byte(Constant >> 4));
        Fn.Code << Byte(Constant >> 8);
    }

    constexpr void FDiv(this Assembler& Self, Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << (Byte)codefile::OpCode::FDiv;
        Self.CIL(Fn, Dest, Src1, Src2);
    }

    constexpr void Cmp(this Assembler& /*Self*/, Function& Fn, Byte L, Byte R) {
        Fn.Code << (Byte)codefile::OpCode::Cmp;
        Fn.Code << (Byte)(L | (R << 4));
    }

    constexpr void ICmp(this Assembler& /*Self*/, Function& Fn, Byte L, Byte R) {
        Fn.Code << (Byte)codefile::OpCode::ICmp;
        Fn.Code << (Byte)(L | (R << 4));
    }

    constexpr void FCmp(this Assembler& /*Self*/, Function& Fn, Byte L, Byte R) {
        Fn.Code << (Byte)codefile::OpCode::FCmp;
        Fn.Code << (Byte)(L | (R << 4));
    }

    constexpr void TestZ(this Assembler& /*Self*/, Function& Fn, Byte R) {
        Fn.Code << (Byte)codefile::OpCode::TestZ;
        Fn.Code << R;
    }

    constexpr void Jmp(this Assembler& /*Self*/, Function& Fn, codefile::OpCode JmpType, 
                       UInt16 Address) {
        Fn.Code << (Byte)JmpType;
        Fn.Code << Address;
    }

    constexpr void Jmp8(this Assembler& /*Self*/, Function& Fn, codefile::OpCode JmpType, Byte Address) {
        Fn.Code << (Byte)JmpType;
        Fn.Code << Address;
    }

    constexpr void Call(this Assembler& /*Self*/, Function& Fn, UInt32 Index) {
        Fn.Code << (Byte)codefile::OpCode::Call;
        Fn.Code << Index;
    }

    constexpr void Call8(this Assembler& /*Self*/, Function& Fn, Byte Index) {
        Fn.Code << (Byte)codefile::OpCode::Call8;
        Fn.Code << Index;
    }

    constexpr void Ret(this Assembler& /*Self*/, Function& Fn) {
        Fn.Code << (Byte)codefile::OpCode::Ret;
    }

    constexpr void ArrayNew(this Assembler& /*Self*/, Function& Fn, Byte Dest, Byte SizeInReg, codefile::PrimitiveType Type) {
        Fn.Code << (Byte)codefile::OpCode::ArrayNew;
        Fn.Code << Byte(Dest | (SizeInReg << 4));
        Fn.Code << Byte(Type);
    }

    constexpr void ArrayLen(this Assembler& /*Self*/, Function& Fn, Byte ArrayInReg, Byte Dest) {
        Fn.Code << (Byte)codefile::OpCode::ArrayLen;
        Fn.Code << Byte(ArrayInReg | (Dest << 4));
    }

    constexpr void ArraySet(this Assembler& /*Self*/, Function& Fn, Byte ArrayInReg, Byte IndexInReg, Byte SrcInReg) {
        Fn.Code << (Byte)codefile::OpCode::ArraySet;
        Fn.Code << Byte(ArrayInReg | (IndexInReg << 4));
        Fn.Code << SrcInReg;
    }

    constexpr void ArrayGet(this Assembler& /*Self*/, Function& Fn, Byte ArrayInReg, Byte IndexInReg, Byte Dest) {
        Fn.Code << (Byte)codefile::OpCode::ArrayGet;
        Fn.Code << Byte(ArrayInReg | (IndexInReg << 4));
        Fn.Code << Dest;
    }

    constexpr void ArrayDestroy(this Assembler& /*Self*/, Function& Fn, Byte Src) {
        Fn.Code << (Byte)codefile::OpCode::ArrayDestroy;
        Fn.Code << Src;
    }

    constexpr void StringGet4(this Assembler& /*Self*/, Function& Fn, Byte Reg, Byte S) {
        Fn.Code << (Byte)codefile::OpCode::StringGet4;
        Fn.Code << Byte(Reg | (S << 4));
    }

    constexpr void StringGet(this Assembler& /*Self*/, Function& Fn, Byte Reg, UInt32 S) {
        Fn.Code << (Byte)codefile::OpCode::StringGet;
        Fn.Code << Byte(Reg | (S<<4));
    }

    constexpr void Push8(this Assembler& /*Self*/, Function& Fn, Byte C) {
        Fn.Code << (Byte)codefile::OpCode::Push8;
        Fn.Code << C;
    }

    constexpr void Push16(this Assembler& /*Self*/, Function& Fn, UInt16 C) {
        Fn.Code << (Byte)codefile::OpCode::Push16;
        Fn.Code << C;
    }

    constexpr void Push32(this Assembler& /*Self*/, Function& Fn, UInt32 C) {
        Fn.Code << (Byte)codefile::OpCode::Push32;
        Fn.Code << C;
    }

    constexpr void Push64(this Assembler& /*Self*/, Function& Fn, UInt64 C) {
        Fn.Code << (Byte)codefile::OpCode::Push64;
        Fn.Code << C;
    }

    constexpr void PopTop(this Assembler& /*Self*/, Function& Fn) {
        Fn.Code << (Byte)codefile::OpCode::PopTop;
    }
    
    constexpr void Push(this Assembler& /*Self*/, Function& Fn, Byte Src) {
        Fn.Code << Byte(Byte(codefile::OpCode::PushR0) + Src);
    }

    constexpr void Pop(this Assembler& /*Self*/, Function& Fn, Byte Dest) {
        Fn.Code << Byte(Byte(codefile::OpCode::PopR0) + Dest);
    }

    constexpr void PushLocal(this Assembler& /*Self*/, Function& Fn, Byte Src) {
        Fn.Code << Byte(codefile::OpCode::PushLocal);
        Fn.Code << Src;
    }

    constexpr void PopLocal(this Assembler& /*Self*/, Function& Fn, Byte Dest) {
        Fn.Code << Byte(codefile::OpCode::PopLocal);
        Fn.Code << Dest;
    }

};

}
