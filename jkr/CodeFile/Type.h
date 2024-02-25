#pragma once
#include "stdjk/CoreHeader.h"

namespace codefile {

enum TypePrimitive : Byte {
    PrimitiveByte,

    PrimitiveInt,
    PrimitiveIntC8,
    PrimitiveIntC16,
    PrimitiveIntC32,

    PrimitiveUInt,
    PrimitiveUIntC8,
    PrimitiveUIntC16,
    PrimitiveUIntC32,
    
    PrimitiveFloat,
    PrimitiveFloatC32,
    
    PrimitiveStringRef,
    
    PrimitiveObject,
    
    PrimitiveAny,
};

enum TypeAttributes : Byte {
    TypeNone = 0,
    TypeArray = 0x01,
    TypePointer = 0x02,
};

}
