#include "jkc/Compiler.h"
#include "jkc/Parser/Parser.h"
#include "jkc/AST/Utility.h"
#include "jkc/CodeGen/Disassembler.h"
#include <fstream>

static inline std::vector<Char> ReadFileContent(FILE* ErrorStream, const char* FileName, bool& Success) {
    std::ifstream file{ FileName, std::ios::ate | std::ios::binary };
    if (!file.is_open()) {
        fprintf(ErrorStream, "Error: can't open the file '%s'\n", FileName);
        Success = false;
        return std::vector<Char>{};
    }

    size_t size = (size_t)(1 + file.tellg());
    file.seekg(0);
    std::vector<Char> content{};
    content.resize(size);
    file.read((char*)content.data(), size);
    content[size - 1] = '\0';

    file.close();
    return content;
}

Compiler::Compiler(FILE* ErrorStream, const ProfileData& PD) :
    ErrorStream(ErrorStream), Emitter(ErrorStream), PD(PD), SourceParser(ErrorStream) {}

Compiler::~Compiler() {}

CompileResult Compiler::CompileFromSource(const char* FileName, CodeGen::EmitOptions Options) {
    bool success = true;
    std::vector<Char> content = ReadFileContent(ErrorStream, FileName, success);
    if (!success)
        return CompileResult(false);

    // Parsing
    StringView fileContent = StringView(content.data(), content.size());

    BeginAction(FileName, ActionType::Parsing, false);
    AST::Program program = SourceParser.ParseContent(FileName, fileContent);
    EndAction(FileName, ActionType::Parsing, !SourceParser.Success());

    if (!SourceParser.Success()) {
        return CompileResult(false);
    }

    String outputFile = (Str)FileName;
    outputFile = outputFile.substr(0, outputFile.find_last_of('.'));
    outputFile += u8".jk";

    // CodeGen
    std::ofstream file{ (const char*)outputFile.c_str(), std::ios::binary};

    BeginAction(FileName, ActionType::CodeGen, false);
    Emitter.Emit(
        program,
        CodeGen::FileType::Executable,
        Options,
        file
    );
    EndAction(FileName, ActionType::CodeGen, !Emitter.Success);

    if (!Emitter.Success) {
        return CompileResult(false);
    }

    file.close();

    return CompileResult(true);
}

CompileResult Compiler::Disassembly(const char* FileName, FILE* Output) {
    bool success = true;

    BeginAction(FileName, ActionType::Disassembly, false);
    if (!CodeGen::Dis(Output, FileName)) {
        success = false;
    }
    EndAction(FileName, ActionType::Disassembly, false);

    return CompileResult{
        .Success = success,
    };
}

