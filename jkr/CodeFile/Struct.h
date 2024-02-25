#pragma once
#include "jkr/CodeFile/Type.h"

namespace codefile {

struct FieldHeader {
    Byte FieldType;
    Byte FieldAttributes;
};

struct StructHeader {
    Byte CountOfFields;
    // A static field is a global
    // FieldHeader Fields[CountOfFields];
};

}
