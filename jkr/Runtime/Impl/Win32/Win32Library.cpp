#include "jkr/Runtime/Library.h"
#include <Windows.h>

Library::Library(const String& FilePath) :
    FilePath(FilePath)
{
    Handle = reinterpret_cast<IntPtr>(
        LoadLibraryA((LPCSTR)FilePath.data())
    );
}

Procedure Library::Get(Str Entry) {
    return reinterpret_cast<Procedure>(
        GetProcAddress(
            (HMODULE)Handle,
            (LPCSTR)Entry
        )
    );
}

Library::~Library() {
    if(Handle)
    {
        FreeLibrary((HMODULE)Handle);
    }
}

