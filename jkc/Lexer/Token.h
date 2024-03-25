#pragma once
#include <jkr/String.h>

enum class Type {
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

    ExternAttr,

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

    BinaryAnd,
    BinaryAndEqual,
    BinaryOr,
    BinaryOrEqual,
    BinaryXOr,
    BinaryXOrEqual,
    BinaryNAND,
    BinaryShl,
    BinaryShlEqual,
    BinaryShr,
    BinaryShrEqual,

    EndOfFile,
};

struct SourceLocation {
    constexpr explicit SourceLocation() :
        FileName(nullptr), Line(0)
    {}
    constexpr explicit SourceLocation(const char* FileName, USize Line) :
        FileName(FileName), Line(Line) 
    {}
    constexpr ~SourceLocation() {}

    const char* FileName;
    USize Line;
};

struct TokenValue {
    constexpr explicit TokenValue() {}
    constexpr ~TokenValue() {}

    TokenValue(TokenValue&&) = default;
    TokenValue& operator=(TokenValue&&) = default;

    String Str{};
    StringView StrRef{};
    union {
        Int Signed = 0;
        UInt Unsigned;
        Float64 Real;
    };
};

struct Token {
    constexpr Token() {}
    constexpr Token(Type Type, TokenValue&& Value, const SourceLocation& Location)
        : Type(Type), Value(std::move(Value)), Location(Location) {}
    constexpr ~Token() {}

    Token(Token&&) = default;
    Token& operator=(Token&&) = default;

    Type Type{};
    TokenValue Value{};
    SourceLocation Location{};
};

