#pragma once
#include "jkr/IO/Output.h"

namespace debug {

static inline void Break() {
#ifdef _WIN32
    __debugbreak();
#else
    __builtin_trap();
#endif // _WIN32

#ifdef NDEBUG
    UNREACHABLE;
#endif // NDEBUG
}

#ifdef NDEBUG

constexpr void Print() {}
constexpr void Assert(bool, const Char*, UInt, const Char*) {}

#define DebugAssert(Expr)
#else

static inline void Print(const Char* Format, ...) {
    va_list args;
    va_start(args, Format);
    io::File err = io::GetStderr();
    err.PrintVa(Format, args);
    va_end(args);
}

static inline void Assert(bool Val, const Char* FileName, UInt Line, const Char* Msg) {
    if (!Val) {
        io::File err = io::GetStderr();
        err.Println(STR("Assertion at {s}:{u}:\n\t{s}"), FileName, Line, Msg);
        debug::Break();
    }
}

#define DebugAssert(Expr) { debug::Assert(Expr, Cast<const Char*>(__FILE__), __LINE__, STR(#Expr)); }

#endif // NDEBUG

}

