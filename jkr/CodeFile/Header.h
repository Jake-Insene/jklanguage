#pragma once
#include "jkr/CodeFile/OperandTypes.h"

namespace codefile {

static constexpr Char Signature[] = {
    CHAR('C'),
    CHAR('D'),
    CHAR('F'),
    CHAR('L'),
};

enum FileType : Byte {
    Executable = 0,
    Module = 1,
};

struct FileHeader {
    // CodeFile signature
    // Used to detect file corruption
    UInt32 Signature;
    // Size of the entire file
    // Used to detect file corruption
    UInt32 CheckSize;

    Byte FileType;
    Byte MajorVersion;
    Byte MinorVersion;
    Byte Padding;

    
    // Number of functions in the assembly
    UInt32 NumberOfFunctions;

    // Number of struct headers in the assembly
    UInt32 NumberOfStructs;

    // Number of globals in the assembly
    UInt16 NumberOfGlobals;

    // count of elements in the string table
    UInt16 NumberOfStrings;

    // Entry point of the executable
    // is ignored in modules
    UInt32 EntryPoint;

    // This is always after the header
    // FunctionHeader Functions[NumberOfFunctions];
    // StructHeader Structs[NumberOfStructs];
    // GlobalHeader Globals[NumberOfGlobals];
    // StringTable Strings;
};

}
