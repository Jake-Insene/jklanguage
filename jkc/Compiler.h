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
            .PreParsedPrograms = List<AST::Program>::New(2)
       };
    }

    [[nodiscard]] constexpr bool PreSuccess(this const Compiler& Self) { return Self.PreParseSuccess; }

    CompileResult CompileFromSource(this Compiler& Self, Str FileName);

    void PreParse(this Compiler& Self);

    void Destroy(this Compiler& Self) {
        Self.PreParsedPrograms.Destroy();
        Self.SourceParser.Destroy();
    }

    StreamOutput& ErrorStream;
    CodeGen::Emitter Emitter;
    ProfileData PD;

    Parser SourceParser;
    List<AST::Program> PreParsedPrograms;
    bool PreParseSuccess = 0;
};

