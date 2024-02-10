#pragma once
#include "jkc/Utility/Memory.h"
#include "jkc/Utility/Slice.h"
#include "jkc/AST/Type.h"
#include "jkc/CodeGen/SymbolTable.h"
#include <jkr/CodeFile/OpCodes.h>
#include <string_view>

namespace CodeGen {

struct [[nodiscard]] JKLocal {
    Str Name;
    AST::TypeDecl Type;
    Uint16 Index;
};

struct [[nodiscard]] JKGlobal {
    Str Name;
    AST::TypeDecl Type;
    Uint32 Index;
};

struct [[nodiscard]] JKFunction {
    Str Name;
    AST::TypeDecl Type;
    MemoryBuffer Code;
    Byte CountOfArguments;
    SymbolTable<JKLocal> Locals;
    Uint32 Index;

    struct {
        Str Entry;
        Str Library;
    } NativeInfo;
};

struct RegisterInfo {
    Byte Index;
    bool IsAllocated;
};

struct JKRAssembler {

    JKRAssembler() {}

    ~JKRAssembler() {}

    constexpr void Brk(this JKRAssembler& /*Self*/, JKFunction& Fn) { Fn.Code << (Byte)codefile::OpCode::Brk; }
    
    constexpr void LI(this JKRAssembler& /*Self*/, JKFunction& Fn, Byte Dest, Byte Src) {
        codefile::LI li = {
            .Dest = Dest,
            .Src = Src,
        };
        Fn.Code.Write((Byte*)&li, sizeof(codefile::LI));
    }

    constexpr void CIL(this JKRAssembler& /*Self*/, JKFunction& Fn, Byte Dest, Byte Src1, Byte Src2) {
        codefile::CIL cil = {
            .Dest = Dest,
            .Src1 = Src1,
            .Src2 = Src2
        };
        Fn.Code.Write((Byte*)&cil, sizeof(codefile::CIL));
    }

    constexpr void LIL(this JKRAssembler& /*Self*/, JKFunction& Fn, Uint16 Idx, Byte SrcDest) {
        codefile::LIL lil = {
            .SrcDest = SrcDest,
        };
        *(Uint16*)&lil.Idx1 = Idx;

        Fn.Code.Write((Byte*)&lil, sizeof(codefile::LIL));
    }

    constexpr void GIL(this JKRAssembler& /*Self*/, JKFunction& Fn, Uint32 Idx, Byte SrcDest) {
        codefile::GIL gil = {
            .SrcDest = SrcDest,
        };
        *(Uint32*)&gil.Idx1 = Idx;
     
        Fn.Code.Write((Byte*)&gil, sizeof(codefile::GIL));
    }

    constexpr void LocalSet(this JKRAssembler& Self, JKFunction& Fn, Byte Src, Uint16 Idx) {
        Fn.Code << (Byte)codefile::OpCode::LocalSet;
        Self.LIL(Fn, Idx, Src);
    }
    
    constexpr void LocalGet(this JKRAssembler& Self, JKFunction& Fn, Byte Dest, Uint16 Idx) {
        Fn.Code << (Byte)codefile::OpCode::LocalGet;
        Self.LIL(Fn, Idx, Dest);
    }

    constexpr void GlobalSet(this JKRAssembler& Self, JKFunction& Fn, Byte Src, Uint32 Idx) {
        Fn.Code << (Byte)codefile::OpCode::GlobalSet;
        Self.GIL(Fn, Idx, Src);
    }

    constexpr void GlobalGet(this JKRAssembler& Self, JKFunction& Fn, Byte Dest, Uint32 Idx) {
        Fn.Code << (Byte)codefile::OpCode::GlobalGet;
        Self.GIL(Fn, Idx, Dest);
    }

    constexpr void Mov(this JKRAssembler& Self, JKFunction& Fn, Byte Dest, Byte Src) {
        Fn.Code << (Byte)codefile::OpCode::Mov;
        Self.CIL(Fn, Dest, Src, 0);
    }

    constexpr void Mov8(this JKRAssembler& /*Self*/, JKFunction& Fn, Byte Dest, Byte Const) {
        Fn.Code << (Byte)codefile::OpCode::Mov8;
        Fn.Code << Dest;
        Fn.Code << Const;
    }

