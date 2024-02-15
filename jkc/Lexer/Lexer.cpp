#include "jkc/Lexer/Lexer.h"

struct IdentifierInfo {
    Uint32 Len;
    const char* Str;
    TokenType Type;
};

static IdentifierInfo Identifiers[] = {
        {.Len = 2, .Str = "fn", .Type = TokenType::Fn },
        {.Len = 6, .Str = "return", .Type = TokenType::Return },
        
        {.Len = 3, .Str = "var", .Type = TokenType::Var },
        {.Len = 5, .Str = "const", .Type = TokenType::Const },
        
        {.Len = 2, .Str = "if", .Type = TokenType::If },
        {.Len = 4, .Str = "elif", .Type = TokenType::Elif },
        {.Len = 4, .Str = "else", .Type = TokenType::Else },
        {.Len = 3, .Str = "for", .Type = TokenType::For },
        {.Len = 5, .Str = "while", .Type = TokenType::While },

        {.Len = 3, .Str = "any", .Type = TokenType::TypeAny },
        {.Len = 4, .Str = "void", .Type = TokenType::TypeVoid },
        {.Len = 4, .Str = "byte", .Type = TokenType::TypeByte },
        {.Len = 4, .Str = "bool", .Type = TokenType::TypeByte },
        {.Len = 3, .Str = "int", .Type = TokenType::TypeInt },
        {.Len = 4, .Str = "uint", .Type = TokenType::TypeUInt },
        {.Len = 5, .Str = "float", .Type = TokenType::TypeFloat },
};

static inline void FindIdentifier(Token& Tk) {
    for (Uint32 i = 0; i < std::size(Identifiers); i++) {
        if (Identifiers[i].Len != Tk.Value.Str.size())
            continue;

        if (memcmp(Identifiers[i].Str, Tk.Value.Str.c_str(), Identifiers[i].Len) == 0) {
            Tk.Type = Identifiers[i].Type;
            return;
        }
    }

    Tk.Type = TokenType::Identifier;
}

Token Lexer::GetIdentifier() {
    Token tk{};
    tk.Type = TokenType::Unknown;
    tk.Location = SourceLocation(FileName, Line);

    tk.Value.Str = {};
    while (isalnum(Current) || Current == '_') {
        tk.Value.Str.push_back(Current);
        Advance();
    }

    FindIdentifier(tk);
    return tk;
}

Token Lexer::GetDigit() {
    Token tk{};
    tk.Type = TokenType::ConstInteger;
    tk.Location = SourceLocation(FileName, Line);
    Int32 base = 10;

    std::string digit{};
loop:
    while (isdigit(Current)) {
        digit.push_back(Current);
        Advance();
    }

    if (Current == '.') {
        tk.Type = TokenType::ConstFloat;
        Advance();
        digit.push_back('.');
        goto loop;
    }
    else if (Current == 'U') {
        tk.Type = TokenType::ConstUInteger;
        Advance();
    }

    if (tk.Type == TokenType::ConstFloat)
        tk.Value.Real = std::stod(digit);
    else if (tk.Type == TokenType::ConstUInteger)
        tk.Value.Unsigned = std::stoull(digit, NULL, base);
    else
        tk.Value.Signed = std::stoll(digit, NULL, base);
    return tk;
}

Token Lexer::GetString() {
    Token tk{};
    tk.Type = TokenType::ConstString;
    tk.Location = SourceLocation(FileName, Line);

    Advance();

    tk.Value.Str = {};
    while (Current != '\"' && Current != '\0') {
        tk.Value.Str.push_back(Current);
        Advance();
    }

    Advance();

    return tk;
}

Token Lexer::GetNext(this Lexer& Self) {
start:
    Token tk{};

    Self.SkipWhiteSpace();

    switch (Self.Current) {
    case ',':
        Self.MakeSimple(TokenType::Comma, tk);
        break;
    case '.':
        Self.MakeSimple(TokenType::Dot, tk);
        break;
    case '(':
        Self.MakeSimple(TokenType::LeftParent, tk);
        break;
    case ')':
        Self.MakeSimple(TokenType::RightParent, tk);
        break;
    case '{':
        Self.MakeSimple(TokenType::LeftBrace, tk);
        break;
    case '}':
        Self.MakeSimple(TokenType::RightBrace, tk);
        break;
    case '[':
        Self.MakeSimple(TokenType::LeftKey, tk);
        break;
    case ']':
        Self.MakeSimple(TokenType::RightKey, tk);
        break;
    case ';':
        Self.MakeSimple(TokenType::Semicolon, tk);
        break;
    case ':':
        Self.MakeSimple(TokenType::Colon, tk);
        break;
    case '=':
        if(Self.GetOffset(1) == '=')
            Self.MakeTwo(TokenType::Compare, tk);
        else
            Self.MakeSimple(TokenType::Equal, tk);
        break;
    case '!':
        if (Self.GetOffset(1) == '=')
            Self.MakeTwo(TokenType::NotEqual, tk);
        else
            Self.MakeSimple(TokenType::Not, tk);
        break;
    case '<':
        if (Self.GetOffset(1) == '=')
            Self.MakeTwo(TokenType::LessEqual, tk);
        else
            Self.MakeSimple(TokenType::Less, tk);
        break;
    case '>':
        if (Self.GetOffset(1) == '=')
            Self.MakeTwo(TokenType::GreaterEqual, tk);
        else
            Self.MakeSimple(TokenType::Greater, tk);
        break;
    case '+':
        if (Self.GetOffset(1) == '=')
            Self.MakeTwo(TokenType::PlusEqual, tk);
        else if (Self.GetOffset(1) == '+')
            Self.MakeTwo(TokenType::Inc, tk);
        else
            Self.MakeSimple(TokenType::Plus, tk);
        break;
    case '-':
        if (Self.GetOffset(1) == '=')
            Self.MakeTwo(TokenType::MinusEqual, tk);
        else if (Self.GetOffset(1) == '-')
            Self.MakeTwo(TokenType::Dec, tk);
        else
            Self.MakeSimple(TokenType::Minus, tk);
        break;
    case '*':
        if (Self.GetOffset(1) == '=')
            Self.MakeTwo(TokenType::StarEqual, tk);
        else
            Self.MakeSimple(TokenType::Star, tk);
        break;
    case '/':
        if (Self.GetOffset(1) == '=')
            Self.MakeTwo(TokenType::SlashEqual, tk);
        else if (Self.GetOffset(1) == '/') {
            while (Self.Current != '\n')
                Self.Advance();
            goto start;
        }
        else
            Self.MakeSimple(TokenType::Slash, tk);
        break;
    case '\"':
        tk = Self.GetString();
        break;
    case '\'':
        break;
    case '\0':
        Self.MakeSimple(TokenType::EndOfFile, tk);
        break;
    default:
        if (isalpha(Self.Current) || Self.Current == '_') {
            tk = Self.GetIdentifier();
        }
        else if (isdigit(Self.Current)) {
            tk = Self.GetDigit();
        }
        else {
            Self.ErrorStream.Println(
                STR("{s}:{u}: Error: Unexpected character {c}"), 
                Self.FileName, Self.Line, Self.Current
            );
            Self.IsPanicMode = true;
            Self.Advance();
        }
        break;
    }

    return tk;
}

