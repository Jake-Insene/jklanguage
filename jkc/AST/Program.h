#pragma once
#include "jkc/AST/Expresion.h"
#include "jkc/AST/Statement.h"

#include <string>

namespace AST {

struct Program {
    static constexpr Program New(std::u8string Name = STR("")) {
        return Program{
            .Name = Name,
            .Statements = List<AST::Statement*>::New(0),
        };
    }

    constexpr void Destroy(this Program& Self) { 
        Self.Name.~basic_string();
        Self.Statements.Destroy();
    }

    std::u8string Name;
    List<AST::Statement*> Statements;
};

}
