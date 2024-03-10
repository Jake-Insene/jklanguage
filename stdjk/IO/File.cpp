#include "stdjk/IO/File.h"
#include "stdjk/String.h"
#include "stdjk/Error.h"
#include <stdio.h>

Char HexDigits[] = {
                '0', '1', '2', '3', '4',
                '5', '6', '7', '8', '9',
                'A', 'B', 'C', 'D', 'E',
                'F',
};

Char HexDigitsD[] = {
                '0', '1', '2', '3', '4',
                '5', '6', '7', '8', '9',
                'a', 'b', 'c', 'd', 'e',
                'f',
};

extern IntPtr __io__File_GetStd(USize Who);

extern IntPtr __io__File_Create(Str FilePath, io::FileError& Err);
extern IntPtr __io__File_Open(Str FilePath, io::FileError& Err);
extern Int __io__File_Write(IntPtr Handle, const Byte* Buff, USize Count);
extern Int __io__File_Read(IntPtr Handle, Byte* Buff, USize Count);
extern USize __io__File_Size(IntPtr Handle);
extern void __io__File_Close(IntPtr Handle);

namespace io {

File File::Create(Str FilePath) {
    io::FileError ec = io::FileError::Success;
    IntPtr handle = __io__File_Create(FilePath, ec);

    return File{
        .Handle = handle,
        .Err = ec,
    };
}
File File::Open(Str FilePath) {
    io::FileError ec = io::FileError::Success;
    IntPtr handle = __io__File_Open(FilePath, ec);
    
    return File{
        .Handle = handle,
        .Err = ec,
    };
}

void File::Write(this File& Self, const Byte* Buff, USize Count) {
    RuntimeError(Self.Err == Success, STR("Use of invalid file"));
    if (__io__File_Write(Self.Handle, Buff, Count) != Success) {
        Self.Err = WriteError;
    }
}

void File::Read(this File& Self, Byte* Buff, USize Count) {
    RuntimeError(Self.Err == Success, STR("Use of invalid file"));
    if (__io__File_Read(Self.Handle, Buff, Count) != Success) {
        Self.Err = ReadError;
    }
}

USize File::Size(this File& Self) {
    RuntimeError(Self.Err == Success, STR("Use of invalid file"));
    return __io__File_Size(Self.Handle);
}

void File::PrintVa(this File& Self, Str Format, va_list Args) {
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

    if (Format[i] == '{') {
        i++;
        Char ft = Format[i++];
        start = i;

        if (ft == '\'') {
            Self.WriteArray({ '{' });
        }
        else if (ft == 's') {
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
            UInt16 c = va_arg(Args, UInt16);

            Char buff[21] = {};
            Char* rNext = UnsignedToBuff(buff + 21, IntCast<UInt>(c));
            Self.Write(Cast<const Byte*>(rNext), (buff + 21) - rNext);
        }
        else if (ft == 'd') {
            UInt32 c = va_arg(Args, UInt32);

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
        else if (ft == 'x') {
            Char buff[16] = {};
            UInt hexLen = 0;
            UInt val;
            UInt hexIndex = 0;
            
            if (Format[i] == 'q') {
                hexLen = static_cast<UInt>(8) << 1;
                val = va_arg(Args, UInt);
            }
            else if (Format[i] == 'w') {
                i++;
                hexLen = static_cast<UInt>(2) << 1;
                val = (UInt)va_arg(Args, UInt16);
            }
            else if (Format[i] == 'b') {
                i++;
                hexLen = static_cast<UInt>(1) << 1;
                val = (UInt)va_arg(Args, Byte);
            }
            else {
                hexLen = static_cast<UInt>(4) << 1;
                val = (UInt)va_arg(Args, UInt32);
            }

            hexIndex = (hexLen - 1) * 4;
            UInt index = 0;
            while (index < hexLen) {
                Char character = HexDigitsD[(val >> hexIndex) & 0x0f];

                buff[index] = character;
                ++index;
                hexIndex -= 4;
            }

            Self.Write(Cast<Byte*>(buff), Strlen(Cast<Char*>(buff)));
        }
        else if (ft == 'X') {
            Char buff[16] = {};
            UInt hexLen = 0;
            UInt val;
            UInt hexIndex = 0;

            if (Format[i] == 'q') {
                hexLen = static_cast<UInt>(8) << 1;
                val = va_arg(Args, UInt);
            }
            else if (Format[i] == 'w') {
                i++;
                hexLen = static_cast<UInt>(2) << 1;
                val = (UInt)va_arg(Args, UInt16);
            }
            else if (Format[i] == 'b') {
                i++;
                hexLen = static_cast<UInt>(1) << 1;
                val = (UInt)va_arg(Args, Byte);
            }
            else {
                hexLen = static_cast<UInt>(4) << 1;
                val = (UInt)va_arg(Args, UInt32);
            }

            hexIndex = (hexLen - 1) * 4;
            UInt index = 0;
            while (index < hexLen) {
                Char character = HexDigits[(val >> hexIndex) & 0x0f];

                buff[index] = character;
                ++index;
                hexIndex -= 4;
            }

            Self.Write(Cast<Byte*>(buff), Strlen(Cast<Char*>(buff)));
        }
        else if (ft == 'i' || ft == 's') {
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
            Char buff[128] = {};
            mem::Set(buff, 0, 128);

            sprintf_s(Cast<char*>(buff), 128, "%f", real);
            Self.Write(Cast<const Byte*>(buff), Strlen(buff+0));
        }

        RuntimeError(Format[i++] == '}', STR("Bad format string"));
        start = i;
    }

    if (Format[i] == 0)
        return;
    goto loop;
}

void File::PrintlnVa(this File& Self, Str Format, va_list Args) {
    Self.PrintVa(Format, Args);
    Self.WriteArray({'\n'});
}

void File::Close(this File& Self) {
    __io__File_Close(Self.Handle);
}

File GetStdout() {
    return File{
        .Handle = __io__File_GetStd(0),
    };
}

File GetStderr() {
    return File{
        .Handle = __io__File_GetStd(1),
    };
}

}
