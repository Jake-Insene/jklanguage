#pragma once

#include "jkc/Lexer/Token.h"
#include "jkc/Utility/Slice.h"
#include "jkc/Utility/File.h"

struct Lexer {
    static Lexer New(StreamOutput& ErrorStream, Str FileName, Slice<Char> Content) {
        return Lexer{
            .FileName = FileName,
            .ErrorStream = ErrorStream,
            .Content = Content,
            .Current = IntCast<Char>(Content.Len ? Content[0] : '\0'),
            .Line = 1,
        };
    }

    void Destroy() {}

    constexpr void Set(this Lexer& Self, Str FileName, Slice<Char> Content) {
        Self.FileName = FileName;
        Self.Content = Content;
        Self.IsPanicMode = false;
        Self.Current = 0;
        Self.Index = 0;
        Self.Line = 1;
    
        if (Content.Len) { Self.Current = Content[0]; }
    }

    constexpr bool Success(this const Lexer& Self) { return !Self.IsPanicMode; }
    
    Token GetNext(this Lexer& Self);

    constexpr void Advance(this Lexer& Self) {
        if ((Self.Index + 1) < Self.Content.Len) {
            Self.Index++;
            Self.Current = Self.Content.Data[Self.Index];

            if (Self.Current == '\n') Self.Line++;
        }
        else {
            Self.Current = '\0';
        }
    }

    constexpr void SkipWhiteSpace(this Lexer& Self) {
        while (
            Self.Current == ' ' ||
            Self.Current == '\t' ||
            Self.Current == '\n' ||
            Self.Current == '\r') {
            Self.Advance();
        }
    }

    constexpr void MakeSimple(this Lexer& Self, TokenType Type, Token& Tk) {
        Tk = Token(Type, TokenValue(), SourceLocation(Self.FileName, Self.Line));
        Self.Advance();
    }

    constexpr void MakeTwo(this Lexer& Self, TokenType Type, Token& Tk) {
        Tk = Token(Type, TokenValue(), SourceLocation(Self.FileName, Self.Line));
        Self.Advance();
        Self.Advance();
    }

    constexpr void MakeThree(this Lexer& Self, TokenType Type, Token& Tk) {
        Tk = Token(Type, TokenValue(), SourceLocation(Self.FileName, Self.Line));
        Self.Advance();
        Self.Advance();
        Self.Advance();
    }

    constexpr Char GetOffset(this Lexer& Self, Uint32 Off) {
        return Self.Index + Off < Self.Content.Len ? Self.Content[Self.Index + Off] : '\0';
    }

    Token GetIdentifier();
    Token GetDigit();
    Token GetString();

    const Char* FileName;
    StreamOutput& ErrorStream;
    bool IsPanicMode = false;

    Slice<Char> Content;
    Char Current = 0;
    USize Index = 0;
    USize Line = 1;
};
