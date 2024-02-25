#pragma once

#include "jkc/AST/Statement.h"
#include "jkc/AST/Expresion.h"
#include "jkc/AST/FunctionParameter.h"
#include "stdjk/Mem/Ptr.h"
#include <string>

namespace AST {

struct Expresion;

struct Function : Statement {
    std::u8string Name;
    TypeDecl FunctionType = TypeDecl();
    List<FunctionParameter> Parameters = {};
    
    bool IsDefined = false;

    mem::Ptr<Block> Body = {};
};

struct Return : Statement {
    mem::Ptr<Expresion> Value;
};

struct Var : Statement {
    std::u8string Name;
    TypeDecl VarType;
    bool IsDefined = false;
    mem::Ptr<Expresion> Value;
};

struct ConstVal : Statement {
    std::u8string Name;
    TypeDecl ConstType;
    bool IsDefined = false;
    mem::Ptr<Expresion> Value;
};

struct If : Statement {
    mem::Ptr<Expresion> Expr;
    mem::Ptr<Expresion> Body;
    mem::Ptr<Statement> Elif;
    mem::Ptr<Expresion> ElseBlock;
};

struct ExpresionStatement : Statement {
    mem::Ptr<Expresion> Value;
};

}
