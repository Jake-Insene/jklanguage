#pragma once
#include "jkr/CodeFile/OperandTypes.h"
#include <vector>

// codefile Opcodes
// A instruction can't have more than 10 bytes
// A total of 16 registers

#define LOCAL_PREFIX "b"
#define GLOBAL_PREFIX "d"
#define FUNCTION_PREFIX "d"
#define ADDRESS_PREFIX "w"
#define FIELD_PREFIX "b"

namespace codefile {

constexpr auto MaxLocals = 0xFF;
constexpr auto MaxGlobals = 0xFFFF'FFFF;
constexpr auto MaxStrings = 0xFFFF'FFFF;
constexpr auto MaxFields = 0xFF;

enum Register : Byte {
    R0 = 0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    RMEM, // Address and memory register, used for memory instructions.
};

// Move Instruction Layout
struct MIL {
    Byte Dest : 4;
    Byte Src : 4;
};

// Common Instruction Layout
PACK(struct CIL {
    Byte Dest : 4;
    Byte Src1 : 4;
    Byte Src2 : 4;
    Byte Extra : 4;
});

// Local Instruction Layout
PACK(struct LIL {
    Byte SrcDest;
    LocalType Index;
});

PACK(struct LIL4 {
    Byte SrcDest : 4;
    Byte Index : 4;
});

// Global Instruction Layout
PACK(struct GIL {
    Byte SrcDest;
    GlobalType Index;
});

// String Instruction layout
PACK(struct SIL {
    Byte SrcDest;
    StringType Index;
});

// Field Instruction Layout
PACK(struct FIL {
    Byte SrcDest;
    FieldType Index;
});

enum class OpCode {
    Brk = 0,

    // MIL
    Mov,
    // Dest register [4 bits]
    // if (OpCode == Const4)
    //      Constant [4 bits]
    // else
    //      Constant [... bits]
    Const4,
    Const8,
    Const16,
    Const32,
    Const64,

    // Move the result of a previus call to the
    // specified register
    // Dest register [4 bits]-[4 bits]
    MovRes,

    // LIL4
    LocalSet4,
    LocalGet4,
    // LIL
    LocalSet,
    LocalGet,
    // GIL
    GlobalSet,
    GlobalGet,

    // --- Math ---
    // // Src/Dest register [4 bits]-[4 bits]
    Inc,
    IInc,
    FInc,
    Dec,
    IDec,
    FDec,

    // Dest register [4 bits]
    // First operand register [4 bits]
    // Second operand register [4 bits]
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

    // Dest register [4 bits]
    // First operand register [4 bits]
    // Constant [16 bits]
    Add8,
    IAdd8,
    Sub8,
    ISub8,
    Mul8,
    IMul8,
    Div8,
    IDiv8,

    // Dest register [4 bits]
    // Src register [4 bits]
    // Constant [16 bits]
    Add16,
    IAdd16,
    Sub16,
    ISub16,
    Mul16,
    IMul16,
    Div16,
    IDiv16,

    // --- Flow Control ---
    // MIL
    Cmp,
    ICmp,
    FCmp,

    // Address [AddressType]
    Jmp,
    Jez,
    Jnz,
    Je,
    Jne,
    Jl,
    Jle,
    Jg,
    Jge,
    // Same as above but with 8 bits values
    Jmp8,
    Jez8,
    Jnz8,
    Je8,
    Jne8,
    Jl8,
    Jle8,
    Jg8,
    Jge8,

    // Perfome a call
    // Function Index [FunctionType]
    Call,
    // Function Index [8 bits]
    Call8,

    // Used in void functions or in functions with register call conv
    RetVoid,
    // Src register [4 bits]-[4 bits]
    Ret,
    // Constant [8 bits]
    Ret8,
    // Constant [16 bits]
    Ret16,
    // Src local [LocalType]
    RetLocal,
    // Src global [GlobalType]
    RetGlobal,

    // --- Objects ---
    
    // Create a object of the given type
    // Dest object register [4 bits]-[4 bits]
    // Object Index [ObjectType]
    NewObject,

    // Src object register [4 bits]-[4 bits]
    DestroyObject,

    // FIL
    FieldSet,
    FieldGet,

    // --- Array ---

    // Dest array register [4 bits]-[4 bits]
    // Type [Type]
    NewArray,

    // Dest array register [4 bits]-[4 bits]
    // Object Index [ObjectType]
    NewArrayObject,

    // Dest register [4 bits]
    // Src array register [4 bits]
    ArrayLen,

    // Dest/Src register [4 bits]
    // Src array register [4 bits]
    ArraySet,
    ArrayGet,

    // Src array register [4 bits]
    DestroyArray,

    // --- String ---
    // SIL
    StringGet,

    MemoryPrefix = 0xFA,
    StackPrefix = 0xFB,
    Prefix3 = 0xFC,
    Prefix4 = 0xFD,
    Prefix5 = 0xFE,
    Prefix6 = 0xFF,
};

enum class OpCodeMemory {
    // Store the result on the rmem register
    // 32 bits buffer size [32 bits]
    Allocate,
    // Deallocate the buffer on the rmem register
    Deallocate,
};

enum class OpCodeStack {
    // Src register [4 bits]-[4 bits]
    Push,
    // Src local [LocalType]
    LocalPush,
    // Src global [GlobalType]
    GlobalPush,

    // Constant [... bits]
    Push8,
    Push16,
    Push32,
    Push64,
    
    PopTop,

    // Dest register [4 bits]-[4 bits]
    Pop,
};

}
