#pragma once
#include "jkr/Runtime/Value.h"
#include "jkr/CodeFile/Type.h"
#include "jkr/CodeFile/Array.h"

namespace runtime {

struct Array {
    codefile::ArrayElement ElementType;
    UInt16 ElementSize = 0;
    USize Size = 0;
    union {
        Byte* Bytes = nullptr;
        Int* Ints;
        UInt* UInts;
        Float* Floats;
    };

    Array(USize Size, codefile::ArrayElement ElementType);
    ~Array();

    Array(Array&&) = default;
    Array& operator=(Array&&) = default;

    constexpr Byte& GetByte(USize Index) {
        return Bytes[Index];
    }

    constexpr Int& GetInt(USize Index) {
        return Ints[Index];
    }

    constexpr UInt& GetUInt(USize Index) {
        return UInts[Index];
    }

    constexpr Float& GetFloat(USize Index) {
        return Floats[Index];
    }
};

}
