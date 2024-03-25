#pragma once
#include "jkr/CoreTypes.h"

namespace codefile {

enum FunctionFlags : UInt16 {
    FunctionNone = 0x0,
    FunctionImport = 0x01,
    FunctionExport = 0x02,
    FunctionNative = 0x04,
    FunctionDebugInfo = 0x08,
};

constexpr Byte MaxArguments = 32;

struct FunctionHeader {
    UInt32 Flags;

    // This fields makes a string index in the string table: Entry
    UInt16 StackArguments;
    UInt16 LocalReserve;

    // If has the Native flag
    // SizeOfCode is a index in string table: Library
    UInt32 SizeOfCode;
};

struct FunctionDebugInfo {
    UInt32 Name; // Index in string table
};

}
