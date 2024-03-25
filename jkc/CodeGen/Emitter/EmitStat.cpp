#include "jkc/CodeGen/Emitter/EmitStat.h"
#include "jkc/CodeGen/Emitter/EmitExpr.h"
#include "jkc/CodeGen/Emitter/EmitterState.h"
#include "jkc/AST/Statements.h"
#include "jkc/AST/Expresions.h"
#include <jkr/Utility.h>

namespace CodeGen {

void EmitProgramStatements(EmitterState& State, AST::Program& Program) {
    for (auto& stat : Program.Statements) {
        EmitStatement(State, stat.get());
    }
}

void EmitStatement(EmitterState& State, AST::Statement* Stat) {
    if (Stat->Type == AST::StatementType::Function) {
        EmitFunction(State, (AST::Function*)Stat);
    }
    else if (Stat->Type == AST::StatementType::Var) {
        EmitVar(State, (AST::Var*)Stat);
    }
    else if (Stat->Type == AST::StatementType::ConstVal) {
        EmitConstVal(State, (AST::ConstVal*)Stat);
    }
}

void EmitFunction(EmitterState& State, AST::Function* ASTFn) {
    if (!ASTFn->IsDefined && !ASTFn->IsExtern)
        return;

    auto& fn = State.Functions.Get(State.Functions.Find(ASTFn->Name)->second);

    if (ASTFn->IsDefined) {
        // Prologue
        if (fn.RegisterArguments) {
            for (Byte i = 1; i <= fn.RegisterArguments; i++)
                State.Registers[i].IsAllocated = true;
        }

        {
            UInt32 i = 0;
            for (auto& stat : ASTFn->Body->Statements) {
                if ((static_cast<unsigned long long>(i) + 1) == ASTFn->Body->Statements.size()) {
                    State.Context.IsLast = true;
                }
                EmitFunctionStatement(State, stat.get(), fn);
                State.Context.IsLast = false;
                i++;
            }
        }

        for (auto& toResolve : fn.ResolveReturns) {
            UInt16 address = UInt16(fn.Code.Buff.size() - (toResolve.IP + 2));
            if (address <= Const16Max) {
                UInt16& jmp = *(UInt16*)&fn.Code.Buff[toResolve.IP];
                jmp = address;
            }
            else {
                State.Error(ASTFn->Location, u8"Code too long");
                return;
            }
        }

        // Epilogue
        Byte deleter = State.AllocateRegister();
        if (deleter == 0) {
            deleter = State.AllocateRegister();
            State.DeallocateRegister(0);
        }

        for (auto& local : fn.Locals.Items) {
            if (!local.Type.HasConst() && local.Type.HasArray() && !local.Type.IsConstString()) {
                if (local.IsRegister) {
                    State.CodeAssembler.ArrayDestroy(fn, local.Reg);
                }
                else {
                    State.CodeAssembler.LocalGet(fn, deleter, local.Index);
                    State.CodeAssembler.ArrayDestroy(fn, deleter);
                }
            }
        }
        State.DeallocateRegister(deleter);

        State.CodeAssembler.Ret(fn);

        if (fn.RegisterArguments) {
            for (Int16 i = 1; i <= fn.RegisterArguments; i++)
                State.Registers[i].IsAllocated = false;
        }
    }
}

void EmitVar(EmitterState& State, AST::Var* Var) {
    auto& global = State.Globals.Get(State.Globals.Find(Var->Name)->second);

    if (Var->Value) {
        if (Var->Value->Type != AST::ExpresionType::Constant &&
            Var->Value->Type != AST::ExpresionType::ArrayList) {
            State.Error(Var->Location, u8"A global must to be initialized with a constant");
            return;
        }

        TmpValue tmp = EmitExpresion(State, Var->Value.get());
        if (tmp.IsErr()) {
            return;
        }
        else if (global.Type.IsUnknown()) {
            global.Type = tmp.Type;
        }
        else {
            State.TypeError(
                global.Type, tmp.Type,
                Var->Location,
                u8"You can't initialize a var of type '%s' with a value of type '%s'",
                global.Type.ToString().c_str(), tmp.Type.ToString().c_str()
            );
            return;
        }
        global.Value.Unsigned = tmp.Data;
    }
    else {
        global.Value.Unsigned = 0;
    }
}

void EmitConstVal(EmitterState& /*State*/, AST::ConstVal* /*ConstVal*/) {}

void EmitFunctionStatement(EmitterState& State, AST::Statement* Stat, Function& Fn) {
    if (Stat->Type == AST::StatementType::Return) {
        EmitFunctionReturn(State, (AST::Return*)Stat, Fn);
    }
    else if (Stat->Type == AST::StatementType::Var) {
        EmitFunctionLocal(State, (AST::Var*)Stat, Fn);
    }
    else if (Stat->Type == AST::StatementType::ConstVal) {
        EmitFunctionConstVal(State, (AST::ConstVal*)Stat, Fn);
    }
    else if (Stat->Type == AST::StatementType::If) {
        EmitFunctionIf(State, (AST::If*)Stat, Fn);
    }
    else if (Stat->Type == AST::StatementType::ExpresionStatement) {
        TmpValue tmp = EmitFunctionExpresion(State, ((AST::ExpresionStatement*)Stat)->Value.get(), Fn);
        if (tmp.IsRegister()) {
            State.DeallocateRegister(tmp.Reg);
        }
    }
}

void EmitFunctionReturn(EmitterState& State, AST::Return* Ret, Function& Fn) {
    if (Ret->Value) {
        State.Context.IsInReturn = true;
        TmpValue tmp = EmitFunctionExpresion(State, Ret->Value.get(), Fn);
        State.Context.IsInReturn = false;

        if (tmp.IsErr()) {
            return;
        }
        State.TypeError(
            Fn.Type, tmp.Type, Ret->Location,
            u8"Invalid conversion from '%s' to '%s'",
            tmp.Type.ToString().c_str(), Fn.Type.ToString().c_str()
        );

        if (tmp.IsRegister()) {
            if (tmp.Reg != 0) {
                State.CodeAssembler.Mov(Fn, 0, tmp.Reg);
            }
            State.DeallocateRegister(tmp.Reg);
        }
        else if (tmp.IsLocal()) {
            State.CodeAssembler.LocalGet(Fn, 0, tmp.Local);
        }
        else if (tmp.IsLocalReg()) {
            if (tmp.Reg != 0) {
                State.CodeAssembler.Mov(Fn, 0, tmp.Reg);
            }
            State.DeallocateRegister(tmp.Reg);
        }
        else if (tmp.IsGlobal()) {
                State.CodeAssembler.GlobalGet(Fn, 0, tmp.Global);
        }
        else if (tmp.IsConstant()) {
            State.MoveTmp(Fn, 0, tmp);
        }

        if (Fn.HasMultiReturn && (!State.Context.IsLast || (State.Context.IsInIf && !State.Context.IsInElse))) {
            State.CodeAssembler.Jmp(Fn, codefile::OpCode::Jmp, 0);
            auto& toResolve = Fn.ResolveReturns.emplace_back();
            toResolve.IP = UInt32(Fn.Code.Buff.size() - 2);
        }
    }
}

void EmitFunctionLocal(EmitterState& State, AST::Var* Var, Function& Fn) {
    {
        auto it = Fn.Locals.Find(Var->Name);
        if (it != Fn.Locals.end()) {
            State.Error(Var->Location, u8"'%s' is already defined", Var->Name.c_str());
            return;
        }
    }

    {
        auto it = State.Globals.Find(Var->Name);
        if (it != State.Globals.end()) {
            State.Error(Var->Location, u8"'%s' is shadowing a variable", Var->Name.c_str());
            return;
        }
    }

    auto& local = Fn.Locals.Add(Var->Name);
    local.Type = Var->VarType;

    if (State.CurrentOptions.OptimizationLevel == OPTIMIZATION_RELEASE_FAST) {
        for (auto& reg : State.Registers) {
            if ((reg.Index >= 1 
                && reg.Index <= 10)
                && !reg.IsAllocated) {
                reg.IsAllocated = true;
                local.Index = reg.Index;
                local.IsRegister = true;
                break;
            }
        }

        if (!local.IsRegister) {
            local.Index = Fn.CountOfStackLocals++;
            local.IsRegister = false;
        }
    }
    else {
        local.Index = Fn.CountOfStackLocals++;
        local.IsRegister = false;
    }

    if (!Var->Value) {
        if (Var->VarType.HasArray()) {
            if (Var->VarType.ArrayLen == 0) {
                State.Error(Var->Location, u8"A uninitialied array var must to have a size");
                return;
            }
            else {
                Byte size = State.AllocateRegister();
                State.MoveConst(Fn, size, Var->VarType.ArrayLen);
                State.CodeAssembler.ArrayNew(
                    Fn,
                    size,
                    State.TypeToArrayElement(AST::TypeDecl{ Var->VarType.Primitive, 0, 0, 0 })
                );
                if (!local.IsRegister) {
                    State.CodeAssembler.LocalSet(Fn, size, local.Index);
                }
                else {
                    State.CodeAssembler.Mov(Fn, local.Reg, size);
                }
             
                local.IsInitialized = true;
            }
        }
        else {
            local.IsInitialized = false;
        }
    }
    else {
        local.IsInitialized = true;

        TmpValue tmp = EmitFunctionExpresion(State, Var->Value.get(), Fn);
        if (tmp.IsErr()) {
            return;
        }

        if (local.Type.IsUnknown()) {
            local.Type = tmp.Type;
        }
        else {
            if (tmp.IsArrayExpr() && !Var->VarType.HasArray()) {
                State.Error(Var->Location, u8"Invalid initializer for '%s'", Var->VarType.ToString().c_str());
                return;
            }

                State.TypeError(
                Var->VarType, tmp.Type, Var->Location,
                u8"You can't initialize a var of type '%s' with a value of type '%s'",
                Var->VarType.ToString().c_str(), tmp.Type.ToString().c_str()
            );
        }

        if (tmp.IsRegister()) {
            if (local.IsRegister) {
                State.CodeAssembler.Mov(Fn, local.Index, tmp.Reg);
            }
            else {
                State.CodeAssembler.LocalSet(Fn, tmp.Reg, local.Index);
                State.DeallocateRegister(tmp.Reg);
            }
        }

        else if (tmp.IsArrayExpr()) {
            AST::ArrayList* arr = (AST::ArrayList*)tmp.Data;
            bool requiredType = false;

            if (Var->VarType.IsUnknown()) {
                requiredType = true;
            }
            else {
                local.Type = Var->VarType;
                UInt arrayLen = local.Type.ArrayLen;
                if (arrayLen == 0) {
                    local.Type.ArrayLen = UInt32(arr->Elements.size());
                }
                else if (arr->Elements.size() > arrayLen) {
                    State.Error(arr->Location, u8"Too much initializer values");
                    return;
                }
            }

            Byte dest = Byte(-1);
            if (!local.IsRegister) {
                dest = State.AllocateRegister();
            }
            else {
                dest = local.Reg; 
            }

            // Also used as register index
            Byte index = State.AllocateRegister();
            if (!requiredType) {
                State.MoveConst(Fn, dest, local.Type.ArrayLen);
                State.CodeAssembler.ArrayNew(Fn,
                                             dest,
                                             State.TypeToArrayElement(AST::TypeDecl{ local.Type.Primitive, 0, 0, 0 })
                );
            }

            UInt32 i = 0;
            // Used in case of non register based elements
            Byte src = State.AllocateRegister();
            AST::TypeDecl elementType = local.Type;
            elementType.Flags ^= AST::TypeDecl::Array;
            elementType.ArrayLen = 0;
            for (auto& element : arr->Elements) {
                TmpValue tmpE = EmitFunctionExpresion(State, element.get(), Fn);

                if (requiredType) {
                    elementType = tmpE.Type;
                    local.Type = tmpE.Type;
                    local.Type.Flags |= AST::TypeDecl::Array;
                    requiredType = false;

                    State.MoveConst(Fn, dest, arr->Elements.size());
                    State.CodeAssembler.ArrayNew(Fn,
                                                 dest,
                                                 State.TypeToArrayElement(AST::TypeDecl{ local.Type.Primitive, 0, 0, 0 })
                    );
                }
                else {
                    State.TypeError(
                        elementType, tmpE.Type, arr->Location,
                        u8"You can't initialize a var of type '%s' with a value of type '%s'",
                        elementType.ToString().c_str(), tmpE.Type.ToString().c_str()
                    );
                }

                State.MoveConst(Fn, index, i);

                if (tmpE.IsRegister() || tmpE.IsLocalReg()) {
                    State.CodeAssembler.ArrayStr(Fn, dest, index, tmpE.Reg);
                    
                    if (tmpE.IsRegister()) {
                        State.DeallocateRegister(tmpE.Reg);
                    }
                }
                else {
                    State.MoveTmp(Fn, src, tmpE);
                    State.CodeAssembler.ArrayStr(Fn, dest, index, src);
                }
                i++;
            }

            State.DeallocateRegister(src);
            State.DeallocateRegister(index);
            if (!local.IsRegister) {
                State.CodeAssembler.LocalSet(Fn, dest, local.Index);
                State.DeallocateRegister(dest);
            }
        }
        else {
            if (local.IsRegister) {
                State.MoveTmp(Fn, local.Index, tmp);
            }
            else {
                UInt8 reg = State.AllocateRegister();
                State.MoveTmp(Fn, reg, tmp);
                State.CodeAssembler.LocalSet(Fn, reg, local.Index);
                State.DeallocateRegister(reg);
            }
        }
    }
}

void EmitFunctionConstVal(EmitterState& /*State*/, AST::ConstVal* /*ConstVal*/, Function& /*Fn*/) {}

void EmitFunctionIf(EmitterState& State, AST::If* _If, Function& Fn) {
    State.Context.IsInIf = true;

    TmpValue result = EmitFunctionExpresion(State, _If->Expr.get(), Fn);
    if (result.IsErr()) {
        return;
    }

    codefile::OpCode opcode = codefile::OpCode::Jmp;
    switch (result.LastOp) {
    case AST::BinaryOperation::Comparision:
        opcode = codefile::OpCode::Jne;
        break;
    case AST::BinaryOperation::NotEqual:
        opcode = codefile::OpCode::Je;
        break;
    case AST::BinaryOperation::Less:
        opcode = codefile::OpCode::Jge;
        break;
    case AST::BinaryOperation::LessEqual:
        opcode = codefile::OpCode::Jg;
        break;
    case AST::BinaryOperation::Greater:
        opcode = codefile::OpCode::Jle;
        break;
    case AST::BinaryOperation::GreaterEqual:
        opcode = codefile::OpCode::Jle;
        break;
    }

    State.CodeAssembler.Jmp(Fn, opcode, 0);
    UInt32 toResolve = UInt16(Fn.Code.Buff.size() - 2);

    (void)EmitFunctionExpresion(State, _If->Body.get(), Fn);
    UInt16 address = UInt16(Fn.Code.Buff.size() - (toResolve+2));

    if (address <= Const16Max) {
        UInt16& jcc = *(UInt16*)&Fn.Code.Buff[toResolve];
        jcc = address;
    }
    else {
        State.Error(_If->Location, u8"Code too long");
        return;
    }
    if (_If->Elif) {
        EmitFunctionIf(State, (AST::If*)_If->Elif.get(), Fn);
    }
    else {
        if (_If->ElseBlock) {
            State.Context.IsInElse = true;
            (void)EmitFunctionExpresion(State, _If->ElseBlock.get(), Fn);
            State.Context.IsInElse = false;
        }
    }

    State.Context.IsInIf = false;
}

}
