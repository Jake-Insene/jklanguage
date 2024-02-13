#include "jkc/Compiler.h"
#include "jkc/Parser/Parser.h"
#include "jkc/AST/Utility.h"
#include "jkc/CodeGen/Disassembler.h"

#include <fstream>

static inline std::vector<Char> ReadFileContent(StreamOutput &ErrorStream, Str FileName) {
    io::File file = io::File::Open(FileName, io::FileRead | io::FileBinary);
    if (file.Err != io::FileError::Success)
    {
        ErrorStream.Println(STR("Error: can't open the file '{s}'"), FileName);
        return {};
    }

    size_t size = (size_t)(1 + file.Size());
    std::vector<Char> content{};
    content.resize(size);
    file.Read((Byte*)content.data(), size);
    content[size - 1] = '\0';

    file.Close();

    return content;
}

CompileResult Compiler::CompileFromSource(this Compiler& Self, Str FileName, CompilerOption /*Options*/) {
    std::vector<Char> content = ReadFileContent(Self.ErrorStream, FileName);
    if (content.size() == 0)
        return CompileResult(false);

    Slice<Char> fileContent = Slice<Char>(content.data(), content.size());

    // Parsing
    CompileResult result{};
    Self.BeginAction(FileName, ActionType::Parsing, false);
    AST::Program program = Self.SourceParser.ParseContent(FileName, fileContent);
    if (!Self.SourceParser.Success()) {
        result.Success = false;
        return result;
    }
    Self.EndAction(FileName, ActionType::Parsing, !Self.SourceParser.Success());

    Self.PreParse();
    AST::InsertPrograms(program, Self.PreParsedPrograms);
    Self.PreParsedPrograms.Clear();

    std::u8string outputFile = std::u8string(FileName);
    outputFile = outputFile.substr(0, outputFile.find_last_of('.'));
    outputFile += STR(".jk");

    // CodeGen
    io::File file = io::File::Open(outputFile.c_str(), io::FileWrite | io::FileBinary);

    Self.BeginAction(FileName, ActionType::CodeGen, false);
    Self.Emitter.Emit(program, CodeGen::FileType::Executable, file);
    program.Destroy();
    if (!Self.Emitter.Success) {
        result.Success = false;
        file.Close();
        return result;
    }
    Self.EndAction(FileName, ActionType::CodeGen, false);

    file.Close();

    return result;
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

Str BuiltinFiles[] = {
#ifdef _WIN32
    STR("std/builtin/windows.jkl"),
#else
    STR("std/builtin/linux.jkl"),
#endif // _WIN32
};

void Compiler::PreParse(this Compiler& Self) {
    Self.PreParseSuccess = true;

    for (auto &file : BuiltinFiles)
    {
        std::vector<Char> content = ReadFileContent(Self.ErrorStream, file);
        if (content.size() == 0)
        {
            Self.ErrorStream.Println(STR("Error: can't open the file '{s}'"), file);
            return;
        }

        Slice<Char> fileContent{content.data(), content.size()};

        Self.PreParsedPrograms.Push() = Self.SourceParser.ParseContent(file, fileContent);
        if (!Self.SourceParser.Success())
        {
            Self.PreParseSuccess = false;
            break;
        }
    }

    if (!Self.PreParseSuccess)
        Self.ErrorStream.Println(STR("Error: Error parsing builtin files"));
}