    constexpr void Mov16(this JKRAssembler& /*Self*/, JKFunction& Fn, Byte Dest, Uint16 Const) {
        Fn.Code << (Byte)codefile::OpCode::Mov16;
        Fn.Code << Dest;
        Fn.Code << Const;
    }

    constexpr void Mov32(this JKRAssembler& /*Self*/, JKFunction& Fn, Byte Dest, Uint32 Const) {
        Fn.Code << (Byte)codefile::OpCode::Mov32;
        Fn.Code << Dest;
        Fn.Code << Const;
    }

    constexpr void Mov64(this JKRAssembler& /*Self*/, JKFunction& Fn, Byte Dest, Uint64 Const) {
        Fn.Code << (Byte)codefile::OpCode::Mov64;
        Fn.Code << Dest;
        Fn.Code << Const;
    }

    constexpr void RMov(this JKRAssembler& /*Self*/, JKFunction& Fn, Byte Dest) {
        Fn.Code << (Byte)codefile::OpCode::RMov;
        Fn.Code << Dest;
    }

    constexpr void Add(this JKRAssembler& Self, JKFunction& Fn, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << (Byte)codefile::OpCode::Add;
        Self.CIL(Fn, Dest, Src1, Src2);
    }

    constexpr void IAdd(this JKRAssembler& Self, JKFunction& Fn, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << (Byte)codefile::OpCode::IAdd;
        Self.CIL(Fn, Dest, Src1, Src2);
    }

    constexpr void FAdd(this JKRAssembler& Self, JKFunction& Fn, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << (Byte)codefile::OpCode::FAdd;
        Self.CIL(Fn, Dest, Src1, Src2);
    }

    constexpr void Sub(this JKRAssembler& Self, JKFunction& Fn, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << (Byte)codefile::OpCode::Sub;
        Self.CIL(Fn, Dest, Src1, Src2);
    }

    constexpr void ISub(this JKRAssembler& Self, JKFunction& Fn, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << (Byte)codefile::OpCode::ISub;
        Self.CIL(Fn, Dest, Src1, Src2);
    }

    constexpr void FSub(this JKRAssembler& Self, JKFunction& Fn, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << (Byte)codefile::OpCode::FSub;
        Self.CIL(Fn, Dest, Src1, Src2);
    }

    constexpr void Mul(this JKRAssembler& Self, JKFunction& Fn, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << (Byte)codefile::OpCode::Mul;
        Self.CIL(Fn, Dest, Src1, Src2);
    }

    constexpr void IMul(this JKRAssembler& Self, JKFunction& Fn, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << (Byte)codefile::OpCode::IMul;
        Self.CIL(Fn, Dest, Src1, Src2);
    }

    constexpr void FMul(this JKRAssembler& Self, JKFunction& Fn, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << (Byte)codefile::OpCode::FMul;
        Self.CIL(Fn, Dest, Src1, Src2);
    }

    constexpr void Div(this JKRAssembler& Self, JKFunction& Fn, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << (Byte)codefile::OpCode::Div;
        Self.CIL(Fn, Dest, Src1, Src2);
    }

    constexpr void IDiv(this JKRAssembler& Self, JKFunction& Fn, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << (Byte)codefile::OpCode::IDiv;
        Self.CIL(Fn, Dest, Src1, Src2);
    }

    constexpr void FDiv(this JKRAssembler& Self, JKFunction& Fn, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << (Byte)codefile::OpCode::FDiv;
        Self.CIL(Fn, Dest, Src1, Src2);
    }

    constexpr void Cmp(this JKRAssembler& Self, JKFunction& Fn, Byte L, Byte R) {
        Fn.Code << (Byte)codefile::OpCode::Cmp;
        Self.LI(Fn, L, R);
    }

    constexpr void ICmp(this JKRAssembler& Self, JKFunction& Fn, Byte L, Byte R) {
        Fn.Code << (Byte)codefile::OpCode::ICmp;
        Self.LI(Fn, L, R);
    }

