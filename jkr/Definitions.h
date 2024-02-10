#pragma once

#define STR(S) u8 ## S
#define CHAR(c) u8 ## c
#define VAL(n) n
#define MAKE_STR(v) STR(##VAL(v))

#ifdef _MSVC_LANG
    #define JK_EXPORT __declspec(dllexport)
    #define JK_IMPORT __declspec(dllimport)
#endif // _MSVC_LANG

#ifdef JK_BUILD
    #define JK_API JK_EXPORT
#else
    #define JK_API JK_IMPORT
#endif // JK_BUILD

#ifdef _MSVC_LANG
    #define UNREACHABLE __assume(true)
#else
    #define UNREACHABLE
#endif // _MSVC_LANG

