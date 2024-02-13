#include "jkr/IO/File.h"
#include "jkr/Lib/String.h"
#include "jkr/Lib/Error.h"

#include <stdio.h>

// Implemented in Impl/Platform/PlatformXXFile.cpp
extern IntPtr __io__File_GetStd(USize Who);

extern IntPtr __io__File_Open(Str FilePath, io::FileMode Mode, io::FileError& Err);
extern Int __io__File_Write(IntPtr Handle, const Byte* Buff, USize Count);
extern Int __io__File_Read(IntPtr Handle, Byte* Buff, USize Count);
extern USize __io__File_Size(IntPtr Handle);
extern void __io__File_Close(IntPtr Handle);

namespace io {

File JK_API File::Open(Str FilePath, FileModeInt Mode) {
    io::FileError ec = io::FileError::Success;
    IntPtr handle = __io__File_Open(FilePath, (io::FileMode)Mode, ec);
    
    return File{
        .Handle = handle,
        .Err = ec,
    };
}

void JK_API File::Write(this File& Self, const Byte* Buff, USize Count) {
    RuntimeError(Self.Err == Success, STR("Use of invalid file"));
    if (__io__File_Write(Self.Handle, Buff, Count) != Success) {
        Self.Err = WriteError;
    }
}

void JK_API File::Read(this File& Self, Byte* Buff, USize Count) {
    RuntimeError(Self.Err == Success, STR("Use of invalid file"));
    if (__io__File_Read(Self.Handle, Buff, Count) != Success) {
        Self.Err = ReadError;
    }
}

USize JK_API File::Size(this File& Self) {
    RuntimeError(Self.Err == Success, STR("Use of invalid file"));
    return __io__File_Size(Self.Handle);
}

void JK_API File::PrintVa(this File& Self, Str Format, va_list Args) {
    RuntimeError(Self.Err == Success, STR("Use of invalid file"));
    USize i = 0;
    USize start = 0;

loop:
    while (Format[i] != 0) {
        if (Format[i] == '{')
            break;
        i++;
    }

    if (Format[i] == 0 && start == 0) {
        Self.Write(Cast<const Byte*>(Format), i);
        return;
    }

    Self.Write(Cast<const Byte*>(Format + start), (i - start));

    Char ft = 0;
    if (Format[i] == '{') {
        i++;
        ft = Format[i++];
        RuntimeError(Format[i++] == '}', STR("Bad format string"));
        start = i;
    }
    else
        goto end;

    if (ft == 's') {
        const Char* str = va_arg(Args, const Char*);
        Self.Write(Cast<const Byte*>(str), Strlen(str));
    }
    else if (ft == 'c') {
        Char c = va_arg(Args, Char);
        Self.WriteArray({ c });
    }
    else if (ft == 'b') {
        Byte c = va_arg(Args, Byte);

        Char buff[21] = {};
        Char* rNext = UnsignedToBuff(buff + 21, IntCast<UInt>(c));
        Self.Write(Cast<const Byte*>(rNext), (buff + 21) - rNext);
    }
    else if (ft == 'w') {
        Uint16 c = va_arg(Args, Uint16);

        Char buff[21] = {};
        Char* rNext = UnsignedToBuff(buff + 21, IntCast<UInt>(c));
        Self.Write(Cast<const Byte*>(rNext), (buff + 21) - rNext);
    }
    else if (ft == 'd') {
        Uint32 c = va_arg(Args, Uint32);

        Char buff[21] = {};
        Char* rNext = UnsignedToBuff(buff + 21, IntCast<UInt>(c));
        Self.Write(Cast<const Byte*>(rNext), (buff + 21) - rNext);
    }
    else if (ft == 'q' || ft == 'u') {
        UInt c = va_arg(Args, UInt);

        Char buff[21] = {};
        Char* rNext = UnsignedToBuff(buff + 21, IntCast<UInt>(c));
        Self.Write(Cast<const Byte*>(rNext), (buff + 21) - rNext);
    }
    else if (ft == 'i' || 's') {
        Int c = va_arg(Args, Int);

        Char buff[21] = {};
        Char* rNext = buff + 21;
        const UInt uVal = UInt(c);
        if (c < 0) {
            rNext = UnsignedToBuff(rNext, 0 - uVal);
            *--rNext = '-';
        }
        else {
            rNext = UnsignedToBuff(rNext, uVal);
        }

        Self.Write(Cast<const Byte*>(rNext), (buff + 21) - rNext);
    }
    else if (ft == 'f') {
        Float real = va_arg(Args, Float);
        Char buff[21] = {};
        sprintf_s(Cast<char*>(buff), 21, "%f", real);
        Self.Write(Cast<const Byte*>(buff), Strlen(buff));
    }

end:
    if (Format[i] == 0)
        return;
    goto loop;
}

void JK_API File::PrintlnVa(this File& Self, Str Format, va_list Args) {
    Self.PrintVa(Format, Args);
    Self.WriteArray({'\n'});
}

void JK_API File::Close(this File& Self) {
    __io__File_Close(Self.Handle);
}

File JK_API GetStdout() {
    return File{
        .Handle = __io__File_GetStd(0),
    };
}

File JK_API GetStderr() {
    return File{
        .Handle = __io__File_GetStd(1),
    };
}

}
