#pragma once
#include "jkr/IO/File.h"

namespace io {

static inline void Print(const Char* Format, ...) {
    va_list args;
    io::File out = io::GetStdout();
    va_start(args, Format);
    out.PrintVa(Format, args);
    va_end(args);
}

static inline void Println(const Char* Format, ...) {
    va_list args;
    io::File out = io::GetStdout();
    va_start(args, Format);
    out.PrintlnVa(Format, args);
    va_end(args);
}

}
