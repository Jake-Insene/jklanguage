#pragma once
#include "jkr/CodeFile/Data.h"
#include "jkr/Runtime/Value.h"

namespace runtime {

struct DataElement : codefile::DataHeader {
    using Float4 = Float[4];
    using Int4 = Int[4];
    using UInt4 = UInt[4];
    using Byte4 = Byte[4];

    static constexpr DataElement New() {
        return DataElement{};
    }

    constexpr void Destroy(this DataElement& /*Self*/) {}

    Value Contant;
};

}
