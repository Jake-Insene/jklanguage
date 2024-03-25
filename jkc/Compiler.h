#pragma once
#include "jkc/Parser/Parser.h"
#include "jkc/CodeGen/Emitter/EmitterState.h"
#include <stdio.h>

enum class ActionType {
    Parsing,
    CodeGen,
    Disassembly,
};

enum CompilerOption {
    OptionNone = 0,
};

struct ProfileData {
    void(*BeginAction)(const char*, ActionType, bool);
    void(*EndAction)(const char*, ActionType, bool);
};

struct CompileResult {
    bool Success;
};

struct Compiler {
    Compiler(FILE* ErrorStream, const ProfileData& PD);
    ~Compiler();

    CompileResult CompileFromSource(const char* FileName, CodeGen::EmitOptions Options);
    CompileResult Disassembly(const char* FileName, FILE* Output);

    constexpr void BeginAction(const char* FileName, ActionType Type, bool Error) {
        if (PD.BeginAction) {
            PD.BeginAction(FileName, Type, Error);
        }
    }
    
    constexpr void EndAction(const char* FileName, ActionType Type, bool Error) {
        if (PD.EndAction) {
            PD.EndAction(FileName, Type, Error);
        }
    }

    FILE* ErrorStream;
    CodeGen::EmitterState Emitter;
    ProfileData PD;

    Parser SourceParser;
};

