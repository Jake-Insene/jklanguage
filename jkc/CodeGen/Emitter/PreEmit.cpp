#include "jkc/CodeGen/Emitter/Emitter.h"
#include "jkc/AST/Statements.h"
#include "jkc/AST/Expresions.h"

namespace CodeGen {

void Emitter::PreDeclareStatement(AST::Statement* Stat) {
    if (Stat->Type == AST::StatementType::Function) {
        PreDeclareFunction((AST::Function*)Stat);
    }
    else if (Stat->Type == AST::StatementType::Var) {
        PreDeclareVar((AST::Var*)Stat);
    }
    else if (Stat->Type == AST::StatementType::ConstVal) {
        PreDeclareConstVal((AST::ConstVal*)Stat);
    }
}

void Emitter::PreDeclareFunction(AST::Function* ASTFn) {
    bool isNative = false;
    for (auto& attr : ASTFn->Attribs) {
        if (attr.Type == Attribute::Native)
            isNative = true;
    }

    if (!ASTFn->IsDefined && !isNative)
        return;

    auto& fn = Functions.Emplace(ASTFn->Name);

    fn.Name = ASTFn->Name.data();
    fn.IsNative = isNative;
    fn.HasMultiReturn = ASTFn->HasMultiReturn;

    for (auto& attrib : ASTFn->Attribs) {
        if (attrib.Type == Attribute::Native) {
            if (attrib.S.empty()) {
                auto it = NativeLibraries.find(STR("jkl"));
                if (it != NativeLibraries.end()) {
                    fn.LibraryAddress = it->second;
                }
                else {
                    Strings.Push(STR("jkl"), 4);
                    fn.LibraryAddress = UInt32(Strings.Size - 1);
                    NativeLibraries.emplace(STR("jkl"), fn.LibraryAddress);
                }
            }
            else {
                auto it = NativeLibraries.find(attrib.S);
                if (it != NativeLibraries.end()) {
                    fn.LibraryAddress = it->second;
                }
                else {
                    Strings.Push(attrib.S.data(), attrib.S.size() + 1);
                    fn.LibraryAddress = UInt32(Strings.Size - 1);
                    NativeLibraries.emplace(STR("jkl"), fn.LibraryAddress);
                }
            }

            Strings.Push(fn.Name, ASTFn->Name.size() + 1);
            fn.EntryAddress = UInt32(Strings.Size - 1);
        }
    }

    if (isNative) {
        fn.CC = CallConv::Register;
        for (Byte i = 0; i < ASTFn->Parameters.Size; i++) {
            auto& local = fn.Locals.Emplace(ASTFn->Parameters[i].Name);
            local.Name = ASTFn->Parameters[i].Name.c_str();
            local.Type = ASTFn->Parameters[i].Type;
            local.IsInitialized = true;

            local.IsRegister = true;
            local.Reg = i + 1;
            fn.RegisterArguments++;
        }
    }
    else if (CurrentOptions.OptimizationLevel == OPTIMIZATION_NONE) {
        for (auto& param : ASTFn->Parameters) {
            auto& local = fn.Locals.Emplace(param.Name);
            local.Name = param.Name.c_str();
            local.Type = param.Type;
            local.IsInitialized = true;
            local.Index = fn.CountOfStackLocals++;
            fn.StackArguments++;
        }
    }
    else {
        if (ASTFn->Parameters.Size == 0)
            fn.CC = CallConv::Register;
        else if (ASTFn->Parameters.Size <= 10)
            fn.CC = CallConv::Register;
        else
            fn.CC = CallConv::RegS;

        for (Byte i = 0; i < ASTFn->Parameters.Size; i++) {
            auto& local = fn.Locals.Emplace(ASTFn->Parameters[i].Name);
            local.Name = ASTFn->Parameters[i].Name.c_str();
            local.Type = ASTFn->Parameters[i].Type;
            local.IsInitialized = true;

            if (i >= 10) {
                fn.StackArguments = Byte(ASTFn->Parameters.Size - 10);
                local.Index = fn.CountOfStackLocals++;
            }
            else {
                local.IsRegister = true;
                local.Reg = i + 1;
                fn.RegisterArguments++;
            }
        }
    }

    fn.CountOfArguments = Byte(ASTFn->Parameters.Size);
    fn.Address = (UInt32)Functions.Size() - 1;
    fn.Type = ASTFn->FunctionType;
    fn.IsDefined = ASTFn->IsDefined;
}

void Emitter::PreDeclareVar(AST::Var* Var) {
    auto& global = Globals.Emplace(Var->Name);
    global.Type = Var->VarType;
    global.Index = UInt32(Globals.Size() - 1);
}

void Emitter::PreDeclareConstVal(AST::ConstVal* /*ConstVal*/) {}

}
