#pragma once
#include "jkr/CoreTypes.h"

// CodeFile use 16 registers,
// Threre are register for simple instructions like
// r0...r31 and for vector instructions like v0...v31

namespace codefile {

constexpr auto MaxLocals = 0xFF;
constexpr auto MaxFields = 0xFF;
constexpr auto MaxFunctions = 0xFFFF'FFFF;
constexpr auto MaxObjects = 0xFFFF'FFFF;
constexpr auto MaxGlobals = 0xFFFF;
constexpr auto MaxStrings = 0xFFFF'FFFF;

enum BaseType {
    BaseSP = 0,
    BaseFP = 1,
    BaseCS = 2,
};

enum CallAddressType {
    CATReg = 0,
    CATStackOffset = 1,
    CATFramePointerOffset = 2,
    CATGlobalSAddress = 3,
};

enum AddressingMode {
    DirectMode = 0,
    IndirectMode = 1,
};

#define INST_ARG1(_I) Byte(_I & 0xF)
#define INST_ARG2(_I) Byte((_I >> 4) &0xF)

#define MOV_SPECIAL(_I) Byte((_I >> 4) & 0xF)

#define MATH_DEST(_I) INST_ARG1(_I)
#define MATH_SRC1(_I) INST_ARG2(_I)
#define MATH_SRC2(_I) Byte((_I >> 8) & 0xF)
// Move(Register)
// Dest register [8-12]/4 bits
// Src register [12-15]/4 bits

// Mov(Immediate)
// Dest register [8-11]/4 bits
// Special [12-13]/2 bits
// Imm Size [14-15]/2 bits
// Imm [15-...]/... bits

// Load String(Immediate)
// Dest register [8-11]/4 bits
// Address Special [12-15]/4 bits
// Address [16-39]/32 bis

// Load(Immediate)
// Dest register [8-11]/4 bits
// Base type [12-13]/4 bits
// Address [14-31]/16 bits

// Use LD_BT for Base Type
// Store(Immediate)
// Src register [8-11]/4 bits
// Base Type [12-15]/4 bits
// Imm [16-31]/16 bits

// Cmp Layout
// First operand register [8-11]/4 bits
// Second operand register [12-15]/4 bits

// Jmp(Immediate)
// Address [8-23]/16 bits

// Call(Immediate)
// Address [8-47]/32 bits

// Call Address(Immediate)
// Call Address type [8-9]/2 bits
// Address [10-31]/22 bits

// Inc/Dec
// Src/Dest register [8-11]/4 bits

// XADD/XSUB/XMUL/XDIV(Register)
// Dest register [8-11]/4 bits
// First operand register [12-15]/4 bits
// Second operand register [16-19]/4 bits

// XADD/XSUB/XMUL/XDIV(Immediate8)
// Dest register [8-11]/4 bits
// First operand register [12-15]/4 bits
// Second immediate operand [16-31]/8 bits

// XADD/XSUB/XMUL/XDIV(Immediate16)
// Dest register [8-11]/4 bits
// First operand register [12-15]/4 bits
// Second immediate operand [16-31]/16 bits

// Not/Neg
// Src/Dest register [8-11]/4 bits

// Ret Normal
// ...

// Ret(Immediate)
// Code [8-...]/... bits

// Push(Immediate)
// Immediate [8-...]/... bits

// Push/Pop(Register)
// Src/Dest [8-11]/4 bits

// Array New Layout
// Dest/Length register [8-11]/4 bits
// Array Element [12-15]/4 bits

// Array Length Layout
//Src array register [8-11]/4 bits
//Dest register [12-15]/4 bits

// Array Get/Str Layout
// Src array register [8-11]/4 bits
// Index register [12-15]/4 bits
// Dest/Src register [16-19]/4 bits

// Array Destroy Layout
// Src array register [8-11]/4 bits

enum class OpCode {
    Brk = 0,

    // Mov
    Mov,
    Mov4,
    Mov8,
    Mov16,
    Mov32,
    Mov64,
    // Load/Store
    Ldstr,
    Ldr,
    Str,

    // Control flow
    Cmp,
    FCmp,
    TestZ,
    Jmp,
    Je,
    Jne,
    Jl,
    Jle,
    Jg,
    Jge,

    Call,
    Calla,
    Ret,
    RetC,

    // Basic Aritmethic
    Inc,
    IInc,
    FInc,
    Dec,
    IDec,
    FDec,

    Add,
    Sub,
    Mul,
    Div,
    IAdd,
    ISub,
    IMul,
    IDiv,
    FAdd,
    FSub,
    FMul,
    FDiv,

    Add8,
    Sub8,
    Mul8,
    Div8,
    IAdd8,
    ISub8,
    IMul8,
    IDiv8,

    Add16,
    Sub16,
    Mul16,
    Div16,
    IAdd16,
    ISub16,
    IMul16,
    IDiv16,

    // Binary Aritmethic
    Or,
    And,
    XOr,
    Shl,
    Shr,

    Not,
    Neg,

    Or8,
    And8,
    XOr8,
    Shl8,
    Shr8,

    Or16,
    And16,
    XOr16,

    Push8,
    Push16,
    Push32,
    Push64,
    Popd,

    Push,
    Pop,

    // Array
    ArrayNew,
    ArrayL,
    ArrayLoad,
    ArrayStore,
    ArrayDestroy,

    // Object
    ObjectNew,
    ObjectDestroy,
};

}
