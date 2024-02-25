#pragma once
#include "stdjk/CoreHeader.h"

using Procedure = void(*)();

struct Library {
    IntPtr Handle = 0;
    Str FilePath;

    static Library New(Str FilePath);

    Procedure Get(this Library& Self, Str Entry);

    void Destroy(this Library& Self);
};
