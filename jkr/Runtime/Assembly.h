#pragma once
#include "jkr/Runtime/Value.h"
#include "jkr/CodeFile/Header.h"
#include "jkr/Runtime/Function.h"
#include "jkr/Runtime/Global.h"
#include "jkr/Runtime/ListTypes.h"
#include <stdjk/String.h>

namespace runtime {

enum AssemblyError {
    AsmOk = 0,
    AsmCorruptFile = 1,
    AsmBadFile = 2,
    AsmNotExists = 3,
};

struct [[nodiscard]] Assembly : codefile::FileHeader {
    const Char* FilePath;
    List<Function> Functions;
    List<Global> Globals;
    List<ByteList> Strings;
    AssemblyError Err;

    static Assembly* FromFile(Str FilePath);

    void Destroy(this Assembly& Self);

};

}
