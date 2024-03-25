#pragma once
#include "jkc/Lexer/Token.h"

struct Lexer {
    constexpr Lexer(FILE* ErrorStream, const char* FileName, const StringView& Content) :
        FileName(FileName), ErrorStream(ErrorStream), FileContent(Content), Line(1)
    {
        Current = Char(Content.size() ? Content[0] : '\0');
    }

    void Destroy() {}

    void Set(const char* FilePath, const StringView& Content) {
        FileName = FilePath;
        FileContent = Content;
        IsPanicMode = false;
        Current = 0;
        Index = 0;
        Line = 1;
    
        if (Content.size()) { Current = Content[0]; }
    }

    constexpr bool Success() { return !IsPanicMode; }
    
    Token GetNext();

    constexpr void Advance() {
        if ((Index + 1) < FileContent.size()) {
            Index++;
            Current = FileContent[Index];

            if (Current == '\n') Line++;
        }
        else {
            Current = '\0';
        }
    }

    constexpr void SkipWhiteSpace() {
        while (
            Current == ' ' ||
            Current == '\t' ||
            Current == '\n' ||
            Current == '\r') {
            Advance();
        }
    }

    constexpr void MakeSimple(Type Type, Token& Tk) {
        Tk = Token(Type, TokenValue(), SourceLocation(FileName, Line));
        Advance();
    }

    constexpr void MakeTwo(Type Type, Token& Tk) {
        Tk = Token(Type, TokenValue(), SourceLocation(FileName, Line));
        Advance();
        Advance();
    }

    constexpr void MakeThree(Type Type, Token& Tk) {
        Tk = Token(Type, TokenValue(), SourceLocation(FileName, Line));
        Advance();
        Advance();
        Advance();
    }

    constexpr Char GetOffset(UInt32 Off) {
        return Index + Off < FileContent.size() ? FileContent[Index + Off] : '\0';
    }

    Token GetIdentifier();
    Token GetDigit();
    Token GetString();

    const char* FileName;
    FILE* ErrorStream;
    bool IsPanicMode = false;

    StringView FileContent;
    Char Current = 0;
    USize Index = 0;
    USize Line = 1;
};
