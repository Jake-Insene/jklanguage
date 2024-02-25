#include "jkr/Runtime/Library.h"
#include <Windows.h>

Library Library::New(Str FilePath) {
    IntPtr h = Cast<IntPtr>(
        LoadLibraryA(Cast<LPCSTR>(FilePath))
    );
    return Library{
        .Handle = h,
        .FilePath = FilePath,
    };
}

Procedure Library::Get(this Library& Self, Str Entry) {
    return Cast<Procedure>(
        GetProcAddress(
            Cast<HMODULE>(Self.Handle),
            Cast<LPCSTR>(Entry)
        )
    );
}

void Library::Destroy(this Library& Self) {
    FreeLibrary(Cast<HMODULE>(Self.Handle));

}

