#pragma once
#include "jkr/CoreHeader.h"

namespace codefile {

enum class FieldType {
    Byte = 0,
    Int = 1,
    UInt = 2,
    Long = 3,
    Float = 4,
    StringRef = 5,
    Object = 6,
    Pointer = 7,
};

struct FieldHeader {
    Byte Type;
};

struct ObjectHeader {
    Byte CountOfFields;
    // FieldHeader Fields[CountOfFields];
};

}
