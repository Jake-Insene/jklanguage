#pragma once
#include "jkc/CodeGen/SymbolTable.h"
#include "jkc/CodeGen/Values.h"
#include "jkc/CodeGen/CodeBuffer.h"

namespace CodeGen {

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
    Str Name = u8"";

    AST::TypeDecl Type = {};
    CodeBuffer Code = {};

    std::vector<AddressToResolve> ResolveReturns;
    SymbolTable<Local> Locals = {};
    Byte CountOfArguments = 0;
    Byte RegisterArguments = 0;
    Byte StackArguments = 0;
    Byte CountOfStackLocals = 0;
    UInt32 Address = 0;

    UInt32 LibraryAddress = 0;
    UInt32 EntryAddress = 0;

    bool IsDefined = false;
    bool IsExtern = false;
    bool HasMultiReturn = false;
    CallConv CC = CallConv::Stack;

    [[nodiscard]] constexpr bool IsRegisterBased() const {
        return CC == CallConv::Register || CC == CallConv::RegS;
    }
};

}
