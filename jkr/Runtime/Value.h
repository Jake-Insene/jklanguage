#pragma once
#include "stdjk/CoreHeader.h"

namespace runtime {

struct Object;
struct Array;

union [[nodiscard]] Value {
    Int Signed;
    UInt Unsigned;
    Float Real;
    Address Ptr;
    Object* Obj;
    Array* ArrayRef;
};

}
