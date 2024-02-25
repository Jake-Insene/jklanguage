#pragma once

#define STR(S) u8 ## S
#define CHAR(c) u8 ## c
#define VAL(n) n
#define MAKE_STR(v) STR(##VAL(v))

constexpr unsigned int Undefined = 0;

enum _Platform {
    Windows = 1,
    Linux,
    Unix,
};

enum _Architecture {
    X64 = 1,
};

#if defined(_WIN32) || defined(_WIN64)
constexpr unsigned int Platform = Windows;
#else
constexpr unsigned int Platform = Undefined;
#endif // _WIN32 || _WIN64

#if defined(_M_X64)
constexpr unsigned int Architecture = X64;
#else
constexpr unsigned int Architecture = Undefined;
#endif // _M_X64

#if defined(_MSC_VER)
    #define JK_EXPORT __declspec(dllexport)
    #define JK_IMPORT __declspec(dllimport)
#endif // _MSC_VER

#define PACK(STRUCT) \
    __pragma(pack(push, 1)) \
    STRUCT \
    __pragma(pack(pop))

#if defined(_MSC_VER)
    #define UNREACHABLE __assume(true)
#else
    #define UNREACHABLE
#endif // _MSC_VER

