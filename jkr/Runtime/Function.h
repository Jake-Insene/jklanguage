#pragma once
#include "stdjk/List.h"
#include "jkr/CodeFile/Function.h"
#include "jkr/Runtime/Value.h"

struct ThreadState;

namespace runtime {

struct Assembly;

struct [[nodiscard]] Function : codefile::FunctionHeader {
    static constexpr Function New() {
        return Function{
            .Code = List<Byte>::New(0),
        };
    }

    constexpr void Destroy(this Function& Self) {
        Self.Code.Destroy();
    }

    List<Byte> Code;
    Value(*Native)(ThreadState*, ...);
    Assembly* Asm;
};

}
