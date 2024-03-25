#pragma once
#include "jkc/AST/Type.h"
#include <jkr/String.h>

namespace AST {

struct FunctionParameter {
    constexpr FunctionParameter() {}

    constexpr ~FunctionParameter() {}

    StringView Name;
    TypeDecl Type;
};

}
