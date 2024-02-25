#pragma once
#include "jkr/CodeFile/OperandTypes.h"

namespace codefile {

enum FunctionAttributes : UInt16 {
    FunctionNone = 0x0,
    FunctionImport = 0x01,
    FunctionExport = 0x02,
    FunctionNative = 0x04,
};

constexpr auto MaxArguments = 32;

struct FunctionHeader {
    UInt16 Attributes;
    // Arguments are part of LocalCount but are used for diferent things
    // If you use the reg callconv you
    union {
        struct {
            Byte Arguments;
            Byte LocalCount;
        } NonNative;
        UInt16 EIndex; // Index in string table: Entry
    };

    union {
        UInt16 SizeOfCode;
        UInt16 LIndex; // Index in string table: Library
    };
};

}
