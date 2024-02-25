#pragma once

#include "jkc/AST/Expresion.h"
#include "jkc/AST/Type.h"
#include "jkc/AST/Enums.h"
#include "stdjk/Mem/Ptr.h"

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
    mem::Ptr<Expresion> Value;
};

struct Call : Expresion {
    mem::Ptr<Expresion> Target;
    List<mem::Ptr<Expresion>> Arguments;
};

struct BinaryOp : Expresion {
    mem::Ptr<Expresion> Left;
    BinaryOperation Op = BinaryOperation::None;
    mem::Ptr<Expresion> Right;
};

struct Unary : Expresion {
    mem::Ptr<Expresion> Value;
    UnaryOperation Op = UnaryOperation::None;
};

struct Dot : Expresion {
    mem::Ptr<Expresion> Left;
    mem::Ptr<Expresion> Right;
};

struct ArrayList : Expresion {
    List<mem::Ptr<Expresion>> Elements;
};

struct Block : Expresion {
    bool HasReturn = false;
    List<mem::Ptr<Statement>> Statements;
};

}
