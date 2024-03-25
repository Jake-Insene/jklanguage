#pragma once
#include "jkr/CoreTypes.h"

namespace codefile {

static constexpr Char Signature[] = {
    'C',
    'D',
    'F',
    'L',
};

enum FileType : Byte {
    Executable = 0,
    Library = 1,
};

struct FileHeader {
    // CodeFile signature
    // Used to detect file corruption
    UInt32 Signature;
    // Size of the entire file
    // Used to detect file corruption
    UInt32 CheckSize;

    Byte FileType;
    Byte Flags;
    UInt16 MajorVersion;
    UInt16 MinorVersion;

    // Number of sections in the file
    UInt16 DataSize;
    UInt32 FunctionSize;
    UInt32 StringsSize;

    // Entry point of the executable
    // is ignored in modules
    UInt32 EntryPoint;
};

}
