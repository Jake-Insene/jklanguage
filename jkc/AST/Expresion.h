#pragma once
#include "jkc/Lexer/Token.h"
#include "stdjk/List.h"

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
struct ArrayAccess;

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
    ArrayAccess,
};

struct Expresion {

    static Expresion New(ExpresionType Type, const SourceLocation& Location, List<Attribute> Attribs = {}) {
        return Expresion{
            .Type = Type,
            .Location = Location,
            .Attribs = Attribs,
        };
    }

    void Destroy(this Expresion& Self);

    ExpresionType Type;
    SourceLocation Location;
    List<Attribute> Attribs;
};

}
