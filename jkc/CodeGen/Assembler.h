#pragma once
#include "jkc/AST/Type.h"
#include "jkc/CodeGen/CodeBuffer.h"
#include "jkc/CodeGen/SymbolTable.h"
#include "jkc/CodeGen/Function.h"
#include <jkr/CodeFile/OpCodes.h>
#include <jkr/CodeFile/Array.h>

namespace CodeGen {

struct Assembler {
    Assembler() {}
    ~Assembler() {}

    constexpr void Brk(Function& Fn) { Fn.Code << UInt32(codefile::OpCode::Brk); }

    constexpr void LDR(Function& Fn, Byte Dest, Byte BT, UInt16 Imm) {
        Fn.Code << Byte(codefile::OpCode::Ldr);
        Fn.Code << Byte(Dest | (BT << 4));
        Fn.Code << Imm;
    }

    constexpr void STR(Function& Fn, Byte Src, Byte BT, UInt16 Imm) {
        Fn.Code << Byte(codefile::OpCode::Str);
        Fn.Code << Byte(Src | (BT << 4));
        Fn.Code << Imm;
    }

    constexpr void SML(Function& Fn, codefile::OpCode I, Byte Dest, Byte Src1, Byte Src2) {
        Fn.Code << Byte(I);
        Fn.Code << Byte(Dest | (Src1 << 4));
        Fn.Code << Src2;
    }

    constexpr void SML8(Function& Fn, codefile::OpCode I, Byte Dest, Byte Src1, UInt8 Imm) {
        Fn.Code << Byte(I);
        Fn.Code << Byte(Dest | (Src1 << 4));
        Fn.Code << Imm;
    }

    constexpr void SML16(Function& Fn, codefile::OpCode I, Byte Dest, Byte Src1, UInt16 Imm) {
        Fn.Code << Byte(I);
        Fn.Code << Byte(Dest | (Src1 << 4));
        Fn.Code << Imm;
    }

    constexpr void LocalSet(Function& Fn, Byte Src, Byte Offset) {
        STR(Fn, Src, Byte(codefile::BaseFP), UInt32(Offset));
    }
    
    constexpr void LocalGet(Function& Fn, Byte Dest, Byte Idx) {
        LDR(Fn, Dest, Byte(codefile::BaseFP), UInt32(Idx));
    }

    constexpr void GlobalSet(Function& Fn, Byte Src, UInt16 Imm) {
        STR(Fn, Src, Byte(codefile::BaseCS), Imm);
    }

    constexpr void GlobalGet(Function& Fn, Byte Dest, UInt16 Imm) {
        LDR(Fn, Dest, codefile::BaseType::BaseCS, Imm);
    }
    
    constexpr void Mov(Function& Fn, Byte Dest, Byte Src) {
        Fn.Code << Byte(codefile::OpCode::Mov);
        Fn.Code << Byte(Dest | (Src << 4));
    }

    constexpr void Mov4(Function& Fn, Byte Dest, Byte Imm) {
        Fn.Code << Byte(codefile::OpCode::Mov4);
        Fn.Code << Byte(Dest | (Imm<<4));
    }

    constexpr void Mov8(Function& Fn, Byte Dest, Byte Imm) {
        Fn.Code << Byte(codefile::OpCode::Mov8);
        Fn.Code << Byte(Dest | 0b00000000);
        Fn.Code << Imm;
    }

    constexpr void Mov16(Function& Fn, Byte Dest, UInt16 Imm) {
        Fn.Code << Byte(codefile::OpCode::Mov16);
        Fn.Code << Byte(Dest | 0b00000000);
        Fn.Code << Imm;
    }

    constexpr void Mov32(Function& Fn, Byte Dest, UInt32 Imm) {
        Fn.Code << Byte(codefile::OpCode::Mov32);
        Fn.Code << Byte(Dest | 0b00000000);
        Fn.Code << Imm;
    }

    constexpr void Mov64(Function& Fn, Byte Dest, UInt64 Imm) {
        Fn.Code << Byte(codefile::OpCode::Mov64);
        Fn.Code << Dest;
        Fn.Code << Imm;
    }

