#pragma once
#include "jkc/Utility/Memory.h"
#include "jkc/Utility/Slice.h"
#include "jkc/AST/Type.h"
#include "jkc/CodeGen/SymbolTable.h"
#include <jkr/CodeFile/OpCodes.h>

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
        codefile::LocalType Index = 0;
        Byte Reg;
    };
    bool IsInitialized = false;
    bool IsRegister = false;
};

struct [[nodiscard]] Global {
    Str Name = STR("");
    AST::TypeDecl Type = {};
    codefile::GlobalType Index = 0;
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

struct [[nodiscard]] Function {
    Str Name = STR("");

    AST::TypeDecl Type = {};
    MemoryBuffer Code = {};

    SymbolTable<Local> Locals = {};
    Byte CountOfArguments = 0;
    Byte RegisterArguments = 0;
    Byte StackArguments = 0;
    Byte CountOfStackLocals = 0;
    codefile::FunctionType Index = 0;

    codefile::StringType LibraryIndex;
    codefile::StringType EntryIndex;

    bool IsDefined = false;
    bool IsNative = false;
    CallConv CC = CallConv::Stack;

    [[nodiscard]] constexpr bool IsRegisterBased() const {
        return CC == CallConv::Register || CC == CallConv::RegS;
    }
};

struct Assembler {
    Assembler() {}
    ~Assembler() {}

    constexpr void Brk(this Assembler& /*Self*/, Function& Fn) { Fn.Code << (Byte)codefile::OpCode::Brk; }
    
    constexpr void BIL(this Assembler& /*Self*/, Function& Fn, Byte Dest, Byte Src) {
        codefile::MIL bil = {
            .Dest = Dest,
            .Src = Src,
        };
        Fn.Code.Write((Byte*)&bil, sizeof(codefile::MIL));
    }

    constexpr void MIL(this Assembler& /*Self*/, Function& Fn, Byte Dest, Byte Src) {
        codefile::MIL mil = {
            .Dest = Dest,
            .Src = Src,
        };
        Fn.Code.Write((Byte*)&mil, sizeof(codefile::MIL));
    }

    constexpr void CIL(this Assembler& /*Self*/, Function& Fn, Byte Dest, Byte Src1, Byte Src2, Byte Extra = 0) {
        codefile::CIL cil = {
            .Dest = Dest,
            .Src1 = Src1,
            .Src2 = Src2,
            .Extra = Extra,
        };
        Fn.Code.Write((Byte*)&cil, sizeof(codefile::CIL));
    }

    constexpr void LIL(this Assembler& /*Self*/, Function& Fn, codefile::LocalType Idx, Byte SrcDest) {
        codefile::LIL lil = {
            .SrcDest = SrcDest,
            .Index = Idx,
        };
        Fn.Code.Write((Byte*)&lil, sizeof(codefile::LIL));
    }

    constexpr void GIL(this Assembler& /*Self*/, Function& Fn, codefile::GlobalType Idx, Byte SrcDest) {
        codefile::GIL gil = {
            .SrcDest = SrcDest,
            .Index = Idx,
        };
     
        Fn.Code.Write((Byte*)&gil, sizeof(codefile::GIL));
    }

    constexpr void LocalSet4(this Assembler& /*Self*/, Function& Fn, Byte Src, Byte Idx) {
        Fn.Code << (Byte)codefile::OpCode::LocalSet4;
        Fn.Code << Byte(Src | (Idx >> 4));
    }

    constexpr void LocalGet4(this Assembler& /*Self*/, Function& Fn, Byte Dest, Byte Idx) {
        Fn.Code << (Byte)codefile::OpCode::LocalGet4;
        Fn.Code << Byte(Dest | (Idx >> 4));
    }

    constexpr void LocalSet(this Assembler& Self, Function& Fn, Byte Src, codefile::LocalType Idx) {
        Fn.Code << (Byte)codefile::OpCode::LocalSet;
        Self.LIL(Fn, Idx, Src);
    }
    
    constexpr void LocalGet(this Assembler& Self, Function& Fn, Byte Dest, codefile::LocalType Idx) {
        Fn.Code << (Byte)codefile::OpCode::LocalGet;
        Self.LIL(Fn, Idx, Dest);
    }

    constexpr void GlobalSet(this Assembler& Self, Function& Fn, Byte Src, codefile::GlobalType Idx) {
        Fn.Code << (Byte)codefile::OpCode::GlobalSet;
        Self.GIL(Fn, Idx, Src);
    }

    constexpr void GlobalGet(this Assembler& Self, Function& Fn, Byte Dest, codefile::GlobalType Idx) {
        Fn.Code << (Byte)codefile::OpCode::GlobalGet;
        Self.GIL(Fn, Idx, Dest);
    }

    constexpr void Mov(this Assembler& Self, Function& Fn, Byte Dest, Byte Src) {
        Fn.Code << (Byte)codefile::OpCode::Mov;
        Self.MIL(Fn, Dest, Src);
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
        Fn.Code << Reg;
    }