    constexpr void FCmp(this JKRAssembler& Self, JKFunction& Fn, Byte L, Byte R) {
        Fn.Code << (Byte)codefile::OpCode::FCmp;
        Self.LI(Fn, L, R);
    }

    constexpr void Jmp(this JKRAssembler& /*Self*/, JKFunction& Fn, codefile::OpCode JmpType, Uint32 Address) {
        Fn.Code << (Byte)JmpType;
        Fn.Code << Address;
    }

    constexpr void Call(this JKRAssembler& /*Self*/, JKFunction& Fn, Uint32 Index) {
        Fn.Code << (Byte)codefile::OpCode::Call;
        Fn.Code << Index;
    }

    constexpr void Ret(this JKRAssembler& /*Self*/, JKFunction& Fn) {
        Fn.Code << (Byte)codefile::OpCode::Ret;
    }

    constexpr void RRet(this JKRAssembler& /*Self*/, JKFunction& Fn, Byte Dest) {
        Fn.Code << (Byte)codefile::OpCode::RRet;
        Fn.Code << Dest;
    }

    constexpr void LRet(this JKRAssembler& Self, JKFunction& Fn, Uint16 Idx) {
        Fn.Code << (Byte)codefile::OpCode::LRet;
        Self.LIL(Fn, Idx, 0);
    }

    constexpr void GRet(this JKRAssembler& Self, JKFunction& Fn, Uint32 Idx) {
        Fn.Code << (Byte)codefile::OpCode::GRet;
        Self.GIL(Fn, Idx, 0);
    }

    constexpr void RPush(this JKRAssembler& /*Self*/, JKFunction& Fn, Byte Reg) {
        Fn.Code << (Byte)codefile::OpCode::StackPrefix;
        Fn.Code << (Byte)codefile::OpCodeStack::RPush;
        Fn.Code << Reg;
    }

    constexpr void LPush(this JKRAssembler& /*Self*/, JKFunction& Fn, Uint16 Idx) {
        Fn.Code << (Byte)codefile::OpCode::StackPrefix;
        Fn.Code << (Byte)codefile::OpCodeStack::LPush;
        Fn.Code << Idx;
    }

    constexpr void LPush(this JKRAssembler& /*Self*/, JKFunction& Fn, Uint32 Idx) {
        Fn.Code << (Byte)codefile::OpCode::StackPrefix;
        Fn.Code << (Byte)codefile::OpCodeStack::GPush;
        Fn.Code << Idx;
    }

    constexpr void Push8(this JKRAssembler& /*Self*/, JKFunction& Fn, Byte C) {
        Fn.Code << (Byte)codefile::OpCode::StackPrefix;
        Fn.Code << (Byte)codefile::OpCodeStack::Push8;
        Fn.Code << C;
    }

    constexpr void Push16(this JKRAssembler& /*Self*/, JKFunction& Fn, Uint16 C) {
        Fn.Code << (Byte)codefile::OpCode::StackPrefix;
        Fn.Code << (Byte)codefile::OpCodeStack::Push16;
        Fn.Code << C;
    }

    constexpr void Push32(this JKRAssembler& /*Self*/, JKFunction& Fn, Uint32 C) {
        Fn.Code << (Byte)codefile::OpCode::StackPrefix;
        Fn.Code << (Byte)codefile::OpCodeStack::Push32;
        Fn.Code << C;
    }

    constexpr void Push64(this JKRAssembler& /*Self*/, JKFunction& Fn, Uint64 C) {
        Fn.Code << (Byte)codefile::OpCode::StackPrefix;
        Fn.Code << (Byte)codefile::OpCodeStack::Push64;
        Fn.Code << C;
    }

    constexpr void PopTop(this JKRAssembler& /*Self*/, JKFunction& Fn) {
        Fn.Code << (Byte)codefile::OpCode::StackPrefix;
        Fn.Code << (Byte)codefile::OpCodeStack::PopTop;
    }

    constexpr void RPop(this JKRAssembler& /*Self*/, JKFunction& Fn, Byte Dest) {
        Fn.Code << (Byte)codefile::OpCode::StackPrefix;
        Fn.Code << (Byte)codefile::OpCodeStack::RPop;
        Fn.Code << Dest;
    }

};

}
