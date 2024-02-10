#pragma once
#include "jkr/CoreHeader.h"

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

    static constexpr Statement New(StatementType Type, Str FileName, USize Line) {
        return Statement{
            .Type = Type,
            .FileName = FileName,
            .Line = Line,
        };
    }

    void Destroy(this Statement& Self);

    StatementType Type;
    Str FileName;
    USize Line;
};

}