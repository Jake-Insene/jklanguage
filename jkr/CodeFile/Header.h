#pragma once
#include "jkr/CodeFile/OperandTypes.h"

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
    Byte LibraryLen;
    Byte EntryLen;
    // Char Library[LibraryLen]
    // Char Entry[EntryLen]
};

struct ExportHeader {
    StringType SIndex; // Index in string table
    FunctionType FIndex;
};

struct FileHeader {
    // codefile signature
    Char Signature[8];
    // Size of the entire file
    Uint32 CheckSize;

    Uint16 Attributes;
    Byte MajorVersion;
    Byte MinorVersion;

    Uint32 CountOfFunctions;
    Uint32 CountOfObjects;
    Uint32 CountOfGlobals;
    Uint32 CountOfImports;
    Uint32 CountOfExports;
    // count of elements in the string table
    Uint32 CountOfStrings;
    // entry point of the executable
    Uint32 EntryPoint;

    // This is always after the header
    // FunctionHeader Functions[CountOfFunctions];
    // StructHeader Structs[CountOfStructs];
    // ImportHeader Imports[CountOfImports];
    // ImportHeader Exports[CountOfExports];
    // StringTable Strings;
};

}
