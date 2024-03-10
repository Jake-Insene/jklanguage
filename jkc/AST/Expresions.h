#pragma once

#include "jkc/AST/Expresion.h"
#include "jkc/AST/Type.h"
#include "jkc/AST/Enums.h"

#include <string>

namespace AST {

struct Statement;

struct Constant : Expresion {
    union {
        Int64 Signed = 0;
        UInt64 Unsigned;
        Float64 Real;
    };
    std::u8string String;
    TypeDecl ValueType;
};

struct Identifier : Expresion {
    std::u8string ID;
};

struct Group : Expresion {
    Expresion* Value;
};

struct Call : Expresion {
    Expresion* Target;
    List<Expresion*> Arguments;
};

struct BinaryOp : Expresion {
    Expresion* Left;
    BinaryOperation Op = BinaryOperation::None;
    Expresion* Right;
};

struct Unary : Expresion {
    Expresion* Value;
    UnaryOperation Op = UnaryOperation::None;
};

struct Dot : Expresion {
    Expresion* Left;
    Expresion* Right;
};

struct ArrayList : Expresion {
    List<Expresion*> Elements;
};

struct Block : Expresion {
    List<Statement*> Statements;
};

struct ArrayAccess : Expresion {
    Expresion* Expr;
    Expresion* IndexExpr;
};

}
