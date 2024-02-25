#include "jkc/Lexer/Lexer.h"

struct IdentifierInfo {
    UInt32 Len;
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

        {.Len = 3, .Str = "Any", .Type = TokenType::TypeAny },
        {.Len = 4, .Str = "Void", .Type = TokenType::TypeVoid },
        {.Len = 4, .Str = "Byte", .Type = TokenType::TypeByte },
        {.Len = 4, .Str = "Bool", .Type = TokenType::TypeByte },
        {.Len = 3, .Str = "Int", .Type = TokenType::TypeInt },
        {.Len = 4, .Str = "UInt", .Type = TokenType::TypeUInt },
        {.Len = 5, .Str = "Float", .Type = TokenType::TypeFloat },
};

static inline void FindIdentifier(Token& Tk) {
    for (UInt32 i = 0; i < std::size(Identifiers); i++) {
        if (Identifiers[i].Len != Tk.Value.Str.size())
            continue;

        if (memcmp(Identifiers[i].Str, Tk.Value.Str.c_str(), Identifiers[i].Len) == 0) {
            Tk.Type = Identifiers[i].Type;
            return;
        }
    }

    Tk.Type = TokenType::Identifier;
}

Token Lexer::GetIdentifier(this Lexer& Self) {
    Token tk{};
    tk.Type = TokenType::Unknown;
    tk.Location = SourceLocation(Self.FileName, Self.Line);

    tk.Value.Str = {};
    while (isalnum(Self.Current) || Self.Current == '_') {
        tk.Value.Str.push_back(Self.Current);
        Self.Advance();
    }

    FindIdentifier(tk);
    return tk;
}

Token Lexer::GetDigit(this Lexer& Self) {
    Token tk{};
    tk.Type = TokenType::ConstInteger;
    tk.Location = SourceLocation(Self.FileName, Self.Line);
    Int32 base = 10;

    std::string digit{};
loop:
    while (isdigit(Self.Current)) {
        digit.push_back(Self.Current);
        Self.Advance();
    }

    if (Self.Current == '.') {
        tk.Type = TokenType::ConstFloat;
        Self.Advance();
        digit.push_back('.');
        goto loop;
    }
    else if (Self.Current == 'U') {
        tk.Type = TokenType::ConstUInteger;
        Self.Advance();
    }

    if (tk.Type == TokenType::ConstFloat)
        tk.Value.Real = std::stod(digit);
    else if (tk.Type == TokenType::ConstUInteger)
        tk.Value.Unsigned = std::stoull(digit, NULL, base);
    else
        tk.Value.Signed = std::stoll(digit, NULL, base);
    return tk;
}

Token Lexer::GetString(this Lexer& Self) {
    Token tk{};
    tk.Type = TokenType::ConstString;
    tk.Location = SourceLocation(Self.FileName, Self.Line);

    Self.Advance();

    tk.Value.Str = {};
    while (Self.Current != '\"' && Self.Current != '\0') {
        if (!Self.ParseScapeSequence(tk.Value.Str)) {
            tk.Value.Str.push_back(Self.Current);
            Self.Advance();
        }
    }

    if (Self.Current == '\0') {
        Self.ErrorStream.Println(STR("{s}:{u}: Error: Bad string"), Self.FileName, Self.Line);
        Self.IsPanicMode = true;
    }

    Self.Advance();

    return tk;
}

bool Lexer::ParseScapeSequence(this Lexer& Self, std::u8string& Str) {
    if (Self.Current == '\\') {
        Self.Advance();
        switch (Self.Current) {
        case '0': Str.push_back('\0'); break;
        case 'n': Str.push_back('\n'); break;
        case 'r': Str.push_back('\r'); break;
        case 't': Str.push_back('\t'); break;
        default:
            Self.ErrorStream.Println(STR("{s}:{u}: Warn: Unknown scape sequence"), Self.FileName, Self.Line);
            break;
        }

        Self.Advance();
        return true;
    }

    return false;
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
    case '#':
        Self.Advance(); // Skip # and get the id
        tk = Self.GetIdentifier();
        tk.Type = TokenType::CompilerAttribute;
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

