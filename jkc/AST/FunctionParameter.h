#pragma once
#include "jkc/AST/Type.h"

#include <string>

namespace AST {

struct FunctionParameter {
    static constexpr FunctionParameter New() {
        return FunctionParameter{};
    }

    constexpr void Destroy(this FunctionParameter& Self) {
        Self.Name.~basic_string();
    }

    std::u8string Name;
    TypeDecl Type;
};

}
