#pragma once
#include "jkc/AST/Expresion.h"
#include "jkc/AST/Statement.h"
#include <jkr/Vector.h>
#include <jkr/String.h>
#include <memory>

namespace AST {

struct Program {
    using StatementList = Vector<std::unique_ptr<AST::Statement>>;

    constexpr Program(String Name) : 
        Name(Name), Statements() {}

    constexpr ~Program() {}

    Program(Program&&) = default;
    Program& operator=(Program&&) = default;

    String Name;
    StatementList Statements;
};

}
