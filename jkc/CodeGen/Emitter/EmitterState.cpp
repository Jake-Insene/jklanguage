#include "jkc/CodeGen/Emitter/EmitterState.h"
#include "jkc/CodeGen/Emitter/PreEmit.h"
#include "jkc/CodeGen/Emitter/EmitStat.h"
#include "jkc/AST/Statements.h"
#include "jkc/AST/Expresions.h"
#include <jkr/CodeFile/Header.h>
#include <jkr/CodeFile/Function.h>
#include <jkr/CodeFile/Data.h>
#include <jkr/CodeFile/Type.h>
#include <jkr/Utility.h>
#include <stdarg.h>

namespace CodeGen {

EmitterState::EmitterState(FILE* ErrorStream) : ErrorStream(ErrorStream) {}

EmitterState::~EmitterState() {}

void EmitterState::Emit(AST::Program& Program, FileType FileTy, EmitOptions Options, std::ostream& Output) {
    CurrentOptions = Options;
    Functions.Clear();
    Globals.Clear();
    Context = {};

    PreEmit(*this, Program);
    
    EmitProgramStatements(*this, Program);

    codefile::FileHeader header = {
            .Signature = {},
            .CheckSize = sizeof(codefile::FileHeader),
    };

    memcpy_s((Char*)&header.Signature, sizeof(header.Signature), codefile::Signature, sizeof(codefile::Signature));

    if (FileTy == FileType::Executable) {
        auto it = Functions.Find(u8"Main");
        if (it == Functions.end()) {
            GlobalError(u8"The entry point was not defined");
            return;
        }

        header.FileType = codefile::Executable;
        header.EntryPoint = (UInt32)it->second;
        header.MajorVersion = 1;
        header.MinorVersion = 0;
    }

    header.DataSize = UInt16(Globals.Size());
    header.FunctionSize = UInt32(Functions.Size());
    header.StringsSize = UInt32(Strings.size());

    // Calculate CheckSize
    for (auto& global : Globals.Items) {
        header.CheckSize += UInt32(sizeof(codefile::DataHeader));
        header.CheckSize += UInt32(global.Type.SizeInBits / 8);
    }

    for (auto& fn : Functions.Items) {
        header.CheckSize += UInt32(sizeof(codefile::FunctionHeader) + fn.Code.Buff.size());
    }

    for (auto& str : Strings) {
        header.CheckSize += UInt32((str.Size + 1) + 2);
    }

    // Writing
    Output.write((const char*)&header, sizeof(codefile::FileHeader));

    // Data section
    for (auto& global : Globals.Items) {
        codefile::DataHeader gHeader = {
            .Primitive = TypeToPrimitive(global.Type),
        };

        Output.write((const char*)(&gHeader), sizeof(codefile::DataHeader));
        Output.write((const char*)(&global.Value.Unsigned), global.Type.SizeInBits / 8);
    }

    // Code section
    for (auto& fn : Functions.Items) {
        codefile::FunctionHeader fnHeader = {
                .Flags = codefile::FunctionNone,
        };

        if (fn.IsExtern) {
            fnHeader.Flags |= codefile::FunctionNative;
            fnHeader.SizeOfCode = fn.LibraryAddress;
            fnHeader.StackArguments = UInt16(fn.EntryAddress & 0xFFFF);
            fnHeader.LocalReserve = UInt16(fn.EntryAddress>>16);
            Output.write((char*)&fnHeader, sizeof(codefile::FunctionHeader));
        }
        else {
            fnHeader.StackArguments = fn.StackArguments;
            fnHeader.LocalReserve = fn.CountOfStackLocals;
            fnHeader.SizeOfCode = UInt32(fn.Code.Buff.size()),

            Output.write((char*)&fnHeader, sizeof(codefile::FunctionHeader));
            Output.write((char*)fn.Code.Buff.data(), fn.Code.Buff.size());
        }
    }

    // ST section
    for (auto& s : Strings) {
        UInt16 size = UInt16(s.Size + 1);
        Output.write((const char*)&size, 2);
        Output.write((const char*)s.Data, s.Size);
        Output.put('\0');
    }
}

void EmitterState::Warn(const SourceLocation& Location, Str Format, ...) {
    fprintf(ErrorStream, "%s:%llu: Warn: ", Location.FileName, Location.Line);

    va_list args;
    va_start(args, Format);
    vfprintf(ErrorStream, (char*)Format, args);
    fputc('\n', ErrorStream);
    va_end(args);
}

void EmitterState::Error(const SourceLocation& Location, Str Format, ...) {
    fprintf(ErrorStream, "%s:%llu: Error: ", Location.FileName, Location.Line);

    va_list args;
    va_start(args, Format);
    vfprintf(ErrorStream, (char*)Format, args);
    fputc('\n', ErrorStream);
    va_end(args);
    Success = false;
}

void EmitterState::TypeError(const AST::TypeDecl& LHS, const AST::TypeDecl& RHS, 
                        const SourceLocation& Location, Str Format, ...) {
    if (LHS != RHS) {
        fprintf(ErrorStream, "%s:%llu: Error: ", Location.FileName, Location.Line);
        va_list args;
        va_start(args, Format);
        vfprintf(ErrorStream, (char*)Format, args);
        fputc('\n', ErrorStream);
        va_end(args);
        Success = false;
    }
}

void EmitterState::GlobalError(Str Format, ...) {
    fprintf(ErrorStream, "Error: ");

    va_list args;
    va_start(args, Format);
    vfprintf(ErrorStream, (char*)Format, args);
    fputc('\n', ErrorStream);
    va_end(args);
    Success = false;
}

TmpValue EmitterState::GetID(const StringView& ID, Function& Fn, const SourceLocation& Location) {
    {
        auto it = Fn.Locals.Find(ID);
        if (it != Fn.Locals.end()) {
            auto& local = Fn.Locals.Get(it->second);
            return TmpValue{
                .Ty = local.IsRegister ? TmpType::LocalReg : TmpType::Local,
                .Data = local.Index,
                .Index = (UInt32)it->second,
                .Type = local.Type,
            };
        }
    }

    {
        auto it = Globals.Find(ID);
        if (it == Globals.end()) {
            Error(Location,
                u8"Undefined reference to '%s'",
                ID.data()
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

Function* EmitterState::GetFn(AST::Expresion* Target) {
    if (Target->Type == AST::ExpresionType::Identifier) {
        auto id = (AST::Identifier*)Target;
        auto it = Functions.Find(id->ID);
        if (it != Functions.end() && (Functions.Get(it->second).IsDefined || Functions.Get(it->second).IsExtern)) {
            return &Functions.Get(it->second);
        }

        Error(id->Location,
            u8"Undefined reference to function '%s'",
            id->ID.c_str()
        );
    }
    return nullptr;
}

void EmitterState::PushTmp(Function& Fn, const TmpValue& Tmp) {
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
        if (Tmp.Data <= ByteMax) {
            CodeAssembler.Push8(Fn, Byte(Tmp.Data));
        }
        else if(Tmp.Data <= Const16Max){
            CodeAssembler.Push16(Fn, UInt16(Tmp.Data));
        }
        else if (Tmp.Data <= Const32Max) {
            CodeAssembler.Push32(Fn, UInt32(Tmp.Data));
        }
        else if (Tmp.Data <= UIntMax) {
            CodeAssembler.Push64(Fn, Tmp.Data);
        }
    }
}

void EmitterState::MoveTmp(Function& Fn, UInt8 Reg, const TmpValue& Tmp) {
    if (Tmp.IsRegister()) {
        CodeAssembler.Mov(Fn, Reg, Tmp.Reg);
    }
    else if (Tmp.IsLocal()) {
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
            CodeAssembler.Ldsr(Fn, Reg, UInt32(Tmp.Data));
        }
        else if (Tmp.Data <= Const4Max) {
            CodeAssembler.Mov4(Fn, Reg, Byte(Tmp.Data));
        }
        else if (Tmp.Data <= ByteMax) {
            CodeAssembler.Mov8(Fn, Reg, Byte(Tmp.Data));
        }
        else if (Tmp.Data <= Const16Max) {
            CodeAssembler.Mov16(Fn, Reg, UInt16(Tmp.Data));
        }
        else if (Tmp.Data <= Const32Max) {
            CodeAssembler.Mov32(Fn, Reg, UInt32(Tmp.Data));
        }
        else if (Tmp.Data <= UIntMax) {
            CodeAssembler.Mov64(Fn, Reg, Tmp.Data);
        }
    }
}

codefile::PrimitiveType EmitterState::TypeToPrimitive(const AST::TypeDecl& Type) {
    if (Type.IsByte())
        return codefile::PrimitiveByte;
    else if (Type.IsInt())
        return codefile::PrimitiveInt;
    else if (Type.IsUInt())
        return codefile::PrimitiveUInt;
    else if (Type.IsFloat())
        return codefile::PrimitiveFloat;
    
    return codefile::PrimitiveAny;
}

void EmitterState::MoveConst(Function& Fn, Byte Dest, UInt64 Const) {
    if (Const <= Const4Max) {
        CodeAssembler.Mov4(Fn, Dest, Byte(Const));
    }
    else if (Const <= ByteMax) {
        CodeAssembler.Mov8(Fn, Dest, Byte(Const));
    }
    else if (Const <= Const16Max) {
        CodeAssembler.Mov16(Fn, Dest, UInt16(Const));
    }
    else if (Const <= Const32Max) {
        CodeAssembler.Mov32(Fn, Dest, UInt32(Const));
    }
    else if (Const <= UIntMax) {
        CodeAssembler.Mov64(Fn, Dest, Const);
    }
}

codefile::ArrayElement EmitterState::TypeToArrayElement(const AST::TypeDecl& Type) {
    if (Type.IsByte())return codefile::AE_1B;
    else if (Type.IsInt() || Type.IsUInt() || Type.IsFloat()) return codefile::AE_8B;

    assert(0 && "Invalid array element type");
    return codefile::AE_1B;
}

}
