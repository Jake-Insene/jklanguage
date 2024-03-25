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
struct ArrayAccess;
struct IncDec;
struct Assignment;

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
    IncDec,
    Assignment,
};

struct Expresion {
    constexpr Expresion(ExpresionType Type, const SourceLocation& Location) :
        Type(Type), Location(Location) {}
    constexpr ~Expresion() {}

    Expresion(Expresion&&) = default;
    Expresion& operator=(Expresion&&) = default;

    ExpresionType Type;
    SourceLocation Location;
};

}
