#include "jkc/CodeGen/Emitter.h"
#include "jkc/AST/Statements.h"
#include "jkc/AST/Expresions.h"
#include <jkr/CodeFile/Header.h>
#include <jkr/CodeFile/Function.h>
#include <jkr/CodeFile/Global.h>

namespace CodeGen {

void Emitter::Emit(AST::Program& Program, FileType FileTy, EmitOptions Options, StreamOutput& Output) {
    CurrentOptions = Options;
    Functions.Clear();
    Globals.Clear();
    Context = {};
    
    for (auto& statement : Program.Statements) {
        PreDeclareStatement(statement);
    }

    for (auto& stat : Program.Statements) {
        EmitStatement(stat);
    }

    codefile::FileHeader header = {
            .Signature = {},
            .CheckSize = sizeof(codefile::FileHeader),
    };

    mem::Copy(Cast<Char*>(&header.Signature), codefile::Signature, sizeof(codefile::Signature));
    header.NumberOfFunctions = (codefile::FunctionType)Functions.Size();
    header.NumberOfGlobals = (codefile::GlobalType)Globals.Size();
    header.NumberOfStrings = (codefile::StringType)Strings.Size;

    if (FileTy == FileType::Executable) {
        auto it = Functions.Find(STR("Main"));
        if (it == Functions.end()) {
            GlobalError(STR("The entry point was not defined"));
            return;
        }
        
        header.FileType = codefile::Executable;
        header.EntryPoint = (codefile::FunctionType)it->second;
        header.MajorVersion = 1;
        header.MinorVersion = 0;
    }

    // Calculate CheckSize
    for (auto& fn : Functions.Data) {
        header.CheckSize += UInt32(sizeof(codefile::FunctionHeader) + fn.Code.Buff.Size);
    }

    for (auto& global : Globals.Data) {
        header.CheckSize += UInt32(sizeof(codefile::GlobalHeader));
        
        header.CheckSize += UInt32(global.Type.SizeInBits / 8);
    }

    for (auto& str : Strings) {
        header.CheckSize += UInt32(str.Size + 2);
    }

    // Writing
    Output.Write((Byte*)&header, sizeof(codefile::FileHeader));
    for (auto& fn : Functions.Data) {
        codefile::FunctionHeader fnHeader = {
                .Attributes = 0,
                .SizeOfCode = (UInt16)fn.Code.Buff.Size,
        };

        if (fn.IsNative) {
            fnHeader.Attributes |= codefile::FunctionNative;
            fnHeader.LIndex = fn.LibraryIndex;
            fnHeader.EIndex = fn.EntryIndex;
            Output.Write(Cast<Byte*>(&fnHeader), sizeof(codefile::FunctionHeader));
        }
        else {
            fnHeader.NonNative.Arguments = fn.StackArguments;
            fnHeader.NonNative.LocalCount = fn.CountOfStackLocals;

            Output.Write(Cast<Byte*>(&fnHeader), sizeof(codefile::FunctionHeader));
            Output.Write(Cast<Byte*>(fn.Code.Buff.Data), fn.Code.Buff.Size);
        }
    }

    for (auto& global : Globals.Data) {
        codefile::GlobalHeader gHeader = {
            .Primitive = TypeToPrimitive(global.Type),
            .Attributes = TypeToAttributes(global.Type),
        };

        Output.Write(Cast<Byte*>(&gHeader), sizeof(codefile::GlobalHeader));
        Output.Write(Cast<Byte*>(&global.Value.Unsigned), global.Type.SizeInBits / 8);
    }

    for (auto& s : Strings) {
        Output.Write(Cast<Byte*>(&s.Size), 2);
        Output.Write((Byte*)s.Data, s.Size);
    }

    // End
    for (auto& fn : Functions.Data) {
        fn.Code.Destroy();
    }
}

void Emitter::Warn(const SourceLocation& Location, Str Format, ...) {
    ErrorStream.Print(STR("{s}:{u}: Warn: "), Location.FileName, Location.Line);

    va_list args;
    va_start(args, Format);
    ErrorStream.PrintlnVa(Format, args);
    va_end(args);
}

void Emitter::Error(const SourceLocation& Location, Str Format, ...) {
    ErrorStream.Print(STR("{s}:{u}: Error: "), Location.FileName, Location.Line);
    va_list args;
    va_start(args, Format);
    ErrorStream.PrintlnVa(Format, args);
    va_end(args);
    Success = false;
}

void Emitter::TypeError(const AST::TypeDecl& LHS, const AST::TypeDecl& RHS, 
                        const SourceLocation& Location, Str Format, ...) {
    if (LHS != RHS) {
        ErrorStream.Print(STR("{s}:{u}: Error: "), Location.FileName, Location.Line);
        va_list args;
        va_start(args, Format);
        ErrorStream.PrintlnVa(Format, args);
        va_end(args);
        Success = false;
    }
}

void Emitter::GlobalError(Str Format, ...) {
    ErrorStream.Print(STR("Error: "));

    va_list args;
    va_start(args, Format);
    ErrorStream.PrintlnVa(Format, args);
    va_end(args);
    Success = false;
}

void Emitter::PreDeclareStatement(mem::Ptr<AST::Statement>& Stat) {
    if (Stat->Type == AST::StatementType::Function) {
        PreDeclareFunction((AST::Function*)Stat.Data);
    }
    else if (Stat->Type == AST::StatementType::Var) {
        PreDeclareVar((AST::Var*)Stat.Data);
    }
    else if (Stat->Type == AST::StatementType::ConstVal) {
        PreDeclareConstVal((AST::ConstVal*)Stat.Data);
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
    for (auto& attrib : ASTFn->Attribs) {
        if (attrib.Type == Attribute::Native) {
            if (attrib.S.empty()) {
                auto it = NativeLibraries.find(STR("jkl"));
                if (it != NativeLibraries.end()) {
                    fn.LibraryIndex = it->second;
                }
                else {
                    Strings.Push(STR("jkl"), 4);
                    fn.LibraryIndex = (codefile::StringType)(Strings.Size - 1);
                    NativeLibraries.emplace(STR("jkl"), fn.LibraryIndex);
                }
            }
            else {
                auto it = NativeLibraries.find(attrib.S);
                if (it != NativeLibraries.end()) {
                    fn.LibraryIndex = it->second;
                }
                else {
                    Strings.Push(attrib.S.data(), attrib.S.size() + 1);
                    fn.LibraryIndex = (codefile::StringType)(Strings.Size - 1);
                    NativeLibraries.emplace(STR("jkl"), fn.LibraryIndex);
                }
            }

            Strings.Push(fn.Name, ASTFn->Name.size() + 1);
            fn.EntryIndex = (codefile::StringType)(Strings.Size - 1);
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
            local.Index = (codefile::LocalType)fn.Locals.Size() - 1;
            fn.StackArguments++;
        }
    }
    else {
        if (ASTFn->Parameters.Size == 0)
            fn.CC = CallConv::Register;
        else if(ASTFn->Parameters.Size <= 10)
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
                local.Reg = i+1;
                fn.RegisterArguments++;
            }
        }
    }

