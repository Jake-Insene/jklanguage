#pragma once
#include "jkc/CodeGen/Function.h"

namespace AST {

struct Program;
struct Statement;
struct Function;
struct Var;
struct ConstVal;
struct Return;
struct If;

}


namespace CodeGen {

struct EmitterState;

// Emittion
void EmitProgramStatements(EmitterState& State, AST::Program& Program);

void EmitStatement(EmitterState& State, AST::Statement* Stat);
void EmitFunction(EmitterState& State, AST::Function* ASTFn);
void EmitVar(EmitterState& State, AST::Var* Var);
void EmitConstVal(EmitterState& State, AST::ConstVal* ConstVal);

void EmitFunctionStatement(EmitterState& State, AST::Statement* Stat, Function& Fn);
void EmitFunctionReturn(EmitterState& State, AST::Return* Ret, Function& Fn);
void EmitFunctionLocal(EmitterState& State, AST::Var* Var, Function& Fn);
void EmitFunctionConstVal(EmitterState& State, AST::ConstVal* ConstVal, Function& Fn);
void EmitFunctionIf(EmitterState& State, AST::If* ConstVal, Function& Fn);

}
