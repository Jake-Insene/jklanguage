#pragma once
#include "jkr/CoreHeader.h"

namespace codefile {

static constexpr Char Signature[] = {
    CHAR('\n'),
    CHAR('C'),
    CHAR('D'),
    CHAR('F'),
    CHAR('L'),
    CHAR('\n'),
    CHAR('!'),
    CHAR('!'),
};

enum FileAttributes {
    AttributeNone = 0x0,
    AttributeExecutable = 0x01,
};

struct ImportHeader {
    Byte CountOfArguments;
    Byte LibraryLen;
    Byte EntryLen;
    // Char Library[LibraryLen]
    // Char Entry[EntryLen]
};

struct FileHeader {
    // codefile signature
    Char Signature[8];
    // Size of the entire file
    unsigned int CheckSize;

    unsigned short Attributes;
    Byte MajorVersion;
    Byte MinorVersion;

    unsigned int CountOfFunctions;
    unsigned int CountOfObjects;
    unsigned int CountOfGlobals;
    unsigned int CountOfImports;
    // count of elements in the string table
    unsigned int CountOfStrings;
    // entry point of the executable
    unsigned int EntryPoint;

    // This is always after the header
    // FunctionHeader Functions[CountOfFunctions];
    // StructHeader Structs[CountOfStructs];
    // ImportHeader Imports[CountOfImports];
    // StringTable Strings;
};

}
