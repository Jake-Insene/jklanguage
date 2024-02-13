#pragma once
#include "jkc/AST/Expresion.h"
#include "jkc/AST/Statement.h"
#include "jkr/Lib/List.h"
#include "jkr/Mem/Ptr.h"

#include <vector>
#include <memory>
#include <string>

namespace AST {

struct Program {
    static constexpr Program New(Str Name = STR("")) {
        return Program{
            .Name = Name,
            .Statements = List<mem::Ptr<AST::Statement>>::New(0),
        };
    }

    constexpr void Destroy(this Program& Self) { 
        Self.Statements.Destroy();
    }

    Str Name;
    List<mem::Ptr<AST::Statement>> Statements;
};

}
