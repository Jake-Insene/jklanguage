#pragma once
#include "jkr/CodeFile/Global.h"
#include "jkr/Runtime/Value.h"

namespace runtime {

struct Global : codefile::GlobalHeader {
    static constexpr Global New() {
        return Global{};
    }

    constexpr void Destroy(this Global& /*Self*/) {}

    Value Contant;
};

}
