#pragma once
#include "jkc/CodeGen/Function.h"

namespace AST {

struct Expresion;
struct Constant;
struct Identifier;
struct Call;
struct BinaryOp;
struct Unary;
struct Dot;
struct Block;
struct ArrayList;
struct ArrayAccess;
struct IncDec;
struct Assignment;

}

namespace CodeGen {

struct EmitterState;

// Expresion Emittion
TmpValue EmitExpresion(EmitterState& State, AST::Expresion* Expr);

// Function Expresion Emittion
TmpValue EmitFunctionExpresion(EmitterState& State, AST::Expresion* Expr, Function& Fn);
TmpValue EmitFunctionConstant(EmitterState& State, AST::Constant* Constant, Function& Fn);
TmpValue EmitFunctionIdentifier(EmitterState& State, AST::Identifier* ID, Function& Fn);
TmpValue EmitFunctionCall(EmitterState& State, AST::Call* Call, Function& Fn);

TmpValue EmitFunctionBinaryOp(EmitterState& State, AST::BinaryOp* BinOp, Function& Fn);
TmpValue EmitBinaryOp(EmitterState& State, TmpValue& Left, TmpValue& Right, AST::BinaryOperation Op, Function& Fn);
TmpValue EmitFunctionAssignment(EmitterState& State, TmpValue& Left, TmpValue& Right, Function& Fn);

TmpValue EmitFunctionUnary(EmitterState& State, AST::Unary* Unary, Function& Fn);
TmpValue EmitFunctionDot(EmitterState& State, AST::Dot* Dot, Function& Fn);
TmpValue EmitFunctionArrayList(EmitterState& State, AST::ArrayList* ArrayList, Function& Fn);
TmpValue EmitFunctionBlock(EmitterState& State, AST::Block* Block, Function& Fn);
TmpValue EmitFunctionArrayAccess(EmitterState& State, AST::ArrayAccess* ArrayAccess, Function& Fn);
TmpValue EmitFunctionIncDec(EmitterState& State, AST::IncDec* IncDec, Function& Fn);
TmpValue EmitFunctionAssignment(EmitterState& State, AST::Assignment* Assignment, Function& Fn);

}
