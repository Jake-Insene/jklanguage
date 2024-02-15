#pragma once
#include "jkc/Lexer/Token.h"

namespace AST {

struct Statement;
struct Constant;
struct Identifier;
struct Group;
struct Call;
struct BinaryOp;
struct Unary;
struct Dot;
struct ArrayList;
struct Block;

enum class ExpresionType {
    Unknown = 0,

    Constant,
    Identifier,
    Group,
    Call,
    BinaryOp,
    Unary,
    Dot,
    ArrayList,
    Block,
};

struct Expresion {

    static constexpr Expresion New(ExpresionType Type, const SourceLocation& Location) {
        return Expresion{
            .Type = Type,
            .Location = Location,
        };
    }

    void Destroy(this Expresion& Self);

    ExpresionType Type;
    SourceLocation Location;
};

}
