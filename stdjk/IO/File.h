#pragma once
#include "stdjk/CoreHeader.h"
#include <stdarg.h>

namespace io {

enum FileError {
    Success = 0,
    NotExists,
    WriteError,
    ReadError,
};

struct [[nodiscard]] File {
    static File Create(Str FilePath);
    static File Open(Str FilePath);

    template<UInt N>
    constexpr void WriteArray(this File& Self, const Byte(&Arr)[N]) {
        Self.Write(Arr, N);
    }

    template<UInt N>
    constexpr void ReadArray(this File& Self, Byte (&Arr)[N]) {
        Self.Read(Arr, N);
    }

    void Write(this File& Self, const Byte* Buff, USize Count);
    void Read(this File& Self, Byte* Buff, USize Count);

    USize Size(this File& Self);

    void Print(this File& Self, Str Format, ...) {
        va_list args;
        va_start(args, Format);
        Self.PrintVa(Format, args);
        va_end(args);
    }

    void Println(this File& Self, Str Format, ...) {
        va_list args;
        va_start(args, Format);
        Self.PrintlnVa(Format, args);
        va_end(args);
    }

    void PrintVa(this File& Self, Str Format, va_list Args);
    void PrintlnVa(this File& Self, Str Format, va_list Args);

    void Close(this File& Self);

    IntPtr Handle;
    FileError Err;
};

File GetStdout();
File GetStderr();

}
