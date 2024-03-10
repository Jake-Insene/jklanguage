#pragma once

#include "jkc/AST/Statement.h"
#include "jkc/AST/Expresion.h"
#include "jkc/AST/FunctionParameter.h"
#include <string>

namespace AST {

struct Expresion;
struct If;
struct Block;

struct Function : Statement {
    std::u8string Name;
    TypeDecl FunctionType = TypeDecl();
    List<FunctionParameter> Parameters = {};
    
    bool IsDefined = false;
    bool HasMultiReturn = false;

    Block* Body = {};
};

struct Return : Statement {
    Expresion* Value;
};

struct Var : Statement {
    std::u8string Name;
    TypeDecl VarType;
    bool IsDefined = false;
    Expresion* Value;
};

struct ConstVal : Statement {
    std::u8string Name;
    TypeDecl ConstType;
    bool IsDefined = false;
    Expresion* Value;
};

struct If : Statement {
    Expresion* Expr;
    Block* Body;
    If* Elif;
    Block* ElseBlock;
};

struct ExpresionStatement : Statement {
    Expresion* Value;
};

}