    fn.CountOfArguments = Byte(ASTFn->Parameters.Size);
    fn.Index = (codefile::FunctionType)Functions.Size() - 1;
    fn.Type = ASTFn->FunctionType;
    fn.IsDefined = ASTFn->IsDefined;
}

void Emitter::PreDeclareVar(AST::Var* Var) {
    auto& global = Globals.Emplace(Var->Name);
    global.Type = Var->VarType;
    global.Index = codefile::GlobalType(Globals.Size() - 1);
}

void Emitter::PreDeclareConstVal(AST::ConstVal* /*ConstVal*/) {}


TmpValue Emitter::EmitExpresion(mem::Ptr<AST::Expresion>& Expr) {
    if (Expr->Type == AST::ExpresionType::Constant) {
        auto constant = (AST::Constant*)Expr.Data;
        return TmpValue{
            .Ty = TmpType::Constant,
            .Data = constant->Unsigned,
            .Type = constant->ValueType,
        };
    }

    return TmpValue();
}

void Emitter::EmitStatement(mem::Ptr<AST::Statement>& Stat) {
    if (Stat->Type == AST::StatementType::Function) {
        EmitFunction((AST::Function*)Stat.Data);
    }
    else if (Stat->Type == AST::StatementType::Var) {
        EmitVar((AST::Var*)Stat.Data);
    }
    else if (Stat->Type == AST::StatementType::ConstVal) {
        EmitConstVal((AST::ConstVal*)Stat.Data);
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
        if (fn.RegisterArguments) {
            for (Int16 i = 1; i <= fn.RegisterArguments; i++)
                Registers[i].IsAllocated = true;
        }

        for (auto& stat : ASTFn->Body->Statements) {
            EmitFunctionStatement(stat, fn);
        }

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

void Emitter::EmitFunctionStatement(mem::Ptr<AST::Statement>& Stat, Function& Fn) {
    if (Stat->Type == AST::StatementType::Return) {
        EmitFunctionReturn((AST::Return*)Stat.Data, Fn);
    }
    else if (Stat->Type == AST::StatementType::Var) {
        EmitFunctionLocal((AST::Var*)Stat.Data, Fn);
    }
    else if (Stat->Type == AST::StatementType::ConstVal) {
        EmitFunctionConstVal((AST::ConstVal*)Stat.Data, Fn);
    }
    else if (Stat->Type == AST::StatementType::If) {
        EmitFunctionIf((AST::If*)Stat.Data, Fn);
    }
    else if (Stat->Type == AST::StatementType::ExpresionStatement) {
        (void)EmitFunctionExpresion(((AST::ExpresionStatement*)Stat.Data)->Value, Fn);
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
            if (Fn.IsRegisterBased()) {
                if (tmp.Reg != 0) {
                    CodeAssembler.Mov(Fn, 0, tmp.Reg);
                }
                CodeAssembler.RetVoid(Fn);
            }
            else {
                if(tmp.Reg == 0)
                    CodeAssembler.RetVoid(Fn);
                else
                    CodeAssembler.Ret(Fn, tmp.Reg);
            }
            DeallocateRegister(tmp.Reg);
        }
        else if (tmp.IsLocal()) {
            if (Fn.IsRegisterBased()) {
                CodeAssembler.LocalGet(Fn, 0, tmp.Local);
                CodeAssembler.RetVoid(Fn);
            }
            else {
                CodeAssembler.RetLocal(Fn, tmp.Local);
            }
        }
        else if (tmp.IsLocalReg()) {
            if (Fn.IsRegisterBased()) {
                if (tmp.Reg != 0) {
                    CodeAssembler.Mov(Fn, 0, tmp.Reg);
                    CodeAssembler.RetVoid(Fn);
                }
            }
            else {
                if (tmp.Reg == 0)
                    CodeAssembler.RetVoid(Fn);
                else
                    CodeAssembler.Ret(Fn, tmp.Reg);
            }
            DeallocateRegister(tmp.Reg);
        }
        else if (tmp.IsGlobal()) {
            if (Fn.IsRegisterBased()) {
                CodeAssembler.GlobalGet(Fn, 0, tmp.Global);
                CodeAssembler.RetVoid(Fn);
            }
            else
                CodeAssembler.RetGlobal(Fn, tmp.Global);
        }
        else if (tmp.IsConstant()) {
            if (Fn.IsRegisterBased()) {
                MoveTmp(Fn, 0, tmp);
                CodeAssembler.RetVoid(Fn);
            }
            else if (tmp.Type.SizeInBits > 16) {
                UInt8 reg = AllocateRegister();
                MoveTmp(Fn, reg, tmp);
                CodeAssembler.Ret(Fn, reg);
                DeallocateRegister(reg);
            }
            else {
                if (tmp.Type.SizeInBits <= 8) {
                    CodeAssembler.Ret8(Fn, tmp.Reg);
                }
                else if (tmp.Type.SizeInBits <= 16) {
                    CodeAssembler.Ret16(Fn, (UInt16)tmp.Data);
                }
            }
        }

    }
    else {
        CodeAssembler.RetVoid(Fn);
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
            CodeAssembler.LocalSet(Fn, tmp.Reg, local.Index);
            DeallocateRegister(tmp.Reg);
        }
        else {
            UInt8 reg = AllocateRegister();
            MoveTmp(Fn, reg, tmp);
            CodeAssembler.LocalSet(Fn, reg, local.Index);
            DeallocateRegister(reg);
        }
        local.Index = Fn.CountOfStackLocals++;
    }
}

