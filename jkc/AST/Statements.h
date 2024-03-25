#pragma once
#include "jkc/AST/Statement.h"
#include "jkc/AST/Expresion.h"
#include "jkc/AST/FunctionParameter.h"
#include <jkr/Vector.h>
#include <memory>

namespace AST {

struct Expresion;
struct If;
struct Block;

struct Function : Statement {
    constexpr Function(const SourceLocation& Location) :
        Statement(StatementType::Function, Location) {}

    constexpr ~Function() {}

    String Name;
    TypeDecl FunctionType = TypeDecl();
    Vector<FunctionParameter> Parameters = {};
    
    bool IsDefined = false;
    bool HasMultiReturn = false;
    bool IsExtern = false;
    String LibraryRef;

    std::unique_ptr<Block> Body = {};
};

struct Return : Statement {
    constexpr Return(const SourceLocation& Location) :
        Statement(StatementType::Return, Location) {}

    constexpr ~Return() {}

    std::unique_ptr<Expresion> Value;
};

struct Var : Statement {
    constexpr Var(const SourceLocation& Location) :
        Statement(StatementType::Var, Location) {}

    constexpr ~Var() {}

    String Name;
    TypeDecl VarType;
    bool IsDefined = false;
    std::unique_ptr<Expresion> Value;
};

struct ConstVal : Statement {
    constexpr ConstVal(const SourceLocation& Location) :
        Statement(StatementType::ConstVal, Location) {}

    constexpr ~ConstVal() {}

    String Name;
    TypeDecl ConstType;
    bool IsDefined = false;
    std::unique_ptr<Expresion> Value;
};

struct If : Statement {
    constexpr If(const SourceLocation& Location) :
        Statement(StatementType::If, Location) {}

    constexpr ~If() {}

    std::unique_ptr<Expresion> Expr;
    std::unique_ptr<Block> Body;
    std::unique_ptr<If> Elif;
    std::unique_ptr<Block> ElseBlock;
};

struct ExpresionStatement : Statement {
    constexpr ExpresionStatement(const SourceLocation& Location) :
        Statement(StatementType::ExpresionStatement, Location) {}

    constexpr ~ExpresionStatement() {}

    std::unique_ptr<Expresion> Value;
};

}
