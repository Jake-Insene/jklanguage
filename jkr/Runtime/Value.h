#pragma once
#include "jkr/CoreHeader.h"

namespace runtime {

struct Object;

struct [[nodiscard]] Value {
    union {
        Int Signed;
        UInt Unsigned;
        Float Real;
        Address Ptr;
        Object* Obj;
    };

    static constexpr Value New() { return {.Unsigned = 0}; }

    constexpr void Destroy(this Value& /*Self*/) {}

};

}