    constexpr void Ldsr(Function& Fn, Byte Dest, UInt32 Imm) {
        Fn.Code << Byte(codefile::OpCode::Ldstr);
        Fn.Code << Dest;
        Fn.Code << Imm;
    }

    constexpr void Cmp(Function& Fn, Byte Src1, Byte Src2) {
        Fn.Code << Byte(codefile::OpCode::Cmp);
        Fn.Code << Byte(Src1 | (Src2 << 4));
    }

    constexpr void FCmp(Function& Fn, Byte Src1, Byte Src2) {
        Fn.Code << Byte(codefile::OpCode::FCmp);
        Fn.Code << Byte(Src1 | (Src2 << 4));
    }

    constexpr void TestZ(Function& Fn, Byte R) {
        Fn.Code << Byte(codefile::OpCode::TestZ);
        Fn.Code << R;
    }

    constexpr void Jmp(Function& Fn, codefile::OpCode OpCode, UInt16 Imm) {
        Fn.Code << Byte(OpCode);
        Fn.Code << Imm;
    }

    constexpr void Call(Function& Fn, UInt32 Imm) {
        Fn.Code << Byte(codefile::OpCode::Call);
        Fn.Code << Imm;
    }

    constexpr void Ret(Function& Fn) {
        Fn.Code << Byte(codefile::OpCode::Ret);
    }

    constexpr void RetC(Function& Fn, UInt32 Code) {
        Fn.Code << Byte(codefile::OpCode::RetC);
        Fn.Code << Code;
    }

    constexpr void Inc(Function& Fn, Byte Reg) {
        Fn.Code << Byte(codefile::OpCode::Inc);
        Fn.Code << Reg;
    }

    constexpr void IInc(Function& Fn, Byte Reg) {
        Fn.Code << Byte(codefile::OpCode::IInc);
        Fn.Code << Reg;
    }

    constexpr void FInc(Function& Fn, Byte Reg) {
        Fn.Code << Byte(codefile::OpCode::FInc);
        Fn.Code << Reg;
    }

    constexpr void Dec(Function& Fn, Byte Reg) {
        Fn.Code << Byte(codefile::OpCode::Dec);
        Fn.Code << Reg;
    }

    constexpr void IDec(Function& Fn, Byte Reg) {
        Fn.Code << Byte(codefile::OpCode::IDec);
        Fn.Code << Reg;
    }

    constexpr void FDec(Function& Fn, Byte Reg) {
        Fn.Code << Byte(codefile::OpCode::FDec);
        Fn.Code << Reg;
    }

    constexpr void Add(Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        SML(Fn, codefile::OpCode::Add, Dest, Src1, Src2);
    }

    constexpr void Sub(Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        SML(Fn, codefile::OpCode::Sub, Dest, Src1, Src2);
    }

    constexpr void Mul(Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        SML(Fn, codefile::OpCode::Mul, Dest, Src1, Src2);
    }

    constexpr void Div(Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        SML(Fn, codefile::OpCode::Div, Dest, Src1, Src2);
    }

    constexpr void IAdd(Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        SML(Fn, codefile::OpCode::IAdd, Dest, Src1, Src2);
    }

    constexpr void ISub(Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        SML(Fn, codefile::OpCode::ISub, Dest, Src1, Src2);
    }

    constexpr void IMul(Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        SML(Fn, codefile::OpCode::IMul, Dest, Src1, Src2);
    }

    constexpr void IDiv(Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        SML(Fn, codefile::OpCode::IDiv, Dest, Src1, Src2);
    }

    constexpr void FAdd(Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        SML(Fn, codefile::OpCode::FAdd, Dest, Src1, Src2);
    }

    constexpr void FSub(Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        SML(Fn, codefile::OpCode::FSub, Dest, Src1, Src2);
    }

    constexpr void FMul(Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        SML(Fn, codefile::OpCode::FMul, Dest, Src1, Src2);
    }

    constexpr void FDiv(Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        SML(Fn, codefile::OpCode::FDiv, Dest, Src1, Src2);
    }

    constexpr void Add8(Function& Fn, Byte Dest, Byte Src1, Byte Imm) {
        SML8(Fn, codefile::OpCode::Add8, Dest, Src1, Imm);
    }

