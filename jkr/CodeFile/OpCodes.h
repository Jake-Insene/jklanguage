#pragma once
#include "jkr/CodeFile/OperandTypes.h"
#include <vector>

// codefile Opcodes
// A instruction can't have more than 10 bytes
// A total of 16 registers

#define LOCAL_PREFIX "b"
#define GLOBAL_PREFIX "w"
#define FUNCTION_PREFIX "d"
#define STRING_PREFIX "w"
#define ADDRESS_PREFIX "w"
#define FIELD_PREFIX "b"

namespace codefile {

constexpr auto MaxLocals = 0xFF;
constexpr auto MaxGlobals = 0xFFFF;
constexpr auto MaxStrings = 0xFFFF;
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

    // Dest register [4 bits]
    // Src register [4 bits]
    Mov,
    // Dest register [4 bits]
    // if (OpCode == Const4)
    //      Constant [4 bits]
    // else
    //      Constant [... bits]
    Mov4,
    // Extra bits layout
    //                     4 Extend left
    //                     5 Extend right
    //                     6 Sign extent
    //                     7 [Zero]
    Mov8,
    Mov16,
    Mov32,
    // Don't use extra bits layout
    Mov64,

    // Move the result of a previus call to the
    // specified register
    // Dest register [4 bits]
    // Unused bits [4 bits]
    MovRes,

    // SrcDest register [4 bits]
    // Constant [4 bits]
    LocalSet4,
    LocalGet4,
    // SrcDest register [4 bits]
    // Unused bits [4 bits]
    // Local index [LocalType]
    LocalSet,
    LocalGet,
    // SrcDest register [4 bits]
    // Zero filled [4 bits]
    // Global index [GlobalType]
    GlobalSet,
    GlobalGet,

    // --- Math ---
    // // Src/Dest register [4 bits]
    //                       4-5 Incrementation type
    //                              Unsigned 00
    //                              Signed 01
    //                              Floating 10
    // Zero filled [4 bits]
    Inc,
    IInc,
    FInc,
    Dec,
    IDec,
    FDec,

    // Dest register [4 bits]
    // First operand register [4 bits]
    // Second operand register [4 bits]
    // Zero filled [4 bits]
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
    // Constant [8 bits]
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
    // First operand register [4 bits]
    // Second operand register [4 bits]
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
    // Function Index [8 bits]
    Call8,
    // Function Index [FunctionType]
    Call,

    // Used in void functions or in functions with register based call conv
    RetVoid,
    // Src register [4 bits]
    // Zero filled [4 bits]
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
    // Dest object register [4 bits]
    // Zero filled [4 bits]
    // Object Index [ObjectType]
    NewObject,

    // Src object register [4 bits]
    // Zero filled [4 bits]
    DestroyObject,

    // SrcDest field register [4 bits]
    // Field index [4 bits]
    FieldSet4,
    FieldGet4,
    // SrcDest field register [4 bits]
    // Zero filled [4 bits]
    // Field index [FieldType]
    FieldSet,
    FieldGet,

    // --- Array ---
    // Dest array register [4 bits]
    // Zero filled [4 bits]
    // Type [Type]
    NewArray,

    // Dest array register [4 bits]
    // Zero filled [4 bits]
    // Object Index [ObjectType]
    NewArrayObject,

    // Dest register [4 bits]
    // Src array register [4 bits]
    ArrayLen,

    // Dest/Src register [4 bits]
    // Index array register [4 bits]
    // Src array register [4 bits]
    ArraySet,
    ArrayGet,

    // Src array register [4 bits]
    // Zero filled [4 bits]
    DestroyArray,

    // --- String ---
    // Dest register [4 bits]
    // String index [4 bits]
    StringGet4,
    // Dest string register [4 bits]
    // Zero filled [4 bits]
    // String index [StringType]
    StringGet,

    // Src register [4 bits]
    //              4 Duplicate
    //              5 PopBefore
    //              6 Unused(Must be zero)
    //              7 Unused(Must be zero)
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

    // Discarp the top of the stack
    PopTop,

    // Dest register [4 bits]
    //                4 Double pop
    //                5 Unused(Must be zero)
    //                6 Unused(Must be zero)
    //                7 Unused(Must be zero)
    Pop,

    // Performe a simd instruction
    SimdPrefix = 0xFA,
};

}
