#pragma once
#include "jkr/Lib/List.h"
#include "jkr/Runtime/Value.h"

namespace runtime {

struct [[nodiscard]] Object {
    List<Value> Fields;
};

}
