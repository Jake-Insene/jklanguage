#pragma once
#include "jkr/Lib/List.h"
#include "jkr/Runtime/Value.h"
#include "jkr/CodeFile/Header.h"
#include "jkr/CodeFile/Function.h"

namespace runtime {

enum AssemblyError {
    AsmOk = 0,
    AsmCorruptFile = 1,
};

struct [[nodiscard]] Function {
    static constexpr Function New() {
        return Function{
            .Header = {},
            .Code = List<Byte>::New(0),
        };
    }

    constexpr void Destroy(this Function& Self) {
        Self.Code.Destroy();
    }

    codefile::FunctionHeader Header;
    List<Byte> Code;
};

struct [[nodiscard]] Import {

    static Import New() {
        return Import{};
    }

    void Destroy(this Import& /*Self*/) {}

};

struct [[nodiscard]] Assembly {
    codefile::FileHeader Header;
    const Char* FilePath;
    List<Function> Functions;
    List<Value> Globals;
    List<Import> Imports;
    AssemblyError Err;

    static Assembly JK_API FromFile(Str FilePath);

    constexpr void Destroy(this Assembly& Self) {
        Self.Functions.Destroy();
        Self.Globals.Destroy();
        Self.Imports.Destroy();
    }

};

}