void Emitter::EmitFunctionConstVal(AST::ConstVal* /*ConstVal*/, Function& /*Fn*/) {
}

void Emitter::EmitFunctionIf(AST::If* _If, Function& Fn) {
    TmpValue result = EmitFunctionExpresion(_If->Expr, Fn);

    switch (result.LastOp) {
    case AST::BinaryOperation::Comparision:
        CodeAssembler.Jmp(Fn, codefile::OpCode::Jne, 0);
        break;
    }
    USize toResolve = Fn.Code.Buff.Size - sizeof(codefile::AddressType);

    (void)EmitFunctionExpresion(_If->Body, Fn);
    if (_If->Elif) {
        codefile::AddressType address = (codefile::AddressType)Fn.Code.Buff.Size;
        if (address > ByteMax) {
            *((codefile::AddressType*)&Fn.Code.Buff[toResolve]) = address;
        }
        else {
            Fn.Code.Buff[toResolve - 1] = Byte(Fn.Code.Buff[toResolve - 1]) + 9;
            Fn.Code.Buff[toResolve] = Byte(address);
            mem::Copy(
                (Fn.Code.Buff.Data + (toResolve + 1)),
                (Fn.Code.Buff.Data + (toResolve + sizeof(codefile::AddressType))),
                (Fn.Code.Buff.Size - 1) - (toResolve + 1)
            );
        }
        EmitFunctionIf((AST::If*)_If->Elif.Data, Fn);
    }
    else if (_If->ElseBlock) {
        (void)EmitFunctionExpresion(_If->ElseBlock, Fn);
    }
    else {
        codefile::AddressType address = (codefile::AddressType)Fn.Code.Buff.Size;
        if (address > ByteMax) {
            *((codefile::AddressType*)&Fn.Code.Buff[toResolve]) = address;
        }
        else {
            Fn.Code.Buff[toResolve - 1] = Byte(Fn.Code.Buff[toResolve - 1]) + 9;
            Fn.Code.Buff[toResolve] = Byte(address-1);
            mem::Copy(
                (Fn.Code.Buff.Data + (toResolve + 1)),
                (Fn.Code.Buff.Data + (toResolve + sizeof(codefile::AddressType))),
                (Fn.Code.Buff.Size) - (toResolve+1)
            );
            Fn.Code.Buff.Size -= 1;
        }
    }

}

