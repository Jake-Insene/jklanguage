#pragma once
#include "jkr/Runtime/Value.h"
#include "jkr/CodeFile/Header.h"
#include "jkr/Runtime/Function.h"
#include "jkr/Runtime/DataElement.h"
#include "jkr/Runtime/Array.h"
#include <stdjk/String.h>

namespace runtime {

enum AssemblyError {
    AsmOk = 0,
    AsmCorruptFile = 1,
    AsmBadFile = 2,
    AsmNotExists = 3,
};

struct [[nodiscard]] Section : codefile::SectionHeader {
    static constexpr Section New() {
        return Section{
        };
    }

    constexpr void Destroy(this Section& Self) {
        if(Self.Type == codefile::SectionCode)
        {
            Self.Functions.Destroy();
        }
        else if (Self.Type == codefile::SectionData) {
            Self.Data.Destroy();
        }
        else if (Self.Type == codefile::SectionST) {
            Self.Strings.Destroy();
        }
    }

    union {
        List<Function> Functions;
        List<DataElement> Data;
        List<Array> Strings;
    };
};

struct [[nodiscard]] Assembly : codefile::FileHeader {
    const Char* FilePath;
    List<Section> Sections;
    AssemblyError Err;

    Section* CodeSection;
    Section* DataSection;
    Section* STSection;

    static Assembly* FromFile(Str FilePath);

    void Destroy(this Assembly& Self);

};

}
