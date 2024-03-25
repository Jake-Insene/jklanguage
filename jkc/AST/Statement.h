#pragma once
#include "jkc/Lexer/Token.h"

namespace AST {

struct Function;
struct Return;
struct ConstVal;
struct Var;
struct If;
struct For;
struct While;
struct ExpresionStatement;

enum class StatementType {
    Unknown = 0,
    Function,
    Return,
    Var,
    ConstVal,
    If,
    For,
    While,
    ExpresionStatement,
};

struct Statement {
    constexpr Statement(StatementType Type, const SourceLocation& Location) :
        Type(Type), Location(Location) {}

    constexpr ~Statement() {}

    Statement(Statement&&) = default;
    Statement& operator=(Statement&&) = default;

    StatementType Type;
    SourceLocation Location;
};

}