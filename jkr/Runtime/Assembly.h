#pragma once
#include "jkr/CodeFile/Header.h"
#include "jkr/Runtime/Function.h"
#include "jkr/Runtime/DataElement.h"
#include "jkr/Runtime/Array.h"
#include "jkr/Vector.h"
#include <memory>

namespace runtime {

enum AssemblyError {
    AsmOk = 0,
    AsmCorruptFile = 1,
    AsmBadFile = 2,
    AsmNotExists = 3,
};

struct [[nodiscard]] Assembly : codefile::FileHeader {
    Assembly(Str FilePath);
    ~Assembly();

    const Char* FilePath;
    AssemblyError Err;

    Vector<Function> CodeSection = {};
    Vector<DataElement> DataSection = {};
    Vector<Array> STSection = {};
};

}
