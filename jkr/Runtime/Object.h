#pragma once
#include "stdjk/List.h"
#include "jkr/Runtime/Value.h"

namespace runtime {

struct Field {
    Byte Type;

    Value Content;
};

struct [[nodiscard]] Object {

    using FieldList = List<Field, false>;

    static constexpr Object New(Byte FieldCount) {
        return Object{
            .Fields = FieldList::New(FieldCount)
        };
    }

    constexpr void Destroy(this Object& Self) {
        Self.Fields.Destroy();
    }

    FieldList Fields;
};

}
