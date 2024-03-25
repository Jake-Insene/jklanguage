#include "jkc/Lexer/Lexer.h"
#include <jkr/Definitions.h>
#include <cassert>

struct IdentifierInfo {
    UInt32 Len;
    Str Str;
    Type Type;
};

static IdentifierInfo Identifiers[] = {
    {.Len = 2, .Str = u8"fn", .Type = Type::Fn },
    {.Len = 6, .Str = u8"return", .Type = Type::Return },
    
    {.Len = 3, .Str = u8"var", .Type = Type::Var },
    {.Len = 5, .Str = u8"const", .Type = Type::Const },
    
    {.Len = 2, .Str = u8"if", .Type = Type::If },
    {.Len = 4, .Str = u8"elif", .Type = Type::Elif },
    {.Len = 4, .Str = u8"else", .Type = Type::Else },
    {.Len = 3, .Str = u8"for", .Type = Type::For },
    {.Len = 5, .Str = u8"while", .Type = Type::While },

    {.Len = 3, .Str = u8"Any", .Type = Type::TypeAny },
    {.Len = 4, .Str = u8"Void", .Type = Type::TypeVoid },
    {.Len = 4, .Str = u8"Byte", .Type = Type::TypeByte },
    {.Len = 4, .Str = u8"Bool", .Type = Type::TypeByte },
    {.Len = 3, .Str = u8"Int", .Type = Type::TypeInt },
    {.Len = 4, .Str = u8"UInt", .Type = Type::TypeUInt },
    {.Len = 5, .Str = u8"Float", .Type = Type::TypeFloat },

    {.Len = 6, .Str = u8"extern", .Type = Type::ExternAttr },
};

static inline void FindIdentifier(Token& Tk) {
    for (UInt32 i = 0; i < std::size(Identifiers); i++) {
        if (Identifiers[i].Len != Tk.Value.StrRef.size())
            continue;

        if (memcmp(Identifiers[i].Str, Tk.Value.StrRef.data(), Identifiers[i].Len) == 0) {
            Tk.Type = Identifiers[i].Type;
            return;
        }
    }

    Tk.Type = Type::Identifier;
}

Token Lexer::GetIdentifier() {
    Token tk{};
    tk.Type = Type::Unknown;
    tk.Location = SourceLocation(FileName, Line);

    USize start = Index;
    while (isalnum(Current) || Current == '_') {
        Advance();
    }
    tk.Value.StrRef = FileContent.substr(start, Index - start);

    FindIdentifier(tk);
    return tk;
}

Token Lexer::GetDigit() {
    Token tk{};
    tk.Type = Type::ConstInteger;
    tk.Location = SourceLocation(FileName, Line);
    Int32 base = 10;

    std::string digit{};
loop:
    while (isdigit(Current)) {
        digit.push_back(Current);
        Advance();
    }

    if (Current == '.') {
        tk.Type = Type::ConstFloat;
        Advance();
        digit.push_back('.');
        goto loop;
    }
    else if (Current == 'U') {
        tk.Type = Type::ConstUInteger;
        Advance();
    }

    if (tk.Type == Type::ConstFloat)
        tk.Value.Real = std::stod(digit);
    else if (tk.Type == Type::ConstUInteger)
        tk.Value.Unsigned = std::stoull(digit, NULL, base);
    else
        tk.Value.Signed = std::stoll(digit, NULL, base);
    return tk;
}

Token Lexer::GetString() {
    Token tk{};
    tk.Type = Type::ConstString;
    tk.Location = SourceLocation(FileName, Line);

    Advance();

    tk.Value.Str = {};
    while (Current != '\"' && Current != '\0') {
        if (Current == '\\') {
            Advance();
            switch (Current) {
            case '0':
                tk.Value.Str.push_back('\0');
                break;
            case 'n':
                tk.Value.Str.push_back('\n');
                break;
            case 't':
                tk.Value.Str.push_back('\t');
                break;
            case 'r':
                tk.Value.Str.push_back('\r');
                break;
            default:
                assert(0 && "Invalid string character");
                UNREACHABLE();
                break;
            }
            Advance();

        }
        else {
            tk.Value.Str.push_back(Current);
            Advance();
        }
    }

    if (Current == '\0') {
        fprintf(ErrorStream, "%s:%llu: Error: Bad string\n", FileName, Line);
        IsPanicMode = true;
    }

    Advance();

    return tk;
}

