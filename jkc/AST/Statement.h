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

enum StatementAttributes {
    AttributeNone,
    AttributeIntrinsec,
    AttributeInternal,
    AttributeNative,
};

struct Statement {

    static constexpr Statement New(StatementType Type, const SourceLocation& Location) {
        return Statement{
            .Type = Type,
            .Location = Location,
        };
    }

    void Destroy(this Statement& Self);

    StatementType Type;
    SourceLocation Location;
};

}