TmpValue Emitter::GetID(const std::u8string& ID, Function& Fn, const SourceLocation& Location) {
    {
        auto it = Fn.Locals.Find(ID);
        if (it != Fn.Locals.end()) {
            auto& local = Fn.Locals.Get(it->second);
            if (!local.IsInitialized) {
                Error(Location, STR("Trying to use a uninitialized var"));
            }
            return TmpValue{
                .Ty = local.IsRegister ? TmpType::LocalReg : TmpType::Local,
                .Data = local.Index,
                .Type = local.Type,
            };
        }
    }

    {
        auto it = Globals.Find(ID);
        if (it == Globals.end()) {
            Error(Location,
                STR("Undefined reference to '{s}'"),
                ID.c_str()
            );
        }
        else {
            auto& global = Globals.Get(it->second);
            return TmpValue{
                .Ty = TmpType::Global,
                .Data = global.Index,
                .Type = global.Type,
            };
        }
    }

    return TmpValue();
}

Function* Emitter::GetFn(mem::Ptr<AST::Expresion>& Target) {
    if (Target->Type == AST::ExpresionType::Identifier) {
        auto id = (AST::Identifier*)Target.Data;
        auto it = Functions.Find(id->ID);
        if (it != Functions.end() && (Functions.Get(it->second).IsDefined || Functions.Get(it->second).IsNative)) {
            return &Functions.Get(it->second);
        }

        Error(id->Location,
            STR("Undefined reference to function '{s}'"),
            id->ID.c_str()
        );
    }
    return nullptr;
}

