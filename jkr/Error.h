#pragma once
#include "jkr/CoreTypes.h"
#include "jkr/Definitions.h"
#include <stdlib.h>
#include <utility>

#define RuntimeError(Expr, ...) { if(!(Expr)) { \
    fprintf(stderr, "Runtime Error at '%s:%d':\n\t", __FILE__, __LINE__);\
    fprintf(stderr, __VA_ARGS__);\
    Break(); \
} }

namespace error {

static inline void Exit(UInt Code) {
    std::exit(int(Code));
    UNREACHABLE();
}

}
