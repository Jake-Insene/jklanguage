#pragma once
#include "jkr/CodeFile/Type.h"

namespace codefile {

struct FieldHeader {
    Type FieldType;
};

struct ObjectHeader {
    Byte CountOfFields;
    // FieldHeader Fields[CountOfFields];
};

}
