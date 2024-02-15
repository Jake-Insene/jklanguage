#pragma once
#include "jkc/Utility/File.h"
#include "jkc/Parser/Parser.h"
#include "jkc/CodeGen/Emitter.h"
#include <stdio.h>
#include <string>

enum class ActionType {
    Parsing,
    CodeGen,
    Disassembly,
};

enum CompilerOption {
    OptionNone = 0,
};

struct ProfileData {
    void(*BeginAction)(Str, ActionType, bool);
    void(*EndAction)(Str, ActionType, bool);
};

struct CompileResult {
    bool Success = true;
};

struct Compiler {

    static Compiler New(StreamOutput& ErrorStream, const ProfileData& PD) {
        return Compiler{
            .ErrorStream = ErrorStream,
            .Emitter = CodeGen::Emitter(ErrorStream),
            .PD = PD,
            .SourceParser = Parser::New(ErrorStream),
            .PreParsedPrograms = List<AST::Program>::New(0)
       };
    }

    CompileResult CompileFromSource(this Compiler& Self, Str FileName, CodeGen::EmitOptions Options);
    CompileResult Disassembly(this Compiler& Self, Str FileName, StreamOutput& Output);

    void PreParse(this Compiler& Self);

    void Destroy(this Compiler& Self);

    constexpr void BeginAction(this Compiler& Self, Str FileName, ActionType Type, bool Error) {
        if (Self.PD.BeginAction) {
            Self.PD.BeginAction(FileName, Type, Error);
        }
    }

    constexpr void EndAction(this Compiler& Self, Str FileName, ActionType Type, bool Error) {
        if (Self.PD.EndAction) {
            Self.PD.EndAction(FileName, Type, Error);
        }
    }

    StreamOutput& ErrorStream;
    CodeGen::Emitter Emitter;
    ProfileData PD;

    Parser SourceParser;
    List<AST::Program> PreParsedPrograms;
    bool PreParseSuccess = 0;
};