TmpValue Emitter::EmitFunctionExpresion(mem::Ptr<AST::Expresion>& Expr, Function& Fn) {
    if (Expr->Type == AST::ExpresionType::Constant) {
        return EmitFunctionConstant((AST::Constant*)Expr.Data, Fn);
    }
    else if(Expr->Type == AST::ExpresionType::Identifier) {
        return EmitFunctionIdentifier((AST::Identifier*)Expr.Data, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::Group) {
        return EmitFunctionExpresion(((AST::Group*)Expr.Data)->Value, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::Call) {
        return EmitFunctionCall((AST::Call*)Expr.Data, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::BinaryOp) {
        return EmitFunctionBinaryOp((AST::BinaryOp*)Expr.Data, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::Unary) {
        return EmitFunctionUnary((AST::Unary*)Expr.Data, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::Dot) {
        return EmitFunctionDot((AST::Dot*)Expr.Data, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::ArrayList) {
        return EmitFunctionArrayList((AST::ArrayList*)Expr.Data, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::Block) {
        return EmitFunctionBlock((AST::Block*)Expr.Data, Fn);
    }

    assert(0 && "Invalid expresion");
    return TmpValue();
}

TmpValue Emitter::EmitFunctionConstant(AST::Constant* Constant, Function& /*Fn*/) {
    TmpValue tmp = {};
    tmp.Ty = TmpType::Constant;
    tmp.Type = Constant->ValueType;
    tmp.Data = Constant->Unsigned;

    if (Constant->ValueType.IsUInt() || Constant->ValueType.IsInt()) {
        if (Constant->Unsigned <= Const4Max) {
            tmp.Type.SizeInBits = 4;
        }
        else if (Constant->Unsigned <= UINT8_MAX) {
            tmp.Type.SizeInBits = 8;
        }
        else if (Constant->Unsigned <= UINT16_MAX) {
            tmp.Type.SizeInBits = 16;
        }
        else if (Constant->Unsigned <= UINT32_MAX) {
            tmp.Type.SizeInBits = 32;
        }
    }
    else if (Constant->ValueType.IsConstString()) {
        Strings.Push(Constant->String.c_str(), Constant->String.size() + 1);
        tmp.Data = Strings.Size - 1;
    }

    return tmp;
}

TmpValue Emitter::EmitFunctionIdentifier(AST::Identifier* ID, Function& Fn) {
    return GetID(ID->ID, Fn, ID->Location);
}

TmpValue Emitter::EmitFunctionCall(AST::Call* Call, Function& Fn) {
    TmpValue result = {};
    Function* target = GetFn(Call->Target);
    if (target == nullptr) return result;
    
    List<Byte> usedRegisters;
    for (auto& reg : Registers) {
        if (reg.IsAllocated) {
            usedRegisters.Push(reg.Index);
            CodeAssembler.Push(Fn, reg.Index);
            DeallocateRegister(reg.Index);
        }
    }

    if (target->CountOfArguments != Call->Arguments.Size) {
        Error(Call->Location,
            STR("{u} arguments was expected but {u} was founded"),
            (UInt)target->CountOfArguments,
            Call->Arguments.Size
        );
    }

    Context.IsInCall = true;
    for (Int8 i = target->CountOfArguments; i > 0; i--) {
        TmpValue arg = EmitFunctionExpresion(
            Call->Arguments[static_cast<USize>(i) - 1], Fn
        );
        if (target->IsRegisterBased()) {
            if (i <= target->RegisterArguments) {
                if (arg.Ty == TmpType::Register) {
                    if (arg.Reg != i) {
                        CodeAssembler.Mov(Fn, Byte(i), arg.Reg);
                    }
                }
                else {
                    MoveTmp(Fn, Byte(i), arg);
                }
            }
        }
        else {
            PushTmp(Fn, arg);
        }

        auto& local = target->Locals.Get(
            static_cast<USize>(i) - 1
        );
        TypeError(
            local.Type, arg.Type, Call->Location, 
            STR("Invalid argument at {d}: using a value of type '{s}' with a argument of type '{s}'"), 
            i, arg.Type.ToString().c_str(), local.Type.ToString().c_str()
        );

    }
    Context.IsInCall = false;

    if (target->Index <= ByteMax) {
        CodeAssembler.Call8(Fn, (Byte)target->Index);
    }
    else {
        CodeAssembler.Call(Fn, target->Index);
    }

    if (usedRegisters.Size) {
        auto end = usedRegisters.end();
        while (--end != usedRegisters.begin() - 1) {
            CodeAssembler.Pop(Fn, *end);
            Registers[*end].IsAllocated = true;
        }
    }

    result.Ty = TmpType::Register;
    result.Type = target->Type;
    if (target->CC == CallConv::Stack) {
        result.Reg = AllocateRegister();
        CodeAssembler.MovRes(Fn, result.Reg);
    }
    else if (!target->Type.IsVoid()){
        Registers[0].IsAllocated = true;
        result.Reg = 0;
    }

    usedRegisters.Destroy();
    return result;
}

TmpValue Emitter::EmitFunctionBinaryOp(AST::BinaryOp* BinOp, Function& Fn) {
    TmpValue left = EmitFunctionExpresion(BinOp->Left, Fn);
    TmpValue right = EmitFunctionExpresion(BinOp->Right, Fn);
    TmpValue result = {};
    TypeError(
        left.Type, right.Type, BinOp->Location,
        STR("Invalid operands type '{s}' and '{s}'"),
        left.Type.ToString().c_str(), right.Type.ToString().c_str()
    );

    switch (BinOp->Op) {
    case AST::BinaryOperation::Add:
    case AST::BinaryOperation::AddEqual:
    case AST::BinaryOperation::Sub:
    case AST::BinaryOperation::SubEqual:
    case AST::BinaryOperation::Mul:
    case AST::BinaryOperation::MulEqual:
    case AST::BinaryOperation::Div:
    case AST::BinaryOperation::DivEqual:
        result = EmitBinaryOp(left, right, BinOp->Op, Fn);
        break;
    case AST::BinaryOperation::Comparision:
    case AST::BinaryOperation::NotEqual:
    case AST::BinaryOperation::Less:
    case AST::BinaryOperation::LessEqual:
    case AST::BinaryOperation::Greater:
    case AST::BinaryOperation::GreaterEqual:
    {
        UInt8 rightReg = UInt8(-1);
        if (!right.IsRegister()) {
            rightReg = AllocateRegister();
            MoveTmp(Fn, rightReg, right);
        }
        else {
            rightReg = (UInt8)right.Data;
        }

        if (left.IsRegister()) {
            if (left.Type.IsInt()) {
                CodeAssembler.ICmp(Fn, left.Reg, rightReg);
            }
            else if (left.Type.IsUInt()) {
                CodeAssembler.Cmp(Fn, left.Reg, rightReg);
            }
            else if (left.Type.IsFloat()) {
                CodeAssembler.FCmp(Fn, left.Reg, rightReg);
            }
        }
        else if (left.IsLocal()) {
            UInt8 tmp = AllocateRegister();
            MoveTmp(Fn, tmp, left);
            DeallocateRegister(tmp);

            if (left.Type.IsInt()) {
                CodeAssembler.ICmp(Fn, tmp, rightReg);
            }
            else if (left.Type.IsUInt()) {
                CodeAssembler.Cmp(Fn, tmp, rightReg);
            }
            else if (left.Type.IsFloat()) {
                CodeAssembler.FCmp(Fn, tmp, rightReg);
            }
        }
        else if (left.IsLocalReg()) {
            if (left.Type.IsInt()) {
                CodeAssembler.ICmp(Fn, left.Reg, rightReg);
            }
            else if (left.Type.IsUInt()) {
                CodeAssembler.Cmp(Fn, left.Reg, rightReg);
            }
            else if (left.Type.IsFloat()) {
                CodeAssembler.FCmp(Fn, left.Reg, rightReg);
            }
        }

        if (rightReg != UInt8(-1)) {
            DeallocateRegister(rightReg);
        }
    }
        break;
    default:
        assert(0 && "Invalid binary operation");
        break;
    }

    result.Type = left.Type;
    result.LastOp = BinOp->Op;
    return result;
}

#define MAKE_OP_CONST(Type, Function, ...) \
    if(Type.IsInt()) {\
        CodeAssembler.I##Function(Fn, __VA_ARGS__);\
    }\
    else if(Type.IsUInt()) {\
        CodeAssembler.##Function(Fn, __VA_ARGS__);\
    }\

#define MAKE_OP(Type, Function, ...) \
    if(Type.IsInt()) {\
        CodeAssembler.I##Function(Fn, __VA_ARGS__);\
    }\
    else if(Type.IsUInt()) {\
        CodeAssembler.##Function(Fn, __VA_ARGS__);\
    }\
    else if (Type.IsFloat()) {\
        CodeAssembler.F##Function(Fn, __VA_ARGS__);\
    }

#define MAKE_CASE(Case) \
    if(Op == AST::BinaryOperation::Case){\
        if(Right.IsRegister()) {\
            MAKE_OP(Right.Type, Case, result.Reg, leftReg, Right.Reg);\
            DeallocateRegister(Right.Reg);\
        }\
        else if (Right.IsConstant()) {\
            if(Right.Data == 1 && \
            (AST::BinaryOperation::Case == AST::BinaryOperation::Sub || \
             AST::BinaryOperation::Case == AST::BinaryOperation::SubEqual)) {\
                MAKE_OP(Right.Type, Dec, leftReg);\
            }\
            else if(Right.Data == 1 && \
            (AST::BinaryOperation::Case == AST::BinaryOperation::Add || \
             AST::BinaryOperation::Case == AST::BinaryOperation::AddEqual)) {\
                MAKE_OP(Right.Type, Inc, leftReg);\
            }\
            else if (Right.Type.SizeInBits <= 8) {\
                MAKE_OP_CONST(Right.Type, Case##8, result.Reg, leftReg, Right.Reg);\
            }\
            else if (Right.Type.SizeInBits <= 16) {\
                MAKE_OP_CONST(Right.Type, Case##16, result.Reg, leftReg, (UInt16)Right.Data);\
            }\
            else {\
            Byte tmp = AllocateRegister();\
            MoveTmp(Fn, tmp, Right);\
            MAKE_OP(Right.Type, Case, result.Reg, leftReg, tmp);\
            DeallocateRegister(tmp);\
            }\
        }\
        else if (Right.IsLocal()) {\
            Byte tmp = AllocateRegister();\
            MoveTmp(Fn, tmp, Right);\
            MAKE_OP(Right.Type, Case, result.Reg, leftReg, tmp);\
            DeallocateRegister(tmp);\
        }\
        else if (Right.IsLocalReg()) {\
            MAKE_OP(Right.Type, Case, result.Reg, leftReg, Right.Reg);\
        }\
    }

TmpValue Emitter::EmitBinaryOp(TmpValue& Left, TmpValue& Right, AST::BinaryOperation Op, Function& Fn) {
    TmpValue result = {};
    result.Reg = Byte(-1);
    UInt asInt = UInt(Op);

    if (asInt >= 8 && asInt <= 11) {
    }
    else {
        UInt8 leftReg = Byte(-1);
        result.Ty = TmpType::Register;
        if (Context.IsInReturn && !Context.IsInCall && (Fn.IsRegisterBased())) {
            if (!Registers[0].IsAllocated) {
                result.Reg = 0;
                Registers[0].IsAllocated = true;
            }
            else {
                if (Left.Reg == 0 || Right.Reg == 0)
                    result.Reg = 0;
            }
        }
        
        if(!Left.IsLocalReg())
        {
            leftReg = AllocateRegister();
            MoveTmp(Fn, leftReg, Left);
            if (result.Reg == Byte(-1))
                result.Reg = leftReg;
        }
        else {
            leftReg = Left.Reg;
            if(result.Reg == Byte(-1))
               result.Reg = Left.Reg;
        }

        MAKE_CASE(Add);
        MAKE_CASE(Sub);
        MAKE_CASE(Mul);
        MAKE_CASE(Div);
    }

    return result;
}

TmpValue Emitter::EmitFunctionUnary(AST::Unary* /*Unary*/, Function& /*Fn*/) {
    return TmpValue();
}

TmpValue Emitter::EmitFunctionDot(AST::Dot* /*Dot*/, Function& /*Fn*/) {
    return TmpValue();
}

TmpValue Emitter::EmitFunctionArrayList(AST::ArrayList* /*ArrayList*/, Function& /*Fn*/) {
    return TmpValue();
}

TmpValue Emitter::EmitFunctionBlock(AST::Block* Block, Function& Fn) {
    for (auto& stat : Block->Statements) {
        EmitFunctionStatement(stat, Fn);
    }

    return TmpValue();
}

void Emitter::PushTmp(Function& Fn, const TmpValue& Tmp) {
    if (Tmp.IsRegister()) {
        CodeAssembler.Push(Fn, Tmp.Reg);
        DeallocateRegister(Tmp.Reg);
    }
    else if (Tmp.IsLocal()) {
        CodeAssembler.LocalPush(Fn, Tmp.Local);
    }
    else if (Tmp.IsLocalReg()) {
        CodeAssembler.Push(Fn, Tmp.Reg);
    }
    else if (Tmp.IsConstant()) {
        if (Tmp.Type.SizeInBits <= 8)
            CodeAssembler.Push8(Fn, (UInt8)Tmp.Data);
        else if (Tmp.Type.SizeInBits == 16)
            CodeAssembler.Push16(Fn, (UInt16)Tmp.Data);
        else if (Tmp.Type.SizeInBits == 32)
            CodeAssembler.Push32(Fn, (UInt32)Tmp.Data);
        else if (Tmp.Type.SizeInBits == 64)
            CodeAssembler.Push64(Fn, Tmp.Data);
    }
}

void Emitter::MoveTmp(Function& Fn, UInt8 Reg, const TmpValue& Tmp) {
    if (Tmp.IsRegister()) {
        CodeAssembler.Mov(Fn, Reg, Tmp.Reg);
    }
    else if (Tmp.IsLocal()) {
        if (Tmp.Local <= 0xF) {
            CodeAssembler.LocalGet4(Fn, Reg, Tmp.Reg);
        }
        else
            CodeAssembler.LocalGet(Fn, Reg, Tmp.Local);
    }
    else if (Tmp.IsLocalReg()) {
        CodeAssembler.Mov(Fn, Reg, Tmp.Reg);
    }
    else if (Tmp.IsGlobal()) {
        CodeAssembler.GlobalGet(Fn, Reg, Tmp.Global);
    }
    else if (Tmp.IsConstant()) {
        if (Tmp.Type.IsConstString()) {
            if (Tmp.Data <= Const4Max)
                CodeAssembler.StringGet4(Fn, Reg, Tmp.Reg);
            else
                CodeAssembler.StringGet(Fn, Reg, (codefile::StringType)Tmp.Data);
        }
        else if (Tmp.Type.SizeInBits == 4)
            CodeAssembler.Const4(Fn, Reg, Tmp.Reg);
        else if (Tmp.Type.SizeInBits == 8)
            CodeAssembler.Const8(Fn, Reg, Tmp.Reg);
        else if (Tmp.Type.SizeInBits == 16)
            CodeAssembler.Const16(Fn, Reg, (UInt16)Tmp.Data);
        else if (Tmp.Type.SizeInBits == 32)
            CodeAssembler.Const32(Fn, Reg, (UInt32)Tmp.Data);
        else if (Tmp.Type.SizeInBits == 64)
            CodeAssembler.Const64(Fn, Reg, Tmp.Data);
        
    }
}

Byte Emitter::TypeToPrimitive(const AST::TypeDecl& Type) {
    if (Type.IsByte())
        return codefile::PrimitiveByte;
    else if (Type.IsInt())
        return codefile::PrimitiveInt;
    else if (Type.IsInt())
        return codefile::PrimitiveUInt;
    else if (Type.IsFloat())
        return codefile::PrimitiveFloat;

    return 0;
}

Byte Emitter::TypeToAttributes(const AST::TypeDecl& /*Type*/) {
    return 0;
}

}
