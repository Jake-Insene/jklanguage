#pragma once

#if defined(_MSC_VER)
    #define JK_EXPORT __declspec(dllexport)
    #define JK_IMPORT __declspec(dllimport)
#endif // _MSC_VER

#define PACK(STRUCT) \
    __pragma(pack(push, 1)) \
    STRUCT \
    __pragma(pack(pop))

#if defined(_MSC_VER)
    #define UNREACHABLE() __assume(true)
    #define Break() __debugbreak()
#else
    #define UNREACHABLE
#endif // _MSC_VER