Token Lexer::GetNext() {
start:
    Token tk{};

    SkipWhiteSpace();

    switch (Current) {
    case ',':
        MakeSimple(Type::Comma, tk);
        break;
    case '.':
        MakeSimple(Type::Dot, tk);
        break;
    case '(':
        MakeSimple(Type::LeftParent, tk);
        break;
    case ')':
        MakeSimple(Type::RightParent, tk);
        break;
    case '{':
        MakeSimple(Type::LeftBrace, tk);
        break;
    case '}':
        MakeSimple(Type::RightBrace, tk);
        break;
    case '[':
        MakeSimple(Type::LeftKey, tk);
        break;
    case ']':
        MakeSimple(Type::RightKey, tk);
        break;
    case ';':
        MakeSimple(Type::Semicolon, tk);
        break;
    case ':':
        MakeSimple(Type::Colon, tk);
        break;
    case '=':
        if (GetOffset(1) == '=')
            MakeTwo(Type::Compare, tk);
        else
            MakeSimple(Type::Equal, tk);
        break;
    case '!':
        if (GetOffset(1) == '=')
            MakeTwo(Type::NotEqual, tk);
        else
            MakeSimple(Type::Not, tk);
        break;
    case '<':
        if (GetOffset(1) == '=')
            MakeTwo(Type::LessEqual, tk);
        else if (GetOffset(1) == '<') {
            if (GetOffset(2) == '=')
                MakeThree(Type::BinaryShlEqual, tk);
            else
                MakeTwo(Type::BinaryShl, tk);
        }
        else
            MakeSimple(Type::Less, tk);
        break;
    case '>':
        if (GetOffset(1) == '=')
            MakeTwo(Type::GreaterEqual, tk);
        else if (GetOffset(1) == '>') {
            if (GetOffset(2) == '=')
                MakeThree(Type::BinaryShrEqual, tk);
            else
                MakeTwo(Type::BinaryShr, tk);
        }
        else
            MakeSimple(Type::Greater, tk);
        break;
    case '+':
        if (GetOffset(1) == '=')
            MakeTwo(Type::PlusEqual, tk);
        else if (GetOffset(1) == '+')
            MakeTwo(Type::Inc, tk);
        else
            MakeSimple(Type::Plus, tk);
        break;
    case '-':
        if (GetOffset(1) == '=')
            MakeTwo(Type::MinusEqual, tk);
        else if (GetOffset(1) == '-')
            MakeTwo(Type::Dec, tk);
        else
            MakeSimple(Type::Minus, tk);
        break;
    case '*':
        if (GetOffset(1) == '=')
            MakeTwo(Type::StarEqual, tk);
        else
            MakeSimple(Type::Star, tk);
        break;
    case '/':
        if (GetOffset(1) == '=')
            MakeTwo(Type::SlashEqual, tk);
        else if (GetOffset(1) == '/') {
            while (Current != '\n')
                Advance();
            goto start;
        }
        else
            MakeSimple(Type::Slash, tk);
        break;
    case '&':
        if (GetOffset(1) == '=')
            MakeTwo(Type::BinaryAndEqual, tk);
        else
            MakeSimple(Type::BinaryAnd, tk);
        break;
    case '|':
        if (GetOffset(1) == '=')
            MakeTwo(Type::BinaryOrEqual, tk);
        else
            MakeSimple(Type::BinaryOr, tk);
        break;
    case '^':
        if (GetOffset(1) == '=')
            MakeTwo(Type::BinaryXOrEqual, tk);
        else
            MakeSimple(Type::BinaryXOr, tk);
        break;
    case '~':
        MakeSimple(Type::BinaryNAND, tk);
        break;
    case '\"':
        tk = GetString();
        break;
    case '\'':
        break;
    case '\0':
        MakeSimple(Type::EndOfFile, tk);
        break;
    default:
        if (isalpha(Current) || Current == '_') {
            tk = GetIdentifier();
        }
        else if (isdigit(Current)) {
            tk = GetDigit();
        }
        else {
            fprintf(ErrorStream,
                    "%s:%llu: Error: Unexpected character %c",
                    FileName, Line, Current
            );
            IsPanicMode = true;
            Advance();
        }
        break;
    }

    return tk;
}

