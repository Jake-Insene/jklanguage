#include "jkc/Compiler.h"
#include "jkc/Parser/Parser.h"
#include "jkc/AST/Utility.h"
#include "jkc/CodeGen/Disassembler.h"

#include <fstream>

static inline List<Char> ReadFileContent(StreamOutput &ErrorStream, Str FileName, bool& Success) {
    io::File file = io::File::Open(FileName);
    if (file.Err != io::FileError::Success)
    {
        ErrorStream.Println(STR("Error: can't open the file '{s}'"), FileName);
        Success = false;
        return List<Char>{
            .Capacity = 0,
            .Size = 0,
            .Data = nullptr,
        };
    }

    size_t size = (size_t)(1 + file.Size());
    List<Char> content{};
    content.GrowCapacity(size);
    file.Read((Byte*)content.Data, size);
    content.Size = size;
    content[size - 1] = '\0';

    file.Close();

    return content;
}

Compiler Compiler::New(StreamOutput& ErrorStream, const ProfileData& PD) {
    return Compiler{
        .ErrorStream = ErrorStream,
        .Emitter = CodeGen::Emitter(ErrorStream),
        .PD = PD,
        .SourceParser = Parser::New(ErrorStream),
    };
}

CompileResult Compiler::CompileFromSource(this Compiler& Self, Str FileName, CodeGen::EmitOptions Options) {
    bool success = true;
    List<Char> content = ReadFileContent(Self.ErrorStream, FileName, success);
    if (!success)
        return CompileResult(false);

    // Parsing
    Slice<Char> fileContent = Slice<Char>(content.Data, content.Size);
    
    Self.BeginAction(FileName, ActionType::Parsing, false);
    AST::Program program = Self.SourceParser.ParseContent(FileName, fileContent);
    content.Destroy();
    Self.EndAction(FileName, ActionType::Parsing, !Self.SourceParser.Success());

    if (!Self.SourceParser.Success()) {
        return CompileResult(false);
    }

    std::u8string outputFile = std::u8string(FileName);
    outputFile = outputFile.substr(0, outputFile.find_last_of('.'));
    outputFile += STR(".jk");

    // CodeGen
    io::File file = io::File::Create(outputFile.c_str());

    Self.BeginAction(FileName, ActionType::CodeGen, false);
    Self.Emitter.Emit(
        program, 
        CodeGen::FileType::Executable,
        Options,
        file
    );
    Self.EndAction(FileName, ActionType::CodeGen, !Self.Emitter.Success);

    program.Destroy();
    if (!Self.Emitter.Success) {
        return CompileResult(false);
    }

    file.Close();

    return CompileResult(true);
}

CompileResult Compiler::Disassembly(this Compiler& Self, Str FileName, StreamOutput& Output) {
    bool success = true;

    Self.BeginAction(FileName, ActionType::Disassembly, false);
    if (!CodeGen::Dis(Output, FileName)) {
        success = false;
    }
    Self.EndAction(FileName, ActionType::Disassembly, false);

    return CompileResult{
        .Success = success,
    };
}

void Compiler::Destroy(this Compiler& Self) {
    Self.SourceParser.Destroy();
}
