#include "jkc/CodeGen/Emitter/EmitterState.h"
#include "jkc/CodeGen/Emitter/PreEmit.h"
#include "jkc/AST/Statements.h"
#include "jkc/AST/Expresions.h"

namespace CodeGen {

void PreEmit(EmitterState& State, AST::Program& Program) {
    for (auto& stat : Program.Statements) {
        PreDeclareStatement(State, stat.get());
    }
}

void PreDeclareStatement(EmitterState& State, AST::Statement* Stat) {
    if (Stat->Type == AST::StatementType::Function) {
        PreDeclareFunction(State, (AST::Function*)Stat);
    }
    else if (Stat->Type == AST::StatementType::Var) {
        PreDeclareVar(State, (AST::Var*)Stat);
    }
    else if (Stat->Type == AST::StatementType::ConstVal) {
        PreDeclareConstVal(State, (AST::ConstVal*)Stat);
    }
}

void PreDeclareFunction(EmitterState& State, AST::Function* ASTFn) {
    if (!ASTFn->IsDefined && !ASTFn->IsExtern)
        return;

    auto& fn = State.Functions.Add(ASTFn->Name);

    fn.Name = ASTFn->Name.data();
    fn.IsExtern = ASTFn->IsExtern;
    fn.HasMultiReturn = ASTFn->HasMultiReturn;

    if (ASTFn->IsExtern) {
        auto it = State.NativeLibraries.find(ASTFn->LibraryRef);
        if (it != State.NativeLibraries.end()) {
            fn.LibraryAddress = it->second;
        }
        else {
            StringTmp& str = State.Strings.emplace_back(ASTFn->LibraryRef.data(), ASTFn->LibraryRef.size());
            fn.LibraryAddress = UInt16(State.Strings.size() - 1);
            State.NativeLibraries.emplace(StringView(str.Data, str.Size), fn.LibraryAddress);
        }

        State.Strings.emplace_back(fn.Name, ASTFn->Name.size());
        fn.EntryAddress = UInt16(State.Strings.size() - 1);
    }

    if (ASTFn->IsExtern) {
        fn.CC = CallConv::Register;
        for (Byte i = 0; i < ASTFn->Parameters.size(); i++) {
            auto& local = fn.Locals.Add(ASTFn->Parameters[i].Name);
            local.Name = ASTFn->Parameters[i].Name.data();
            local.Type = ASTFn->Parameters[i].Type;
            local.IsInitialized = true;

            local.IsRegister = true;
            local.Reg = i + 1;
            fn.RegisterArguments++;
        }
    }
    else if (State.CurrentOptions.OptimizationLevel == OPTIMIZATION_NONE) {
        for (auto& param : ASTFn->Parameters) {
            auto& local = fn.Locals.Add(param.Name);
            local.Name = param.Name.data();
            local.Type = param.Type;
            local.IsInitialized = true;
            local.Index = fn.CountOfStackLocals++;
            fn.StackArguments++;
        }
    }
    else {
        if (ASTFn->Parameters.size() == 0)
            fn.CC = CallConv::Register;
        else if (ASTFn->Parameters.size() <= 10)
            fn.CC = CallConv::Register;
        else
            fn.CC = CallConv::RegS;

        for (Byte i = 0; i < ASTFn->Parameters.size(); i++) {
            auto& local = fn.Locals.Add(ASTFn->Parameters[i].Name);
            local.Name = ASTFn->Parameters[i].Name.data();
            local.Type = ASTFn->Parameters[i].Type;
            local.IsInitialized = true;

            if (i >= 10) {
                fn.StackArguments = Byte(ASTFn->Parameters.size() - 10);
                local.Index = fn.CountOfStackLocals++;
            }
            else {
                local.IsRegister = true;
                local.Reg = i + 1;
                fn.RegisterArguments++;
            }
        }
    }

    fn.CountOfArguments = Byte(ASTFn->Parameters.size());
    fn.Address = (UInt32)State.Functions.Size() - 1;
    fn.Type = ASTFn->FunctionType;
    fn.IsDefined = ASTFn->IsDefined;
}

void PreDeclareVar(EmitterState& State, AST::Var* Var) {
    auto& global = State.Globals.Add(Var->Name);
    global.Type = Var->VarType;
    global.Index = UInt32(State.Globals.Size() - 1);
    global.Name = Var->Name.data();
}

void PreDeclareConstVal(EmitterState& /*State*/, AST::ConstVal* /*ConstVal*/) {}

}
