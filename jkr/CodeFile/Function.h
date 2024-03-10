#pragma once
#include "stdjk/CoreHeader.h"

namespace codefile {

enum FunctionFlags : UInt16 {
    FunctionNone = 0x0,
    FunctionImport,
    FunctionExport,
    FunctionNative,
    FunctionDebugInfo,
};

constexpr Byte MaxArguments = 32;

struct FunctionHeader {
    UInt32 Flags;
    // This field makes a string index in the string table: Entry
    UInt32 LocalReserve;

    // If type is Native
    // SizeOfCode is a index in string table: Library
    UInt32 SizeOfCode;
};

struct FunctionDebugInfo {
    UInt16 Name; // Index in string table
};

}
