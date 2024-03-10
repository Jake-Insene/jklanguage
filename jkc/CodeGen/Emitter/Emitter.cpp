#include "jkc/CodeGen/Emitter/Emitter.h"
#include "jkc/AST/Statements.h"
#include "jkc/AST/Expresions.h"
#include <jkr/CodeFile/Header.h>
#include <jkr/CodeFile/Function.h>
#include <jkr/CodeFile/Data.h>
#include <jkr/CodeFile/Type.h>

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

    header.NumberOfSections = 0;
    if (Functions.Size()) {
        header.NumberOfSections++;
    }
    if (Globals.Size()) {
        header.NumberOfSections++;
    }
    if (Strings.Size) {
        header.NumberOfSections++;
    }

    if (FileTy == FileType::Executable) {
        auto it = Functions.Find(STR("Main"));
        if (it == Functions.end()) {
            GlobalError(STR("The entry point was not defined"));
            return;
        }

        header.FileType = codefile::Executable;
        header.EntryPoint = (UInt32)it->second;
        header.MajorVersion = 1;
        header.MinorVersion = 0;
    }

    // Calculate CheckSize
    header.CheckSize += sizeof(codefile::SectionHeader) * header.NumberOfSections;

    for (auto& fn : Functions.Data) {
        header.CheckSize += UInt32(sizeof(codefile::FunctionHeader) + fn.Code.Buff.Size);
    }

    for (auto& global : Globals.Data) {
        header.CheckSize += UInt32(sizeof(codefile::DataHeader));

        header.CheckSize += UInt32(global.Type.SizeInBits / 8);
    }

    for (auto& str : Strings) {
        header.CheckSize += UInt32(str.Size + 2);
    }

    // Writing
    Output.Write((Byte*)&header, sizeof(codefile::FileHeader));

    // Code section
    if (Functions.Size()) {
        codefile::SectionHeader codeS = {
            .Type = codefile::SectionCode,
            .Flags = 0,
            .UserData = 0,
            .CountOfElements = (UInt32)Functions.Size(),
        };
        Output.Write((Byte*)&codeS, sizeof(codefile::SectionHeader));
    }

    for (auto& fn : Functions.Data) {
        codefile::FunctionHeader fnHeader = {
                .Flags = codefile::FunctionNone,
                .SizeOfCode = (UInt16)fn.Code.Buff.Size,
        };

        if (fn.IsNative) {
            fnHeader.Flags |= codefile::FunctionNative;
            fnHeader.SizeOfCode = fn.LibraryAddress;
            fnHeader.LocalReserve = fn.EntryAddress;
            Output.Write(Cast<Byte*>(&fnHeader), sizeof(codefile::FunctionHeader));
        }
        else {
            fnHeader.LocalReserve = fn.CountOfStackLocals;

            Output.Write(Cast<Byte*>(&fnHeader), sizeof(codefile::FunctionHeader));
            Output.Write(Cast<Byte*>(fn.Code.Buff.Data), fn.Code.Buff.Size);
        }
    }

    // Data section
    if (Globals.Size()) {
        codefile::SectionHeader dataS = {
            .Type = codefile::SectionData,
            .Flags = 0,
            .UserData = 0,
            .CountOfElements = (UInt32)Globals.Size(),
        };
        Output.Write((Byte*)&dataS, sizeof(codefile::SectionHeader));
    }

    for (auto& global : Globals.Data) {
        codefile::DataHeader gHeader = {
            .Primitive = TypeToPrimitive(global.Type),
        };

        Output.Write(Cast<Byte*>(&gHeader), sizeof(codefile::DataHeader));
        Output.Write(Cast<Byte*>(&global.Value.Unsigned), global.Type.SizeInBits / 8);
    }

    // ST section
    if (Strings.Size) {
        codefile::SectionHeader stS = {
            .Type = codefile::SectionST,
            .Flags = 0,
            .UserData = 0,
            .CountOfElements = (UInt32)Strings.Size,
        };
        Output.Write((Byte*)&stS, sizeof(codefile::SectionHeader));
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

Function* Emitter::GetFn(AST::Expresion* Target) {
    if (Target->Type == AST::ExpresionType::Identifier) {
        auto id = (AST::Identifier*)Target;
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

void Emitter::PushTmp(Function& Fn, const TmpValue& Tmp) {
    if (Tmp.IsRegister()) {
        CodeAssembler.Push(Fn, Tmp.Reg);
        DeallocateRegister(Tmp.Reg);
    }
    else if (Tmp.IsLocal()) {
        UInt8 reg = AllocateRegister();
        MoveTmp(Fn, reg, Tmp);
        CodeAssembler.Push(Fn, reg);
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
                CodeAssembler.StringGet(Fn, Reg, UInt32(Tmp.Data));
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

codefile::PrimitiveType Emitter::TypeToPrimitive(const AST::TypeDecl& Type) {
    if (Type.IsByte())
        return codefile::PrimitiveByte;
    else if (Type.IsInt())
        return codefile::PrimitiveInt;
    else if (Type.IsUInt())
        return codefile::PrimitiveUInt;
    else if (Type.IsFloat())
        return codefile::PrimitiveFloat;
    else if (Type.IsArray()) {
        if (Type.Primitive == AST::TypeDecl::Type::Byte)
            return codefile::PrimitiveByte;
        else if (Byte(Type.Primitive) >= Byte(AST::TypeDecl::Type::Int) &&
                 Byte(Type.Primitive) <= Byte(AST::TypeDecl::Type::Any))
            return codefile::PrimitiveAny;
    }

    return codefile::PrimitiveAny;
}

void Emitter::MoveConst(Function& Fn, Byte Dest, UInt64 Const) {
    if (Const <= Const4Max) {
        CodeAssembler.Const4(Fn, Dest, Byte(Const));
    }
    else if (Const <= ByteMax) {
        CodeAssembler.Const8(Fn, Dest, Byte(Const));
    }
    else if (Const <= UINT16_MAX) {
        CodeAssembler.Const16(Fn, Dest, UInt16(Const));
    }
    else if (Const <= UINT32_MAX) {
        CodeAssembler.Const32(Fn, Dest, UInt32(Const));
    }
    else {
        CodeAssembler.Const64(Fn, Dest, Const);
    }
}

}
