#pragma once
#include "jkr/CoreTypes.h"

namespace codefile {

struct FieldHeader {
    Byte FieldType;
};

struct ObjectHeader {
    Byte CountOfFields;
    // A static field is a global
    // FieldHeader Fields[CountOfFields];
};

}
