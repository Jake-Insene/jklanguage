#pragma once
#include "jkr/CoreHeader.h"

// codefile Opcodes
// A instruction can't have more than 10 bytes
// A total of 16 registers

#define LOCAL_PREFIX "w"
#define FUNCTION_PREFIX "d"
#define FIELD_PREFIX "w"

namespace codefile {

constexpr auto MaxLocals = 0x1FFF;
constexpr auto MaxGlobals = 0x1FFF'FFFF;
constexpr auto MaxStrings = 0x1FFF'FFFF;

using LocalType = Uint16;
using FunctionType = Uint32;
using GlobalType = Uint32;
using FieldType = Uint16;

enum Register : Byte {
    R0 = 0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    RMEM, // Address and memory register, used for memory instructions.
};

// Low instruction
struct LI {
    Byte Dest : 4;
    Byte Src : 4;
};

// Common Instruction Layout
struct CIL {
    Byte Dest;
    Byte Src1;
    Byte Src2;
};

// Local Instruction Layout
struct LIL {
    Byte SrcDest;
    Byte Idx1;
    Byte Idx2;

    constexpr Uint16 Index() const {
        return ((unsigned short)(Idx1) | (Idx2 << 8));
    }
};

// Global Instruction Layout
struct GIL {
    Byte SrcDest;
    Byte Idx1;
    Byte Idx2;
    Byte Idx3;
    Byte Idx4;
};

// String Instruction layout
struct SIL {
    Byte SrcDest;
    Byte Idx1;
    Byte Idx2;
    Byte Idx3;
    Byte Idx4;
};

enum class OpCode {
    Brk = 0,

    // LI
    Mov,
    // Dest register [3 bits]-[5 bits]
    // Constant [... bits]
    Mov8,
    Mov16,
    Mov32,
    Mov64,

    // Move the result of a previus call to the
    // specified register
    // Dest register [3 bits]-[5 bits]
    RMov,

    // LIL
    LocalSet,
    LocalGet,
    // GIL
    GlobalSet,
    GlobalGet,

    Add,
    IAdd,
    FAdd,
    Sub,
    ISub,
    FSub,
    Mul,
    IMul,
    FMul,
    Div,
    IDiv,
    FDiv,

    // LI
    Cmp,
    ICmp,
    FCmp,

    // Address [32 bits]
    Jmp,
    Jez,
    Jnz,
    Je,
    Jne,
    Jl,
    Jle,
    Jg,
    Jge,

    // Perfome a call
    // Function Index [32 bits]
    Call,

    Ret,
    // Src register [3 bits]-[5 bits]
    RRet,
    // LIL
    LRet,
    // GIL
    GRet,

    MemoryPrefix = 0xF0,
    StackPrefix = 0xF1,
};

enum class OpCodeMemory {
    // Store the result on the rmem register
    // 32 bits buffer size [32 bits]
    Allocate,
    // Deallocate the buffer on the rmem register
    Deallocate,
};

enum class OpCodeStack {
    // Src register [3 bits]
    RPush,
    // Src local [16 bits]
    LPush,
    // Src global [32 bits]
    GPush,

    Push8,
    Push16,
    Push32,
    Push64,
    
    PopTop,

    // Dest register [3 bits]
    RPop,
};

}
