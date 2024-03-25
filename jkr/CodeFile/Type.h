#pragma once
#include "jkr/CoreTypes.h"

namespace codefile {

enum PrimitiveType : Byte {
    PrimitiveByte,
    PrimitiveInt,
    PrimitiveUInt,
    PrimitiveFloat,
    PrimitiveRef,
    PrimitiveAny, // Use for store references and values
                  // for example: Int, Array, Object, Float or a Byte
    // Another Primitive Type follows this one generally
    PrimitiveArray, /*Primitive Type[8 bits]*/

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