    constexpr void Sub8(Function& Fn, Byte Dest, Byte Src1, Byte Imm) {
        SML8(Fn, codefile::OpCode::Sub8, Dest, Src1, Imm);
    }

    constexpr void Mul8(Function& Fn, Byte Dest, Byte Src1, Byte Imm) {
        SML8(Fn, codefile::OpCode::Mul8, Dest, Src1, Imm);
    }

    constexpr void Div8(Function& Fn, Byte Dest, Byte Src1, Byte Imm) {
        SML8(Fn, codefile::OpCode::Div8, Dest, Src1, Imm);
    }

    constexpr void IAdd8(Function& Fn, Byte Dest, Byte Src1, Byte Imm) {
        SML8(Fn, codefile::OpCode::IAdd8, Dest, Src1, Imm);
    }

    constexpr void ISub8(Function& Fn, Byte Dest, Byte Src1, Byte Imm) {
        SML8(Fn, codefile::OpCode::ISub8, Dest, Src1, Imm);
    }

    constexpr void IMul8(Function& Fn, Byte Dest, Byte Src1, Byte Imm) {
        SML8(Fn, codefile::OpCode::IMul8, Dest, Src1, Imm);
    }

    constexpr void IDiv8(Function& Fn, Byte Dest, Byte Src1, Byte Imm) {
        SML8(Fn, codefile::OpCode::IDiv8, Dest, Src1, Imm);
    }

    constexpr void Add16(Function& Fn, Byte Dest, Byte Src1, UInt16 Imm) {
        SML16(Fn, codefile::OpCode::Add16, Dest, Src1, Imm);
    }

    constexpr void Sub16(Function& Fn, Byte Dest, Byte Src1, UInt16 Imm) {
        SML16(Fn, codefile::OpCode::Sub16, Dest, Src1, Imm);
    }

    constexpr void Mul16(Function& Fn, Byte Dest, Byte Src1, UInt16 Imm) {
        SML16(Fn, codefile::OpCode::Mul16, Dest, Src1, Imm);
    }

    constexpr void Div16(Function& Fn, Byte Dest, Byte Src1, UInt16 Imm) {
        SML16(Fn, codefile::OpCode::Div16, Dest, Src1, Imm);
    }

    constexpr void IAdd16(Function& Fn, Byte Dest, Byte Src1, UInt16 Imm) {
        SML16(Fn, codefile::OpCode::IAdd16, Dest, Src1, Imm);
    }

    constexpr void ISub16(Function& Fn, Byte Dest, Byte Src1, UInt16 Imm) {
        SML16(Fn, codefile::OpCode::ISub16, Dest, Src1, Imm);
    }

    constexpr void IMul16(Function& Fn, Byte Dest, Byte Src1, UInt16 Imm) {
        SML16(Fn, codefile::OpCode::IMul16, Dest, Src1, Imm);
    }

    constexpr void IDiv16(Function& Fn, Byte Dest, Byte Src1, UInt16 Imm) {
        SML16(Fn, codefile::OpCode::IDiv16, Dest, Src1, Imm);
    }

    constexpr void Or(Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        SML(Fn, codefile::OpCode::Or, Dest, Src1, Src2);
    }

    constexpr void And(Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        SML(Fn, codefile::OpCode::And, Dest, Src1, Src2);
    }

    constexpr void XOr(Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        SML(Fn, codefile::OpCode::XOr, Dest, Src1, Src2);
    }

    constexpr void Shl(Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        SML(Fn, codefile::OpCode::Shl, Dest, Src1, Src2);
    }

    constexpr void Shr(Function& Fn, Byte Dest, Byte Src1, Byte Src2) {
        SML(Fn, codefile::OpCode::Shr, Dest, Src1, Src2);
    }

    constexpr void Or8(Function& Fn, Byte Dest, Byte Src1, Byte Imm) {
        SML8(Fn, codefile::OpCode::Or8, Dest, Src1, Imm);
    }

    constexpr void And8(Function& Fn, Byte Dest, Byte Src1, Byte Imm) {
        SML8(Fn, codefile::OpCode::And8, Dest, Src1, Imm);
    }

