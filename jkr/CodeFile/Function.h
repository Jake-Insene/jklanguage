#pragma once
#include "jkr/CoreHeader.h"

namespace codefile {

enum FunctionAttributes {
    FunctionNone = 0x0,
    FunctionAOT = 0x01,
};

constexpr auto MaxArguments = 32;

struct FunctionHeader {
    Uint16 Attributes;
    // Arguments are part of LocalCount but are used for diferent things
    // If you use the reg callconv you
    Byte Arguments;
    Byte LocalCount;
    Uint16 SizeOfCode;
};

}
