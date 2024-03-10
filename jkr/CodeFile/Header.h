#pragma once
#include "stdjk/CoreHeader.h"

namespace codefile {

static constexpr Char Signature[] = {
    CHAR('\n'),
    CHAR('#'),
    CHAR('C'),
    CHAR('D'),
    CHAR('F'),
    CHAR('L'),
    CHAR('#'),
    CHAR('\n'),
};

enum FileType : Byte {
    Executable = 0,
    Module = 1,
};

enum SectionType {
    SectionCode = 0xC0,
    SectionData = 0xD0,
    SectionTypeObject = 0x01,
    SectionConstant = 0xCC,
    SectionST = 0xCF,
    SectionCustom = 0xFF,
};

enum SectionFlags {
    SectionNone = 0x00,
    SectionWritable = 0x01,
    SectionReadable = 0x02,
    SectionExecutable = 0x04,
};

struct SectionHeader {
    Byte Type;
    Byte Flags;
    // For aot this can describe a architecture
    UInt16 UserData;
    UInt32 CountOfElements;
};

struct FileHeader {
    // CodeFile signature
    // Used to detect file corruption
    UInt64 Signature;
    // Size of the entire file
    // Used to detect file corruption
    UInt64 CheckSize;

    Byte FileType;
    Byte MajorVersion;
    Byte MinorVersion;

    // Number of sections in the file
    Byte NumberOfSections;

    // Entry point of the executable
    // is ignored in modules
    UInt32 EntryPoint;
};

}
