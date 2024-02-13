#pragma once
#include "jkr/CoreHeader.h"

namespace codefile {

enum FunctionAttributes {
    FunctionNone = 0x0,
    FunctionAOT = 0x01,
};

constexpr auto MaxArguments = 32;

struct FunctionHeader {
    Byte Attributes;
    // Arguments are part of LocalCount but are used for diferent things
    Byte Arguments;
    Byte LocalCount;
    unsigned int SizeOfCode;
};

}
