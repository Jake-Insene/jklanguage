#pragma once
#include "jkr/CoreTypes.h"
#include "jkr/String.h"

using Procedure = void(*)();

struct Library {
    IntPtr Handle = 0;
    String FilePath;

    Library() {}

    Library(const String& FilePath);
    ~Library();

    Library(Library&&) = default;
    Library& operator=(Library&&) = default;

    Procedure Get(Str Entry);
};
