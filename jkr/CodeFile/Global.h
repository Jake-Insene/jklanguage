#pragma once
#include "jkr/CodeFile/Type.h"

namespace codefile {

struct GlobalHeader {
    Byte Primitive;
    Byte Attributes;
    // If Attributes has TypeArray then
    //  Array Size [32 bits]
    //  Primitive Data[ArraySize];
    // Else
    //      Data based on primitive...
};

}
