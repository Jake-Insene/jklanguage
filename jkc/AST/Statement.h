#pragma once
#include "jkc/Lexer/Token.h"
#include "stdjk/List.h"

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

    static Statement New(StatementType Type, const SourceLocation& Location, List<Attribute> Attribs = {}) {
        return Statement{
            .Type = Type,
            .Location = Location,
            .Attribs = Attribs,
        };
    }

    void Destroy(this Statement& Self);

    StatementType Type;
    SourceLocation Location;
    List<Attribute> Attribs;
};

}