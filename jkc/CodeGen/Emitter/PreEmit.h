#pragma once

namespace AST {

struct Program;

}

namespace CodeGen {

struct EmitterState;

void PreEmit(EmitterState& State, AST::Program& Program);

// Pre emittion
void PreDeclareStatement(EmitterState& State, AST::Statement* Stat);
void PreDeclareFunction(EmitterState& State, AST::Function* ASTFn);
void PreDeclareVar(EmitterState& State, AST::Var* Var);
void PreDeclareConstVal(EmitterState& State, AST::ConstVal* ConstVal);

}