    constexpr void IInc(this Assembler& /*Self*/, Function& Fn, Byte Reg) {
        Fn.Code << (Byte)codefile::OpCode::IInc;
        Fn.Code << Reg;
    }

    constexpr void FInc(this Assembler& /*Self*/, Function& Fn, Byte Reg) {
        Fn.Code << (Byte)codefile::OpCode::FInc;
        Fn.Code << Reg;
    }

    constexpr void Dec(this Assembler& /*Self*/, Function& Fn, Byte Reg) {
        Fn.Code << (Byte)codefile::OpCode::Dec;
        Fn.Code << Reg;
    }

    constexpr void IDec(this Assembler& /*Self*/, Function& Fn, Byte Reg) {
        Fn.Code << (Byte)codefile::OpCode::IDec;
        Fn.Code << Reg;
    }

    constexpr void FDec(this Assembler& /*Self*/, Function& Fn, Byte Reg) {
        Fn.Code << (Byte)codefile::OpCode::FDec;
        Fn.Code << Reg;
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

    constexpr void Cmp(this Assembler& Self, Function& Fn, Byte L, Byte R) {
        Fn.Code << (Byte)codefile::OpCode::Cmp;
        Self.BIL(Fn, L, R);
    }

    constexpr void ICmp(this Assembler& Self, Function& Fn, Byte L, Byte R) {
        Fn.Code << (Byte)codefile::OpCode::ICmp;
        Self.BIL(Fn, L, R);
    }

    constexpr void FCmp(this Assembler& Self, Function& Fn, Byte L, Byte R) {
        Fn.Code << (Byte)codefile::OpCode::FCmp;
        Self.BIL(Fn, L, R);
    }

    constexpr void Jmp(this Assembler& /*Self*/, Function& Fn, codefile::OpCode JmpType, codefile::AddressType Address) {
        Fn.Code << (Byte)JmpType;
        Fn.Code << Address;
    }

    constexpr void Jmp8(this Assembler& /*Self*/, Function& Fn, codefile::OpCode JmpType, Byte Address) {
        Fn.Code << (Byte)JmpType;
        Fn.Code << Address;
    }

    constexpr void Call(this Assembler& /*Self*/, Function& Fn, codefile::FunctionType Index) {
        Fn.Code << (Byte)codefile::OpCode::Call;
        Fn.Code << Index;
    }

    constexpr void Call8(this Assembler& /*Self*/, Function& Fn, Byte Index) {
        Fn.Code << (Byte)codefile::OpCode::Call8;
        Fn.Code << Index;
    }

    constexpr void RetVoid(this Assembler& /*Self*/, Function& Fn) {
        Fn.Code << (Byte)codefile::OpCode::RetVoid;
    }

    constexpr void Ret(this Assembler& /*Self*/, Function& Fn, Byte Dest) {
        Fn.Code << (Byte)codefile::OpCode::Ret;
        Fn.Code << Dest;
    }

    constexpr void Ret8(this Assembler& /*Self*/, Function& Fn, Byte Constant) {
        Fn.Code << (Byte)codefile::OpCode::Ret8;
        Fn.Code << Constant;
    }

    constexpr void Ret16(this Assembler& /*Self*/, Function& Fn, UInt16 Constant) {
        Fn.Code << (Byte)codefile::OpCode::Ret16;
        Fn.Code << Constant;
    }

    constexpr void RetLocal(this Assembler& /*Self*/, Function& Fn, codefile::LocalType Index) {
        Fn.Code << (Byte)codefile::OpCode::RetLocal;
        Fn.Code << Index;
    }

    constexpr void RetGlobal(this Assembler& /*Self*/, Function& Fn, codefile::GlobalType Index) {
        Fn.Code << (Byte)codefile::OpCode::RetGlobal;
        Fn.Code << Index;
    }

    constexpr void StringGet4(this Assembler& /*Self*/, Function& Fn, Byte Reg, Byte S) {
        Fn.Code << (Byte)codefile::OpCode::StringGet4;
        Fn.Code << Byte(Reg | (S << 4));
    }

    constexpr void StringGet(this Assembler& /*Self*/, Function& Fn, Byte Reg, codefile::StringType S) {
        Fn.Code << (Byte)codefile::OpCode::StringGet;
        Fn.Code << Reg;
        Fn.Code << S;
    }

    constexpr void Push(this Assembler& /*Self*/, Function& Fn, Byte Reg) {
        Fn.Code << (Byte)codefile::OpCode::Push;
        Fn.Code << Reg;
    }

    constexpr void LocalPush(this Assembler& /*Self*/, Function& Fn, codefile::LocalType Idx) {
        Fn.Code << (Byte)codefile::OpCode::LocalPush;
        Fn.Code << Idx;
    }

    constexpr void GlobalPush(this Assembler& /*Self*/, Function& Fn, codefile::GlobalType Idx) {
        Fn.Code << (Byte)codefile::OpCode::GlobalPush;
        Fn.Code << Idx;
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

    constexpr void Pop(this Assembler& /*Self*/, Function& Fn, Byte Dest) {
        Fn.Code << (Byte)codefile::OpCode::Pop;
        Fn.Code << Dest;
    }

};

}
