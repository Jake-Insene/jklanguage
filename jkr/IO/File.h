#pragma once
#include "jkr/CoreHeader.h"
#include <stdarg.h>

namespace io {

enum FileMode {
    FileRead = 0x01,
    FileWrite = 0x02,
    FileBinary = 0x04,
};

enum FileError {
    Success = 0,
    NotExists,
    WriteError,
    ReadError,
};

using FileModeInt = unsigned int;

struct [[nodiscard]] File {
    static File JK_API Open(Str FilePath, FileModeInt Mode);

    template<UInt N>
    constexpr void WriteArray(this File& Self, const Byte(&Arr)[N]) {
        Self.Write(Arr, N);
    }

    template<UInt N>
    constexpr void ReadArray(this File& Self, Byte (&Arr)[N]) {
        Self.Read(Arr, N);
    }

    void JK_API Write(this File& Self, const Byte* Buff, USize Count);
    void JK_API Read(this File& Self, Byte* Buff, USize Count);

    USize JK_API Size(this File& Self);

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

    void JK_API PrintVa(this File& Self, Str Format, va_list Args);
    void JK_API PrintlnVa(this File& Self, Str Format, va_list Args);

    void JK_API Close(this File& Self);

    IntPtr Handle;
    FileError Err;
};

File JK_API GetStdout();
File JK_API GetStderr();

}
