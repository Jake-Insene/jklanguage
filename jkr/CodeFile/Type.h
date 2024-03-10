#pragma once
#include "stdjk/CoreHeader.h"

namespace codefile {

enum PrimitiveType : Byte {
    PrimitiveByte,
    PrimitiveInt,
    PrimitiveUInt,
    PrimitiveFloat,
    PrimitiveStringRef,
    PrimitiveObject,
    PrimitiveAny, // Use for store references and values
                  // for example: Int, Array, Object, Float or a Byte
    PrimitiveArray,

    PrimitiveVector2F,
    PrimitiveVector3F,
    PrimitiveVector4F,
    PrimitiveVector2I,
    PrimitiveVector3I,
    PrimitiveVector4I,
    PrimitiveVector2U,
    PrimitiveVector3U,
    PrimitiveVector4U,
};

}
