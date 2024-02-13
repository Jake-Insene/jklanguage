#pragma once

#define STR(S) u8 ## S
#define CHAR(c) u8 ## c
#define VAL(n) n
#define MAKE_STR(v) STR(##VAL(v))

#define PLATFORM_UNKNOWN 0
#define PLATFORM_WINDOWS 1
#define PLATFORM_UNIX 1
#define PLATFORM_LINUX 1
#define PLATFORM_MACOS 1

#define ARCHITECTURE_UNKNOWN 0
#define ARCHITECTURE_X64 1

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM PLATFORM_WINDOWS
#else
    #define PLATFORM PLATFORM_UNKNOWN
#endif // _WIN32 || _WIN64

#if defined(_M_X64)
    #define ARCHITECTURE ARCHITECTURE_X64
#else
    #define ARCHITECTURE ARCHITECTURE_UNKNOWN
#endif // _M_X64

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

