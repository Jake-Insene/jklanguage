#pragma once
#include "jkr/CoreHeader.h"
#include "jkr/Lib/Debug.h"
#include "jkr/IO/Output.h"

#define RuntimeError(Expr, ...) { if(!(Expr)) { \
    io::File err = io::GetStderr(); \
    err.Print(STR("Runtime-Error at {s}:{u}:\n\t"), __FILE__, __LINE__); \
    err.Println(__VA_ARGS__); \
    debug::Break(); \
} }

namespace error {

void JK_API Exit(UInt Code);

}
