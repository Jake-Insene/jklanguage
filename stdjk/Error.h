#pragma once
#include "stdjk/CoreHeader.h"
#include "stdjk/Debug.h"
#include "stdjk/IO/Output.h"

#define RuntimeError(Expr, ...) { if(!(Expr)) { \
    io::File err = io::GetStderr();\
    err.Print(STR("Runtime Error at '{s}:{d}':\n\t"), __FILE__, __LINE__);\
    err.Println(__VA_ARGS__);\
    debug::Break(); \
} }

namespace error {

void Exit(UInt Code);

}
