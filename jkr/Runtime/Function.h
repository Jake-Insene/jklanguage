#pragma once
#include "jkr/CodeFile/Function.h"
#include "jkr/Runtime/Value.h"
#include <vector>

namespace runtime {

struct Assembly;

struct [[nodiscard]] Function : codefile::FunctionHeader {
    constexpr Function() {}

    constexpr ~Function() {}

    std::vector<Byte> Code;
    Value(*Native)(...) = nullptr;
    Assembly* Asm;
};

}
