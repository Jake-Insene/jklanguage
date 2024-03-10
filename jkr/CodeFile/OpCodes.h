#pragma once
#include "stdjk/CoreHeader.h"

// codefile Opcodes
// A instruction can't have more than 10 bytes
// A total of 16 registers

namespace codefile {

constexpr auto MaxLocals = 0xFF;
constexpr auto MaxFields = 0xFF;
constexpr auto MaxFunctions = 0xFFFF'FFFF;
constexpr auto MaxObjects = 0x00FF'FFFF;
constexpr auto MaxGlobals = 0x00FF'FFFF;
constexpr auto MaxStrings = 0x00FF'FFFF;

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
    R15, // Address and memory register, used for memory instructions.
};

enum class IncDecType {
    Signed = 0,
    Unsigned = 1,
    Real = 2,
    SignedNO = 3,
    UnsignedNO = 4,
    RealNO = 5,
};

enum ImmOperation {
    ImmOperationLeft = 0,
    ImmOperationExtendRight = 1,
    ImmOperationExtendSign = 2,
    ImmOperationPutInRightMidle = 3,
};

enum class OpCode {
    Brk = 0,

    // Dest register [4 bits]
    // Src register [4 bits]
    Mov,
    // Dest register [4 bits]
    // Constant [4 bits]
    Mov4,
    // Dest register [4 bits]
    // Imm operation [4 bits]
    // Constant [... bits]
    Mov8,
    Mov16,
    Mov32,
    // Don't use imm operation
    Mov64,

    // Move the result of a previus call to the
    // specified register
    // Dest register [4 bits]
    // Zero filled [4 bits]
    MovRes,

    // Src/Dest register [4 bits]
    // Local index [4 bits]
    LocalSet4,
    LocalGet4,
    // Src/Dest register [4 bits]
    // Zero filled [4 bits]
    // Local address [8 bits]
    LocalSet,
    LocalGet,
    // Src/Dest register [4 bits]
    // Data address [28 bits]
    GlobalSet,
    GlobalGet,

    // --- Math ---
    // Src register [4 bits]
    // Increment/Decrement type [4 bits]
    Inc,
    Dec,

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
    Or,
    And,
    XOr,
    Shl,
    Shr,

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
    Or8,
    And8,
    XOr8,
    Shl8,
    Shr8,

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
    Or16,
    And16,
    XOr16,

    // --- Conditional Execution ---
    // First operand register [4 bits]
    // Second operand register [4 bits]
    Cmp,
    ICmp,
    FCmp,

    // First operand register [4 bits]
    // Zero filled [4 bits]
    // Check if the first operand is zero
    // Same as:
    //      mov.c4 r0, 1
    //      mov.c4 r1, 0
    //      cmp r0, r1
    TestZ,

    // IP Relative address [16 bits]
    Jmp,
    Jez,
    Jnz,
    Je,
    Jne,
    Jl,
    Jle,
    Jg,
    Jge,
    Jo,
    Jno,
    Jc,
    Jnc,
    Js,
    Jns,
    // Same as above but with 8 bits address
    Jmp8,
    Jez8,
    Jnz8,
    Je8,
    Jne8,
    Jl8,
    Jle8,
    Jg8,
    Jge8,
    Jo8,
    Jno8,
    Jc8,
    Jnc8,
    Js8,
    Jns8,

    // Dest register [4 bits]
    // Src register [4 bits]
    CMovez,
    CMovnz,
    CMove,
    CMovne,
    CMovl,
    CMovle,
    CMovg,
    CMovge,
    CMovo,
    CMovno,
    CMovc,
    CMovnc,
    CMovs,
    CMovns,

    // --- Function ---
    // Perfome a call
    // Function address [8 bits]
    Call8,
    // Function address [32 bits]
    Call,

    // Normaly, a function returns a value in the register 0
    Ret,

    // --- Objects ---
    // Create a object of the given type
    // Dest object register [4 bits]
    // Object type address(type object section) [28 bits]
    ObjectNew,

    // SrcDest field register [4 bits]
    // Field index [4 bits]
    FieldSet4,
    FieldGet4,
    // Src/Dest field register [4 bits]
    // Zero filled [4 bits]
    // Field index [8 bits]
    FieldSet,
    FieldGet,

    // Src object register [4 bits]
    // Src operand type [4 bits]
    ObjectDestroy,

    // --- Array ---
    // Dest array register [4 bits]
    // Size register [4 bits]
    // Element Type [8 bits]
    ArrayNew,

    // Dest array register [4 bits]
    // Size register [4 bits]
    // Object type address(type section) [32 bits]
    ArrayObjectNew,

    // Dest register [4 bits]
    // Src array register [4 bits]
    ArrayLen,

    // Src array register [4 bits]
    // Index array register [4 bits]
    // Src/Dest register [4 bits]
    // Zero filled [4 bits]
    ArraySet,
    ArrayGet,

    // Src array register [4 bits]
    // Constant address [4 bits]
    FillArray4,
    // Src array register [4 bits]
    // Constant address [28 bits]
    FillArray,

    // Src array register [4 bits]
    // Zero filled [4 bits]
    ArrayDestroy,

    // --- String ---
    // Dest register [4 bits]
    // String Table address [4 bits]
    StringGet4,
    // Dest string register [4 bits]
    // String Table address [28 bits]
    StringGet,

    // --- Constant ---
    // Dest register [4 bits]
    // Constant address [4 bits]
    LoadConst4,

    // Dest register [4 bits]
    // Constant address [28 bits]
    LoadConst,

    // --- Stack ---
    // Constant [... bits]
    Push8,
    Push16,
    Push32,
    Push64,

    // Discarp the top of the stack
    PopTop,

    PushR0 = 192,
    PushR1,
    PushR2,
    PushR3,
    PushR4,
    PushR5,
    PushR6,
    PushR7,
    PushR8,
    PushR9,
    PushR10,
    PushR11,
    PushR12,
    PushR13,
    PushR14,
    PushR15,

    PopR0 = 208,
    PopR1,
    PopR2,
    PopR3,
    PopR4,
    PopR5,
    PopR6,
    PopR7,
    PopR8,
    PopR9,
    PopR10,
    PopR11,
    PopR12,
    PopR13,
    PopR14,
    PopR15,

    // Local address [8 bits]
    PushLocal,
    // Local address [8 bits]
    PopLocal,

    // Simd prefix to access to the simd instruction set
    SimdPrefix = 0xF0,
};

}
