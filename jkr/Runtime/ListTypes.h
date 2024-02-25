#pragma once
#include <stdjk/List.h>
#include "jkr/Runtime/Value.h"

namespace runtime {

enum ListType {
    ListOfBytes,
    ListOfValues,
};

struct BasicList {
    Byte ListType;
};

struct ValueList : BasicList {
    using ElementList = List<Value, false>;
    ElementList Elements;

    static constexpr ValueList New(USize InitialSize) {
        return ValueList{
            .Elements = ElementList::New(InitialSize),
        };
    }

    constexpr void Destroy(this ValueList& Self) {
        Self.Elements.Destroy();
    }
};

struct ByteList : BasicList {
    using ElementList = List<Byte, false>;
    ElementList Elements;

    static constexpr ByteList New(USize InitialSize) {
        return ByteList{
            .Elements = ElementList::New(InitialSize),
        };
    }

    constexpr void Destroy(this ByteList& Self) {
        Self.Elements.Destroy();
    }
};

}
