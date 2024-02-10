#pragma once
#include "jkr/CoreHeader.h"

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

    static constexpr Expresion New(ExpresionType Type, Str FileName, USize Line) {
        return Expresion{
            .Type = Type,
            .FileName = FileName,
            .Line = Line,
        };
    }

    void Destroy(this Expresion& Self);

    ExpresionType Type;
    Str FileName;
    USize Line;
};

}
