#pragma once
#include "stdjk/CoreHeader.h"

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
};

}
