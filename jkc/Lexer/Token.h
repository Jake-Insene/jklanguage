#pragma once
#include "jkr/CoreHeader.h"
#include <string>

enum class TokenType {
    Unknown = 0,

    Identifier,
    ConstInteger,
    ConstUInteger,
    ConstFloat,
    ConstString,

    Fn,
    Return,
    Var,
    Const,

    If,
    Elif,
    Else,
    For,
    While,

    TypeAny,
    TypeVoid,
    TypeByte,
    TypeInt,
    TypeUInt,
    TypeFloat,

    Comma,
    Dot,
    LeftParent,
    RightParent,
    LeftBrace,
    RightBrace,
    LeftKey,
    RightKey,
    Semicolon,
    Colon,
    Equal,
    Compare,
    Not,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,

    Plus,
    PlusEqual,
    Inc,
    Minus,
    MinusEqual,
    Dec,
    Star,
    StarEqual,
    Slash,
    SlashEqual,

    EndOfFile,
};

struct SourceLocation {
    constexpr explicit SourceLocation() noexcept :
        FileName(nullptr), Line(0)
    {}
    constexpr explicit SourceLocation(Str FileName, USize Line) noexcept :
        FileName(FileName), Line(Line) 
    {}
    constexpr ~SourceLocation() {}

    Str FileName;
    USize Line;
};

struct TokenValue {
    constexpr explicit TokenValue() {}
    constexpr ~TokenValue() {}

    TokenValue(TokenValue&&) = default;
    TokenValue& operator=(TokenValue&&) = default;

    std::u8string Str{};
    union {
        Int Signed = 0;
        UInt Unsigned;
        Float64 Real;
    };
};

struct Token {
    constexpr Token() {}
    constexpr Token(TokenType Type, TokenValue&& Value, const SourceLocation& Location)
        : Type(Type), Value(std::move(Value)), Location(Location) {}
    constexpr ~Token() {}

    Token(const Token&) = delete;
    Token& operator=(const Token&) = delete;
    Token(Token&&) = default;
    Token& operator=(Token&&) = default;

    TokenType Type{};
    TokenValue Value{};
    SourceLocation Location{};
};

