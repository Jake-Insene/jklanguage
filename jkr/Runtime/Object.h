#pragma once
#include "jkr/Runtime/Value.h"

namespace runtime {

struct Field {
    Byte Type;

    Value Content;
};

struct Object {
    constexpr Object(Byte FieldCount) :
        FieldCount(FieldCount) {
        Fields = new Field[FieldCount]{};
    }
    constexpr ~Object() {}

    Byte FieldCount;
    Field* Fields = nullptr;
};

}
