#pragma once
#include "jkr/CoreHeader.h"

namespace codefile {

enum FunctionAttributes {
    FunctionNone = 0x0,
    FunctionAOT = 0x01,
};

struct FunctionHeader {
    Byte Attributes;
    Byte Arguments;
    unsigned short LocalCount;
    unsigned int SizeOfCode;
};

}
