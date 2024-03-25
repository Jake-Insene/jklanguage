#pragma once
#include "jkc/AST/Expresion.h"
#include "jkc/AST/Type.h"
#include "jkc/AST/Enums.h"
#include <jkr/Vector.h>
#include <memory>

namespace AST {

struct Statement;

struct Constant : Expresion {
    constexpr Constant(const SourceLocation& Location) : 
        Expresion(ExpresionType::Constant, Location), ValueType() {}

    constexpr ~Constant() {}

    union {
        Int64 Signed = 0;
        UInt64 Unsigned;
        Float64 Real;
    };
    String String;
    TypeDecl ValueType;
};

struct Identifier : Expresion {
    constexpr Identifier(const SourceLocation& Location) :
        Expresion(ExpresionType::Identifier, Location) {}

    constexpr ~Identifier() {}
    String ID;
};

struct Group : Expresion {
    constexpr Group(const SourceLocation& Location) :
        Expresion(ExpresionType::Group, Location) {}

    constexpr ~Group() {}

    std::unique_ptr<Expresion> Value;
};

struct Call : Expresion {
    constexpr Call(const SourceLocation& Location) :
        Expresion(ExpresionType::Call, Location) {}

    constexpr ~Call() {}

    std::unique_ptr<Expresion> Target;
    Vector<std::unique_ptr<Expresion>> Arguments;
};

struct BinaryOp : Expresion {
    constexpr BinaryOp(const SourceLocation& Location) :
        Expresion(ExpresionType::BinaryOp, Location) {}

    constexpr ~BinaryOp() {}

    std::unique_ptr<Expresion> Left;
    BinaryOperation Op = BinaryOperation::None;
    std::unique_ptr<Expresion> Right;
};

struct Unary : Expresion {
    constexpr Unary(const SourceLocation& Location) :
        Expresion(ExpresionType::Unary, Location) {}

    constexpr ~Unary() {}

    std::unique_ptr<Expresion> Value;
    UnaryOperation Op = UnaryOperation::None;
};

struct Dot : Expresion {
    constexpr Dot(const SourceLocation& Location) :
        Expresion(ExpresionType::Dot, Location) {}

    constexpr ~Dot() {}

    std::unique_ptr<Expresion> Left;
    std::unique_ptr<Expresion> Right;
};

struct ArrayList : Expresion {
    constexpr ArrayList(const SourceLocation& Location) :
        Expresion(ExpresionType::ArrayList, Location) {}

    constexpr ~ArrayList() {}

    Vector<std::unique_ptr<Expresion>> Elements;
};

struct Block : Expresion {
    constexpr Block(const SourceLocation& Location) :
        Expresion(ExpresionType::Block, Location) {}

    constexpr ~Block() {}

    Vector<std::unique_ptr<Statement>> Statements;
};

struct ArrayAccess : Expresion {
    constexpr ArrayAccess(const SourceLocation& Location) :
        Expresion(ExpresionType::ArrayAccess, Location) {}

    constexpr ~ArrayAccess() {}

    std::unique_ptr<Expresion> Expr;
    std::unique_ptr<Expresion> IndexExpr;
};

struct IncDec : Expresion {
    constexpr IncDec(const SourceLocation& Location) :
        Expresion(ExpresionType::IncDec, Location) {}

    constexpr ~IncDec() {}

    std::unique_ptr<Expresion> Expr;
    bool Increment;
    bool After; // The inc/dec was after the expresion
};

struct Assignment : Expresion {
    constexpr Assignment(const SourceLocation& Location) :
        Expresion(ExpresionType::Assignment, Location) {}
    constexpr ~Assignment() {}

    std::unique_ptr<Expresion> Target;
    std::unique_ptr<Expresion> Source;
};

}