    constexpr void XOr8(Function& Fn, Byte Dest, Byte Src1, Byte Imm) {
        SML8(Fn, codefile::OpCode::XOr8, Dest, Src1, Imm);
    }

    constexpr void Shl8(Function& Fn, Byte Dest, Byte Src1, Byte Imm) {
        SML8(Fn, codefile::OpCode::Shl8, Dest, Src1, Imm);
    }

    constexpr void Shr8(Function& Fn, Byte Dest, Byte Src1, Byte Imm) {
        SML8(Fn, codefile::OpCode::Shr8, Dest, Src1, Imm);
    }

    constexpr void Or16(Function& Fn, Byte Dest, Byte Src1, UInt16 Imm) {
        SML16(Fn, codefile::OpCode::Or16, Dest, Src1, Imm);
    }

    constexpr void And16(Function& Fn, Byte Dest, Byte Src1, UInt16 Imm) {
        SML16(Fn, codefile::OpCode::And16, Dest, Src1, Imm);
    }

    constexpr void XOr16(Function& Fn, Byte Dest, Byte Src1, UInt16 Imm) {
        SML16(Fn, codefile::OpCode::XOr16, Dest, Src1, Imm);
    }

    constexpr void Not(Function& Fn, Byte SrcDest) {
        Fn.Code << Byte(codefile::OpCode::Not);
        Fn.Code << SrcDest;
    }

    constexpr void Neg(Function& Fn, Byte SrcDest) {
        Fn.Code << Byte(codefile::OpCode::Neg);
        Fn.Code << SrcDest;
    }

    constexpr void Push8(Function& Fn, UInt8 Imm) {
        Fn.Code << Byte(codefile::OpCode::Push8);
        Fn.Code << Imm;
    }

    constexpr void Push16(Function& Fn, UInt16 Imm) {
        Fn.Code << Byte(codefile::OpCode::Push16);
        Fn.Code << Imm;
    }

    constexpr void Push32(Function& Fn, UInt32 Imm) {
        Fn.Code << Byte(codefile::OpCode::Push32);
        Fn.Code << Imm;
    }

    constexpr void Push64(Function& Fn, UInt64 Imm) {
        Fn.Code << Byte(codefile::OpCode::Push64);
        Fn.Code << Imm;
    }

    constexpr void PopTop(Function& Fn) {
        Fn.Code << Byte(codefile::OpCode::Popd);
    }
    
    constexpr void Push(Function& Fn, Byte Src) {
        Fn.Code << Byte(codefile::OpCode::Push);
        Fn.Code << Src;
    }

    constexpr void Pop(Function& Fn, Byte Dest) {
        Fn.Code << Byte(codefile::OpCode::Pop);
        Fn.Code << Dest;
    }

    constexpr void ArrayNew(Function& Fn, Byte Dest, codefile::ArrayElement ET) {
        Fn.Code << Byte(codefile::OpCode::ArrayNew);
        Fn.Code << Byte(Dest | (ET<<4));
    }

    constexpr void ArrayLen(Function& Fn, Byte ArrayInReg, Byte Dest) {
        Fn.Code << Byte(codefile::OpCode::ArrayL);
        Fn.Code << Byte(ArrayInReg | (Dest << 4));
    }

    constexpr void ArrayLoad(Function& Fn, Byte ArrayInReg, Byte IndexInReg, Byte Dest) {
        Fn.Code << Byte(codefile::OpCode::ArrayLoad);
        Fn.Code << Byte(ArrayInReg | (IndexInReg << 4));
        Fn.Code << Dest;
    }

    constexpr void ArrayStr(Function& Fn, Byte ArrayInReg, Byte IndexInReg, Byte Src) {
        Fn.Code << Byte(codefile::OpCode::ArrayStore);
        Fn.Code << Byte(ArrayInReg | (IndexInReg << 4));
        Fn.Code << Src;
    }

    constexpr void ArrayDestroy(Function& Fn, Byte Src) {
        Fn.Code << Byte(codefile::OpCode::ArrayDestroy);
        Fn.Code << Src;
    }

};

}
