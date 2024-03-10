#pragma once
#include <stdjk/List.h>
#include "jkr/Runtime/Value.h"
#include "jkr/CodeFile/Type.h"

namespace runtime {

struct Array {
    codefile::PrimitiveType ItemType;
    UInt32 ElementSize;
    USize Size;
    void* Items;

    static constexpr Array New(USize Size, UInt32 ElementSize) {
        Array array = Array{
            .Items = Cast<Value*>(mem::Allocate(ElementSize * Size)),
        };

        array.Size = Size;
        array.ElementSize = ElementSize;
        return array;
    }

    void Destroy(this Array& Self);
};

}
