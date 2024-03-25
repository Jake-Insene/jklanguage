#pragma once
#include "jkr/CodeFile/Data.h"
#include "jkr/Runtime/Value.h"

namespace runtime {

struct Object;
struct Array;

struct DataElement : codefile::DataHeader {
    using Float4 = Float[4];
    using Int4 = Int[4];
    using UInt4 = UInt[4];

    constexpr DataElement() : Value() {}
    constexpr ~DataElement() {}

    union {
        union {
            Float Real;
            Int Signed;
            UInt Unsigned;
            Address Ptr;
            Object* Obj;
            Array* ArrayRef;
        };
        union {
            Float4 FV;
            Int4 IV;
            UInt4 UV;
        };
    } Value;
};

}
