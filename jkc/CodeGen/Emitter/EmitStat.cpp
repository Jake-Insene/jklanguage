#include "jkc/CodeGen/Emitter/Emitter.h"
#include "jkc/AST/Statements.h"
#include "jkc/AST/Expresions.h"

namespace CodeGen {

void Emitter::EmitStatement(AST::Statement* Stat) {
    if (Stat->Type == AST::StatementType::Function) {
        EmitFunction((AST::Function*)Stat);
    }
    else if (Stat->Type == AST::StatementType::Var) {
        EmitVar((AST::Var*)Stat);
    }
    else if (Stat->Type == AST::StatementType::ConstVal) {
        EmitConstVal((AST::ConstVal*)Stat);
    }
}

void Emitter::EmitFunction(AST::Function* ASTFn) {
    bool isNative = false;
    for (auto& attr : ASTFn->Attribs) {
        if (attr.Type == Attribute::Native)
            isNative = true;
    }

    if (!ASTFn->IsDefined && !isNative)
        return;

    auto& fn = Functions.Get(Functions.Find(ASTFn->Name)->second);

    if (ASTFn->IsDefined) {
        // Prologue
        if (fn.RegisterArguments) {
            for (Byte i = 1; i <= fn.RegisterArguments; i++)
                Registers[i].IsAllocated = true;
        }
        else {
            for (Int16 i = fn.StackArguments - 1; i >= 0; i--) {
                Local& local = fn.Locals.Data[i];
                if (!local.IsRegister) {
                    CodeAssembler.PopLocal(fn, local.Index);
                }
            }
        }

        {
            UInt32 i = 0;
            for (auto& stat : ASTFn->Body->Statements) {
                if ((i+1) == ASTFn->Body->Statements.Size) {
                    Context.IsLast = true;
                }
                EmitFunctionStatement(stat, fn);
                Context.IsLast = false;
                i++;
            }
        }

        for (auto& toResolve : fn.ResolveReturns) {
            UInt16 address = UInt16((fn.Code.Buff.Size) - toResolve.IP);
            if (address > ByteMax) {
                *((UInt16*)&fn.Code.Buff[toResolve.IP]) = address;
            }
            else {
                fn.Code.Buff[toResolve.IP - 1] = Byte(codefile::OpCode::Jmp8);
                fn.Code.Buff[toResolve.IP] = Byte(address - 1);
                mem::Copy(
                    (fn.Code.Buff.Data + (toResolve.IP + 1)),
                    (fn.Code.Buff.Data + (toResolve.IP + 2)),
                    (fn.Code.Buff.Size) - (toResolve.IP + 1)
                );
                fn.Code.Buff.Size -= 1;
            }
        }

        // Epilogue
        Byte deleter = AllocateRegister();
        if (deleter == 0) {
            deleter = AllocateRegister();
            DeallocateRegister(0);
        }

        for (auto& local : fn.Locals.Data) {
            if (!local.Type.HasConst() && local.Type.HasArray() && !local.Type.IsConstString()) {
                if (local.IsRegister) {
                    CodeAssembler.ArrayDestroy(fn, local.Reg);
                }
                else {
                    if (local.Index <= Const4Max) {
                        CodeAssembler.LocalGet4(fn, deleter, local.Index);
                    }
                    else {
                        CodeAssembler.LocalGet(fn, deleter, local.Index);
                    }
                    CodeAssembler.ArrayDestroy(fn, deleter);
                }
            }
        }
        DeallocateRegister(deleter);

        CodeAssembler.Ret(fn);

        if (fn.RegisterArguments) {
            for (Int16 i = 1; i <= fn.RegisterArguments; i++)
                Registers[i].IsAllocated = false;
        }
    }
}

void Emitter::EmitVar(AST::Var* Var) {
    auto& global = Globals.Get(Globals.Find(Var->Name)->second);

    if (Var->Value) {
        if (Var->Value->Type != AST::ExpresionType::Constant &&
            Var->Value->Type != AST::ExpresionType::ArrayList) {
            Error(Var->Location, STR("A global must to be initialized with a constant"));
        }

        TmpValue tmp = EmitExpresion(Var->Value);
        if (global.Type.IsUnknown()) {
            global.Type = tmp.Type;
        }
        else {
            TypeError(
                global.Type, tmp.Type,
                Var->Location,
                STR("You can't initialize a var of type '{s}' with a value of type '{s}'"),
                global.Type.ToString().c_str(), tmp.Type.ToString().c_str()
            );
        }
        global.Value.Unsigned = tmp.Data;
    }
    else {
        global.Value.Unsigned = 0;
    }
}

void Emitter::EmitConstVal(AST::ConstVal* /*ConstVal*/) {}

void Emitter::EmitFunctionStatement(AST::Statement* Stat, Function& Fn) {
    if (Stat->Type == AST::StatementType::Return) {
        EmitFunctionReturn((AST::Return*)Stat, Fn);
    }
    else if (Stat->Type == AST::StatementType::Var) {
        EmitFunctionLocal((AST::Var*)Stat, Fn);
    }
    else if (Stat->Type == AST::StatementType::ConstVal) {
        EmitFunctionConstVal((AST::ConstVal*)Stat, Fn);
    }
    else if (Stat->Type == AST::StatementType::If) {
        EmitFunctionIf((AST::If*)Stat, Fn);
    }
    else if (Stat->Type == AST::StatementType::ExpresionStatement) {
        (void)EmitFunctionExpresion(((AST::ExpresionStatement*)Stat)->Value, Fn);
    }
}

void Emitter::EmitFunctionReturn(AST::Return* Ret, Function& Fn) {
    if (Ret->Value) {
        Context.IsInReturn = true;
        TmpValue tmp = EmitFunctionExpresion(Ret->Value, Fn);
        Context.IsInReturn = false;

        if (tmp.IsErr()) return;
        TypeError(
            Fn.Type, tmp.Type, Ret->Location,
            STR("Invalid conversion from '{s}' to '{s}'"),
            tmp.Type.ToString().c_str(), Fn.Type.ToString().c_str()
        );

        if (tmp.IsRegister()) {
            if (tmp.Reg != 0) {
                CodeAssembler.Mov(Fn, 0, tmp.Reg);
            }
            DeallocateRegister(tmp.Reg);
        }
        else if (tmp.IsLocal()) {
            CodeAssembler.LocalGet(Fn, 0, tmp.Local);
        }
        else if (tmp.IsLocalReg()) {
            if (tmp.Reg != 0) {
                CodeAssembler.Mov(Fn, 0, tmp.Reg);
            }
            DeallocateRegister(tmp.Reg);
        }
        else if (tmp.IsGlobal()) {
                CodeAssembler.GlobalGet(Fn, 0, tmp.Global);
        }
        else if (tmp.IsConstant()) {
            MoveTmp(Fn, 0, tmp);
        }

        if (Fn.HasMultiReturn && (!Context.IsLast || !Context.IsInElse)) {
            CodeAssembler.Jmp(Fn, codefile::OpCode::Jmp, 0);
            auto& toResolve = Fn.ResolveReturns.Push();
            toResolve.IP = UInt32(Fn.Code.Buff.Size - 3);
        }
    }
}

void Emitter::EmitFunctionLocal(AST::Var* Var, Function& Fn) {
    {
        auto it = Fn.Locals.Find(Var->Name);
        if (it != Fn.Locals.end()) {
            Error(Var->Location, STR("'{s}' is already defined"), Var->Name.c_str());
            return;
        }
    }

    {
        auto it = Globals.Find(Var->Name);
        if (it != Globals.end()) {
            Error(Var->Location, STR("'{s}' is shadowing a variable"), Var->Name.c_str());
            return;
        }
    }

    auto& local = Fn.Locals.Emplace(Var->Name);
    local.Type = Var->VarType;

    if (CurrentOptions.OptimizationLevel == OPTIMIZATION_RELEASE_FAST) {
        for (auto& reg : Registers) {
            if ((reg.Index >= 1 && reg.Index <= 10)
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
        local.IsInitialized = false;
    }
    else {
        local.IsInitialized = true;

        TmpValue tmp = EmitFunctionExpresion(Var->Value, Fn);
        if (local.Type.IsUnknown()) {
            local.Type = tmp.Type;
        }
        else {
            TypeError(
                Var->VarType, tmp.Type, Var->Location,
                STR("You can't initialize a var of type '{s}' with a value of type '{s}'"),
                Var->VarType.ToString().c_str(), tmp.Type.ToString().c_str()
            );
        }

        if (tmp.IsRegister()) {
            if (local.IsRegister) {
                CodeAssembler.Mov(Fn, local.Index, tmp.Reg);
            }
            else {
                if (local.Index <= Const4Max) {
                    CodeAssembler.LocalSet4(Fn, tmp.Reg, local.Index);
                }
                else {
                    CodeAssembler.LocalSet(Fn, tmp.Reg, local.Index);
                }
                DeallocateRegister(tmp.Reg);
            }
        }
        else if (tmp.IsArrayExpr()) {
            AST::ArrayList* arr = Cast<AST::ArrayList*>(tmp.Data);
            bool requiredType = false;

            if (Var->VarType.IsUnknown()) {
                requiredType = true;
            }
            else {
                UInt arrayLen = Var->VarType.ArrayLen;
                if (arrayLen == 0) {
                    arrayLen = arr->Elements.Size;
                }
            }

            Byte dest = Byte(-1);
            if (!local.IsRegister) {
                dest = AllocateRegister();
            }
            else {
                dest = local.Reg; 
            }

            // Also used as register index
            Byte index = AllocateRegister();
            if (!requiredType) {
                CodeAssembler.Const32(Fn, dest, local.Type.ArrayLen);
                codefile::PrimitiveType t = TypeToPrimitive(local.Type);
                CodeAssembler.ArrayNew(Fn, dest, dest, t);
            }

            UInt32 i = 0;
            // Used in case of non register based elements
            Byte src = AllocateRegister();
            for (auto& element : arr->Elements) {
                TmpValue tmpE = EmitFunctionExpresion(element, Fn);

                if (requiredType) {
                    local.Type = tmpE.Type;
                    local.Type.Flags |= AST::TypeDecl::Array;
                    requiredType = false;

                    MoveConst(Fn, dest, arr->Elements.Size);
                    codefile::PrimitiveType t = TypeToPrimitive(local.Type);
                    CodeAssembler.ArrayNew(Fn, dest, dest, t);
                }
                else {
                    TypeError(
                        local.Type, tmpE.Type, arr->Location,
                        STR("You can't initialize a var of type '{s}' with a value of type '{s}'"),
                        Var->VarType.ToString().c_str(), tmpE.Type.ToString().c_str()
                    );
                }

                MoveConst(Fn, index, i);

                if (tmpE.IsRegister() || tmpE.IsLocalReg()) {
                    CodeAssembler.ArraySet(Fn, dest, index, tmpE.Reg);
                    
                    if (tmpE.IsRegister()) {
                        DeallocateRegister(tmpE.Reg);
                    }
                }
                else {
                    MoveTmp(Fn, src, tmpE);
                    CodeAssembler.ArraySet(Fn, dest, index, src);
                }
                i++;
            }

            DeallocateRegister(src);
            DeallocateRegister(index);
            if (!local.IsRegister) {
                if (local.Index <= Const4Max) {
                    CodeAssembler.LocalSet4(Fn, dest, local.Index);
                }
                else {
                    CodeAssembler.LocalSet(Fn, dest, local.Index);
                }
                DeallocateRegister(dest);
            }
        }
        else {
            if (local.IsRegister) {
                MoveTmp(Fn, local.Index, tmp);
            }
            else {
                UInt8 reg = AllocateRegister();
                MoveTmp(Fn, reg, tmp);
                if (local.Index <= Const4Max) {
                    CodeAssembler.LocalSet4(Fn, reg, local.Index);
                }
                else {
                    CodeAssembler.LocalSet(Fn, reg, local.Index);
                }
                DeallocateRegister(reg);
            }
        }
    }
}

void Emitter::EmitFunctionConstVal(AST::ConstVal* /*ConstVal*/, Function& /*Fn*/) {}

void Emitter::EmitFunctionIf(AST::If* _If, Function& Fn) {
    Context.IsInIf = true;

    TmpValue result = EmitFunctionExpresion(_If->Expr, Fn);

    switch (result.LastOp) {
    case AST::BinaryOperation::Comparision:
        CodeAssembler.Jmp(Fn, codefile::OpCode::Jne, 0);
        break;
    }
    USize toResolve = Fn.Code.Buff.Size - 2;

    (void)EmitFunctionExpresion(_If->Body, Fn);
    UInt16 relativeAddress = UInt16((Fn.Code.Buff.Size-1) - toResolve);
    if (_If->Elif) {
        if (relativeAddress > ByteMax) {
            *((UInt16*)&Fn.Code.Buff[toResolve]) = relativeAddress;
        }
        else {
            Fn.Code.Buff[toResolve - 1] = Byte(Fn.Code.Buff[toResolve - 1]) + 15;
            Fn.Code.Buff[toResolve] = Byte(relativeAddress);
            mem::Copy(
                (Fn.Code.Buff.Data + (toResolve + 1)),
                (Fn.Code.Buff.Data + (toResolve + 2)),
                (Fn.Code.Buff.Size) - (toResolve + 1)
            );
            Fn.Code.Buff.Size -= 1;
        }
        EmitFunctionIf((AST::If*)_If->Elif, Fn);
    }
    else {
        if (relativeAddress > ByteMax) {
            *((UInt16*)&Fn.Code.Buff[toResolve]) = relativeAddress;
        }
        else {
            Fn.Code.Buff[toResolve - 1] = Byte(Fn.Code.Buff[toResolve - 1]) + 15;
            Fn.Code.Buff[toResolve] = Byte(relativeAddress - 1);
            mem::Copy(
                (Fn.Code.Buff.Data + (toResolve + 1)),
                (Fn.Code.Buff.Data + (toResolve + 2)),
                (Fn.Code.Buff.Size) - (toResolve + 1)
            );
            Fn.Code.Buff.Size -= 1;
        }

        if (_If->ElseBlock) {
            Context.IsInElse = true;
            (void)EmitFunctionExpresion(_If->ElseBlock, Fn);
            Context.IsInElse = false;
        }
    }

    Context.IsInIf = false;
}

}
