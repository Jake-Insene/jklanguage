#pragma once
#include "jkr/CoreHeader.h"

namespace codefile {

enum class TypePrimitive : Byte {
    Byte,
    Int,
    UInt,
    Float,
    StringRef,

    Object,
    Any,
};

enum TypeAttributes : Byte {
    TypeNone = 0,
    TypeArray = 0x01,
    TypePointer = 0x02,
};

struct Type {
    TypePrimitive Primitive : 4;
    TypeAttributes Attributes : 4;
};

}
