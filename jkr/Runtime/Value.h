#pragma once
#include "jkr/CoreTypes.h"

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
