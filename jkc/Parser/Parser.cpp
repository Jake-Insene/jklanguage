#include "jkc/Parser/Parser.h"
#include "jkc/AST/Expresions.h"
#include "jkc/AST/Statements.h"
#include <stdarg.h>

using ParseExpresionFn = std::unique_ptr<AST::Expresion> (Parser::*)(bool, std::unique_ptr<AST::Expresion>);

struct ParseRule {
    Type Type;
    ParseExpresionFn Prefix;
    ParseExpresionFn Infix;
    Parser::ParsePrecedence Precedence;
};

static constexpr ParseRule Rules[] = {
    { Type::Unknown,           nullptr,                    nullptr,                  Parser::ParsePrecedence::None },
    { Type::Identifier,        &Parser::ParseIdentifier,   nullptr,                  Parser::ParsePrecedence::None },
    { Type::ConstInteger,      &Parser::ParseConstantValue,nullptr,                  Parser::ParsePrecedence::None },
    { Type::ConstUInteger,     &Parser::ParseConstantValue,nullptr,                  Parser::ParsePrecedence::None },
    { Type::ConstFloat,        &Parser::ParseConstantValue,nullptr,                  Parser::ParsePrecedence::None },
    { Type::ConstString,       &Parser::ParseConstantValue,nullptr,                  Parser::ParsePrecedence::None },
    { Type::Fn,                nullptr,                    nullptr,                  Parser::ParsePrecedence::None },
    { Type::Return,            nullptr,                    nullptr,                  Parser::ParsePrecedence::None },
    { Type::Var,               nullptr,                    nullptr,                  Parser::ParsePrecedence::None },
    { Type::Const,             nullptr,                    nullptr,                  Parser::ParsePrecedence::None },
    { Type::If,                nullptr,                    nullptr,                  Parser::ParsePrecedence::None },
    { Type::Elif,              nullptr,                    nullptr,                  Parser::ParsePrecedence::None },
    { Type::Else,              nullptr,                    nullptr,                  Parser::ParsePrecedence::None },
    { Type::For,               nullptr,                    nullptr,                  Parser::ParsePrecedence::None },
    { Type::While,             nullptr,                    nullptr,                  Parser::ParsePrecedence::None },
    { Type::TypeAny,           nullptr,                    nullptr,                  Parser::ParsePrecedence::None },
    { Type::TypeVoid,          nullptr,                    nullptr,                  Parser::ParsePrecedence::None },
    { Type::TypeByte,          nullptr,                    nullptr,                  Parser::ParsePrecedence::None },
    { Type::TypeInt,           nullptr,                    nullptr,                  Parser::ParsePrecedence::None },
    { Type::TypeUInt,          nullptr,                    nullptr,                  Parser::ParsePrecedence::None },
    { Type::TypeFloat,         nullptr,                    nullptr,                  Parser::ParsePrecedence::None },
    { Type::ExternAttr,        nullptr,                    nullptr,                  Parser::ParsePrecedence::None },
    { Type::Comma,             nullptr,                    nullptr,                  Parser::ParsePrecedence::None },
    { Type::Dot,               nullptr,                    &Parser::ParseDot,        Parser::ParsePrecedence::Call },
    { Type::LeftParent,        &Parser::ParseGroup,        &Parser::ParseCall,       Parser::ParsePrecedence::Call },
    { Type::RightParent,       nullptr,                    nullptr,                  Parser::ParsePrecedence::None },
    { Type::LeftBrace,         &Parser::ParseArrayList,    nullptr,                  Parser::ParsePrecedence::None },
    { Type::RightBrace,        nullptr,                    nullptr,                  Parser::ParsePrecedence::None },
    { Type::LeftKey,           nullptr,                    &Parser::ParseArrayAccess,Parser::ParsePrecedence::Call },
    { Type::RightKey,          nullptr,                    nullptr,                  Parser::ParsePrecedence::None },
    { Type::Semicolon,         nullptr,                    nullptr,                  Parser::ParsePrecedence::None },
    { Type::Colon,             nullptr,                    nullptr,                  Parser::ParsePrecedence::None },
    { Type::Equal,             nullptr,                    &Parser::ParseAssignment, Parser::ParsePrecedence::Primary },
    { Type::Compare,           nullptr,                    &Parser::ParseBinaryOp,   Parser::ParsePrecedence::Equality },
    { Type::Not,               &Parser::ParseUnary,        nullptr,                  Parser::ParsePrecedence::None },
    { Type::NotEqual,          nullptr,                    &Parser::ParseBinaryOp,   Parser::ParsePrecedence::Equality },
    { Type::Less,              nullptr,                    &Parser::ParseBinaryOp,   Parser::ParsePrecedence::Comparision },
    { Type::LessEqual,         nullptr,                    &Parser::ParseBinaryOp,   Parser::ParsePrecedence::Comparision },
    { Type::Greater,           nullptr,                    &Parser::ParseBinaryOp,   Parser::ParsePrecedence::Comparision },
    { Type::GreaterEqual,      nullptr,                    &Parser::ParseBinaryOp,   Parser::ParsePrecedence::Comparision },
    { Type::Plus,              nullptr,                    &Parser::ParseBinaryOp,   Parser::ParsePrecedence::Term },
    { Type::PlusEqual,         nullptr,                    &Parser::ParseBinaryOp,   Parser::ParsePrecedence::Term },
    { Type::Inc,               &Parser::ParseIncDec,       &Parser::ParseIncDec,     Parser::ParsePrecedence::Term },
    { Type::Minus,             &Parser::ParseUnary,        &Parser::ParseBinaryOp,   Parser::ParsePrecedence::Term },
    { Type::MinusEqual,        nullptr,                    &Parser::ParseBinaryOp,   Parser::ParsePrecedence::Term },
    { Type::Dec,               &Parser::ParseIncDec,       &Parser::ParseIncDec,     Parser::ParsePrecedence::Term },
    { Type::Star,              nullptr,                    &Parser::ParseBinaryOp,   Parser::ParsePrecedence::Factor },
    { Type::StarEqual,         nullptr,                    &Parser::ParseBinaryOp,   Parser::ParsePrecedence::Factor },
    { Type::Slash,             nullptr,                    &Parser::ParseBinaryOp,   Parser::ParsePrecedence::Factor },
    { Type::SlashEqual,        nullptr,                    &Parser::ParseBinaryOp,   Parser::ParsePrecedence::Factor },
    { Type::BinaryAnd,         nullptr,                    &Parser::ParseBinaryOp,   Parser::ParsePrecedence::Term },
    { Type::BinaryAndEqual,    nullptr,                    &Parser::ParseBinaryOp,   Parser::ParsePrecedence::Term },
    { Type::BinaryOr,          nullptr,                    &Parser::ParseBinaryOp,   Parser::ParsePrecedence::Term },
    { Type::BinaryOrEqual,     nullptr,                    &Parser::ParseBinaryOp,   Parser::ParsePrecedence::Term },
    { Type::BinaryXOr,         nullptr,                    &Parser::ParseBinaryOp,   Parser::ParsePrecedence::Term },
    { Type::BinaryXOrEqual,    nullptr,                    &Parser::ParseBinaryOp,   Parser::ParsePrecedence::Term },
    { Type::BinaryNAND,        &Parser::ParseUnary,        nullptr,                  Parser::ParsePrecedence::Term },
    { Type::BinaryShl,         nullptr,                    &Parser::ParseBinaryOp,   Parser::ParsePrecedence::Term },
    { Type::BinaryShlEqual,    nullptr,                    &Parser::ParseBinaryOp,   Parser::ParsePrecedence::Term },
    { Type::BinaryShr,         nullptr,                    &Parser::ParseBinaryOp,   Parser::ParsePrecedence::Term },
    { Type::BinaryShrEqual,    nullptr,                    &Parser::ParseBinaryOp,   Parser::ParsePrecedence::Term },
};

static constexpr ParseRule GetRule(Type Type) {
    return Rules[Int32(Type)];
}

//////////////////////////////////////////////////////////////////////////////////////////
// Utility
//////////////////////////////////////////////////////////////////////////////////////////

void Parser::Advance() {
    Last = std::move(Current);
    Current = std::move(Next);
    Next = SourceLexer.GetNext();

    if (!SourceLexer.Success()) {
        IsPanicMode = true;
    }
}


void Parser::ErrorAtCurrent(Str Format, ...) {
    fprintf(ErrorStream, "%s:%llu: Error: ", Current.Location.FileName, Current.Location.Line);
    va_list args;
    va_start(args, Format);
    vfprintf(ErrorStream, (const char*)Format, args);
    va_end(args);
    fputc('\n', ErrorStream);

    IsPanicMode = true;
    MustSyncronize = true;
}

bool Parser::Expected(Type Type, Str Format, ...) {
    if (Current.Type != Type) {
        IsPanicMode = true;
        MustSyncronize = true;

        fprintf(ErrorStream, "%s:%llu: Error: ", Current.Location.FileName, Current.Location.Line);
        va_list args;
        va_start(args, Format);
        vfprintf(ErrorStream, (const char*)Format, args);
        va_end(args);
        fputc('\n', ErrorStream);

        return false;
    }
    Advance();
    return true;
}

void Parser::Syncronize() {
    while (Current.Type != Type::EndOfFile) {
        MustSyncronize = false;

        switch (Current.Type) {
        case Type::Fn:
            return;
        case Type::Return:
            if (Context.IsInFn) {
                return;
            }
            break;
        case Type::Var:
            return;
        default:
            break;
        }

        Advance();
    }
}

AST::TypeDecl Parser::ParseType() {
    AST::TypeDecl type{};
    type.Primitive = AST::TypeDecl::Type::Unknown;

    while (IsType() || IsTypeAttrib()) {
        switch (Current.Type) {
        case Type::TypeAny:
            type.SizeInBits = 64;
            type.Primitive = AST::TypeDecl::Type::Any;
            Advance();
            break;
        case Type::TypeVoid:
            type.SizeInBits = 0;
            type.Primitive = AST::TypeDecl::Type::Void;
            Advance();
            break;
        case Type::TypeByte:
            type.SizeInBits = 8;
            type.Primitive = AST::TypeDecl::Type::Byte;
            Advance();
            break;
        case Type::TypeInt:
            type.SizeInBits = 64;
            type.Primitive = AST::TypeDecl::Type::Int;
            Advance();
            break;
        case Type::TypeUInt:
            type.SizeInBits = 64;
            type.Primitive = AST::TypeDecl::Type::UInt;
            Advance();
            break;
        case Type::TypeFloat:
            type.SizeInBits = 64;
            type.Primitive = AST::TypeDecl::Type::Float;
            Advance();
            break;
        case Type::Const:
            type.Flags |= AST::TypeDecl::Const;
            Advance();
            break;
        case Type::LeftKey:
            type.Flags |= AST::TypeDecl::Array;
            if (type.Primitive == AST::TypeDecl::Type::Unknown) {
                ErrorAtCurrent(u8"A type must to be before the left key");
            }
            Advance();

            if (Current.Type == Type::ConstInteger) {
                type.ArrayLen = (UInt32)Current.Value.Unsigned;
                Advance();
            }

            Expected(Type::RightKey, u8"']' was expected");
            break;
        }
    }

    return type;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Utility
//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
// Expresions
//////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<AST::Expresion> Parser::ParseConstantValue(bool, 
                                                       std::unique_ptr<AST::Expresion> /*RHS*/) {
    auto constVal = std::make_unique<AST::Constant>(
        Last.Location
    );

    if (Last.Type == Type::ConstInteger) {
        constVal->ValueType.Primitive = AST::TypeDecl::Type::Int;
        constVal->ValueType.SizeInBits = 64;
        constVal->Signed = Last.Value.Signed;
    }
    else if (Last.Type == Type::ConstFloat) {
        constVal->ValueType.Primitive = AST::TypeDecl::Type::Float;
        constVal->ValueType.SizeInBits = 64;
        constVal->Unsigned = Last.Value.Unsigned;
    }
    else if (Last.Type == Type::ConstUInteger) {
        constVal->ValueType.Primitive = AST::TypeDecl::Type::UInt;
        constVal->ValueType.SizeInBits = 64;
        constVal->Unsigned = Last.Value.Unsigned;
    }
    else if (Last.Type == Type::ConstString) {
        constVal->ValueType.Primitive = AST::TypeDecl::Type::Byte;
        constVal->ValueType.Flags |= AST::TypeDecl::Const;
        constVal->ValueType.Flags |= AST::TypeDecl::Array;
        
        constVal->ValueType.ArrayLen = (UInt32)Last.Value.Str.size();
        constVal->ValueType.SizeInBits = 8;

        constVal->String = std::move(Last.Value.Str);
    }

    return constVal;
}

std::unique_ptr<AST::Expresion> Parser::ParseIdentifier(bool /*CanAssign*/, 
                                                        std::unique_ptr<AST::Expresion>) {
    auto id = std::make_unique<AST::Identifier>(
        Last.Location
    );
    id->ID = Last.Value.StrRef;

    return id;
}

std::unique_ptr<AST::Expresion> Parser::ParseGroup(bool, std::unique_ptr<AST::Expresion>) {
    auto group = std::make_unique<AST::Group>(
        Last.Location
    );

    group->Value = ParseExpresion(ParsePrecedence::Assignment);
    Expected(Type::RightParent, u8"')' was expected");
    return group;
}

std::unique_ptr<AST::Expresion> Parser::ParseCall(bool, std::unique_ptr<AST::Expresion> Expresion) {
    auto call = std::make_unique<AST::Call>(
        SourceLocation(Current.Location)
    );

    call->Target = std::move(Expresion);

    Advance(); // (

    while (Current.Type != Type::RightParent && Current.Type != Type::EndOfFile) {
        std::unique_ptr<AST::Expresion>& expr = call->Arguments.emplace_back();
        expr = ParseExpresion(ParsePrecedence::Assignment);

        if (Current.Type != Type::RightParent) {
            Expected(Type::Comma, u8"',' was expected");
        }
    }
    Advance(); // )
    
    return call;
}

std::unique_ptr<AST::Expresion> Parser::ParseDot(bool, std::unique_ptr<AST::Expresion>) {
    auto dot = std::make_unique<AST::Dot>(
        SourceLocation(Current.Location)
    );
    Advance();
    return dot;
}

std::unique_ptr<AST::Expresion> Parser::ParseUnary(bool, std::unique_ptr<AST::Expresion>) {
    auto unary = std::make_unique<AST::Unary>(
        Last.Location
    );

    switch (Last.Type) {
    case Type::Minus: unary->Op = AST::UnaryOperation::Negate; break;
    case Type::Not: unary->Op = AST::UnaryOperation::LogicalNegate; break;
    case Type::BinaryNAND: unary->Op = AST::UnaryOperation::BinaryNAND; break;
    }

    unary->Value = ParseExpresion(ParsePrecedence::Assignment);

    return unary;
}

std::unique_ptr<AST::Expresion> Parser::ParseBinaryOp(bool, std::unique_ptr<AST::Expresion> Expresion) {
    auto binOp = std::make_unique<AST::BinaryOp>(
        Last.Location
    );

    binOp->Left = std::move(Expresion);

    switch (Current.Type) {
    case Type::Plus: binOp->Op = AST::BinaryOperation::Add; break;
    case Type::PlusEqual: binOp->Op = AST::BinaryOperation::AddEqual; break;
    case Type::Minus: binOp->Op = AST::BinaryOperation::Sub; break;
    case Type::MinusEqual: binOp->Op = AST::BinaryOperation::SubEqual; break;
    case Type::Star: binOp->Op = AST::BinaryOperation::Mul; break;
    case Type::StarEqual: binOp->Op = AST::BinaryOperation::MulEqual; break;
    case Type::Slash: binOp->Op = AST::BinaryOperation::Div; break;
    case Type::SlashEqual: binOp->Op = AST::BinaryOperation::DivEqual; break;
    case Type::Compare: binOp->Op = AST::BinaryOperation::Comparision; break;
    case Type::NotEqual: binOp->Op = AST::BinaryOperation::NotEqual; break;
    case Type::Less: binOp->Op = AST::BinaryOperation::Less; break;
    case Type::LessEqual: binOp->Op = AST::BinaryOperation::LessEqual; break;
    case Type::Greater: binOp->Op = AST::BinaryOperation::Greater; break;
    case Type::GreaterEqual: binOp->Op = AST::BinaryOperation::GreaterEqual; break;
    case Type::BinaryAnd: binOp->Op = AST::BinaryOperation::BinaryAnd; break;
    case Type::BinaryAndEqual: binOp->Op = AST::BinaryOperation::BinaryAndEqual; break;
    case Type::BinaryOr: binOp->Op = AST::BinaryOperation::BinaryOr; break;
    case Type::BinaryOrEqual: binOp->Op = AST::BinaryOperation::BinaryOrEqual; break;
    case Type::BinaryXOr: binOp->Op = AST::BinaryOperation::BinaryXOr; break;
    case Type::BinaryXOrEqual: binOp->Op = AST::BinaryOperation::BinaryXOrEqual; break;
    case Type::BinaryShl: binOp->Op = AST::BinaryOperation::BinaryShl; break;
    case Type::BinaryShlEqual: binOp->Op = AST::BinaryOperation::BinaryShlEqual; break;
    case Type::BinaryShr: binOp->Op = AST::BinaryOperation::BinaryShr; break;
    case Type::BinaryShrEqual: binOp->Op = AST::BinaryOperation::BinaryShrEqual; break;
    }

    ParseRule rule = GetRule(Current.Type);
    Advance();
    binOp->Right = ParseExpresion((ParsePrecedence)((UInt32)rule.Precedence + 1));

    return binOp;
}

std::unique_ptr<AST::Expresion> Parser::ParseArrayList(bool, std::unique_ptr<AST::Expresion>) {
    auto arrayList = std::make_unique<AST::ArrayList>(
        Last.Location
    );
    
    while (Current.Type != Type::RightBrace && Current.Type != Type::EndOfFile) {
        std::unique_ptr<AST::Expresion>& expr = arrayList->Elements.emplace_back();
        expr = ParseExpresion(ParsePrecedence::Assignment);

        if (Current.Type != Type::RightBrace) {
            Expected(Type::Comma, u8"',' was expected");
        }
    }

    Advance();
    return arrayList;
}

std::unique_ptr<AST::Expresion> Parser::ParseArrayAccess(bool /*CanAssign*/, 
                                                     std::unique_ptr<AST::Expresion> Expr) {
    auto arrayAccess = std::make_unique<AST::ArrayAccess>(
        Last.Location
    );

    Advance(); // [
    arrayAccess->Expr = std::move(Expr);
    arrayAccess->IndexExpr = ParseExpresion(ParsePrecedence::Assignment);
    Advance(); // ]

    return arrayAccess;
}

std::unique_ptr<AST::Expresion> Parser::ParseIncDec(bool /*CanAssign*/, 
                                                std::unique_ptr<AST::Expresion> Expr) {
    auto incDec = std::make_unique<AST::IncDec>(
        Last.Location
    );

    if (Last.Type == Type::Inc || Current.Type == Type::Inc) {
        if (Current.Type == Type::Inc)
            Advance();
        incDec->Increment = true;
    }
    else {
        if (Current.Type == Type::Dec)
            Advance();
        incDec->Increment = false;
    }

    if (Expr) { // Expr++
        incDec->Expr = std::move(Expr);
        incDec->After = true;
    }
    else { // ++Expr
        incDec->After = false;
        incDec->Expr = ParseExpresion(ParsePrecedence::Assignment);
    }

    return incDec;
}

std::unique_ptr<AST::Expresion> Parser::ParseAssignment(bool CanAssign,
                                                        std::unique_ptr<AST::Expresion> Expr) {
    auto assignment = std::make_unique<AST::Assignment>(
        Last.Location
    );


    assignment->Target = std::move(Expr);

    if (CanAssign) {
        Advance(); // =
        assignment->Source = ParseExpresion(ParsePrecedence::Or);
    }

    return assignment;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Expresions
//////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<AST::Expresion> Parser::ParseExpresion(ParsePrecedence Precedence) {
    std::unique_ptr<AST::Expresion> expr = {};

    ParseExpresionFn prefix = GetRule(Current.Type).Prefix;
    if (prefix == nullptr) {
        ErrorAtCurrent(u8"A expresion was expected");
        Advance();
        return expr;
    }

    Advance();
    bool canAssign = Precedence <= ParsePrecedence::Assignment;
    expr = (this->*prefix)(canAssign, nullptr);

    while (Precedence < (GetRule(Current.Type).Precedence)) {
        expr = (this->*GetRule(Current.Type).Infix)(canAssign, std::move(expr));
    }

    if (canAssign && Current.Type == Type::Equal) {
        ErrorAtCurrent(u8"Invalid assignment");
    }

    return expr;
}

std::unique_ptr<AST::Block> Parser::ParseBlock() {
    auto block = std::make_unique<AST::Block>(
        Current.Location
    );

    while (Current.Type != Type::RightBrace && Current.Type != Type::EndOfFile) {
        block->Statements.emplace_back(ParseStatement());
    }

    return block;
}

void Parser::ParseFunctionParameters(AST::Function& Function) {
    while (Current.Type != Type::RightParent && Current.Type != Type::EndOfFile) {
        Expected(Type::Identifier, u8"A identifier was expected");

        auto& param = Function.Parameters.emplace_back();
        param.Name = Last.Value.StrRef;

        Expected(Type::Colon, u8"':' was expected");

        param.Type = ParseType();
        if (param.Type.Primitive == AST::TypeDecl::Type::Unknown) {
            ErrorAtCurrent(u8"A type was expected");
            Advance();
        }

        if (Current.Type != Type::RightParent) {
            Expected(Type::Comma, u8"',' was expected");
        }
    }

    if (Function.Parameters.size() > MaxParameterSize) {
        ErrorAtCurrent(u8"%d is the max parameter size", MaxParameterSize);
    }
}

std::unique_ptr<AST::Statement> Parser::ParseFunction(const AttributeInfo& Attribs) {
    auto function = std::make_unique<AST::Function>(
        SourceLocation(Current.Location)
    );
    Advance(); // fn

    if (Attribs.Attributes & AttribExtern) {
        function->IsExtern = true;
        function->LibraryRef = std::move(Attribs.Library);
    }

    Expected(Type::Identifier, u8"A identifier was expected");
    function->Name = Last.Value.StrRef;

    Expected(Type::LeftParent, u8"'(' was expected");
    ParseFunctionParameters(*function);
    Expected(Type::RightParent, u8"')' was expected");

    if (Current.Type == Type::Semicolon || Current.Type == Type::LeftBrace) {
        function->FunctionType = AST::TypeDecl::Void();
    }
    else {
        function->FunctionType = ParseType();
        if (function->FunctionType.Primitive == AST::TypeDecl::Type::Unknown) {
            ErrorAtCurrent(u8"A type was expected");
            Advance();
        }
    }

    if (Current.Type == Type::Semicolon) {
        function->IsDefined = false;
        Advance();
    }
    else {
        if (function->IsExtern) {
            ErrorAtCurrent(u8"A native function can't be defined");
            goto return_fn;
        }

        if (!Expected(Type::LeftBrace, u8"'{' or ';' was expected")) {
            Advance();
            goto return_fn;
        }
        function->IsDefined = true;

        Context.IsInFn = true;
        function->Body = ParseBlock();
        Context.IsInFn = false;
        Expected(Type::RightBrace, u8"'}' was expected");

        if (Context.ReturnCount == 0) {
            if (function->FunctionType.IsVoid()) {
                function->Body->Statements.emplace_back(
                    std::make_unique<AST::Return>(Last.Location)
                );
            }
            else {
                ErrorAtCurrent(
                    u8"\'%s\' must to return a value",
                    function->Name.c_str()
                );
            }
        }
        else if (Context.ReturnCount > 1) {
            function->HasMultiReturn = true;
        }

        Context.ReturnCount = 0;
    }

return_fn:
    return function;
}

std::unique_ptr<AST::Statement> Parser::ParseReturn() {
    Context.ReturnCount++;

    auto ret = std::make_unique<AST::Return>(Current.Location);
    Advance();

    if (Current.Type == Type::Semicolon) {
        Advance();
    }
    else {
        Context.IsInReturn = true;
        ret->Value = ParseExpresion(ParsePrecedence::Assignment);
        Context.IsInReturn = false;
        Expected(Type::Semicolon, u8"';' was expected");
    }
    return ret;
}

std::unique_ptr<AST::Statement> Parser::ParseConstVal() {
    auto constVal = std::make_unique<AST::ConstVal>(
        Current.Location
    );
    Advance(); // const

    Expected(Type::Identifier, u8"A identifier was expected");
    constVal->Name = Last.Value.StrRef;
    
    Expected(Type::Colon, u8"':' was expected");
    constVal->ConstType = ParseType();
    if (constVal->ConstType.Primitive == AST::TypeDecl::Type::Unknown) {
        ErrorAtCurrent(u8"A type was expected");
        Advance();
    }

    Expected(Type::Equal, u8"A const must to be initialized with a value");
    constVal->Value = ParseExpresion(ParsePrecedence::Assignment);
    Expected(Type::Semicolon, u8"';' was expected");

    return constVal;
}

std::unique_ptr<AST::Statement> Parser::ParseVar() {
    auto var = std::make_unique<AST::Var>(Current.Location);
    Advance();

    Expected(Type::Identifier, u8"A identifier was expected");
    var->Name = Last.Value.StrRef;

    if (Current.Type != Type::Equal) {
        Expected(Type::Colon, u8"':' was expected");
        var->VarType = ParseType();
        if (var->VarType.Primitive == AST::TypeDecl::Type::Unknown) {
            ErrorAtCurrent(u8"A type was expected");
            Advance();
        }
    }
    else {
        var->VarType = AST::TypeDecl();
    }

    if (Current.Type == Type::Equal) {
        Advance();

        var->IsDefined = true;
        var->Value = ParseExpresion(ParsePrecedence::Assignment);
    }

    Expected(Type::Semicolon, u8"';' was expected");
    return var;
}

std::unique_ptr<AST::If> Parser::ParseIf() {
    auto _if = std::make_unique<AST::If>(
        Current.Location
    );
    Advance(); // if

    Expected(Type::LeftParent, u8"'(' was expected");
    _if->Expr = ParseExpresion(ParsePrecedence::Assignment);
    Expected(Type::RightParent, u8"')' was expected");

    Expected(Type::LeftBrace, u8"'{' was expected");
    _if->Body = ParseBlock();

    Expected(Type::RightBrace, u8"'}' was expected");

    if (Current.Type == Type::Elif) {
        _if->Elif = ParseIf();
    }
    else if (Current.Type == Type::Else) {
        Advance(); // else
        Expected(Type::LeftBrace, u8"'{' was expected");
        _if->ElseBlock = ParseBlock();
        Expected(Type::RightBrace, u8"'}' was expected");
    }

    return _if;
}

std::unique_ptr<AST::Statement> Parser::ParseExpresionStatement() {
    auto es = std::make_unique<AST::ExpresionStatement>(
        Current.Location
    );
    es->Value = ParseExpresion(ParsePrecedence::Assignment);
    Expected(Type::Semicolon, u8"';' was expected");
    return es;
}

std::unique_ptr<AST::Statement> Parser::ParseStatement() {
    std::unique_ptr<AST::Statement> statement = {};

    if (MustSyncronize) {
        Syncronize();
        if (Current.Type == Type::EndOfFile)
            return statement;
    }

    if (Current.Type == Type::Fn || IsAttribute(Current.Type)) {
        statement = ParseFunction(ParseAttribs());
    }
    else if (Current.Type == Type::Return) {
        if(Context.IsInFn)
        {
            statement = ParseReturn();
        }
        else {
            ErrorAtCurrent(u8"Invalid statement");
            return nullptr;
        }
    }
    else if (Current.Type == Type::Var) {
        statement = ParseVar();
    }
    else if (Current.Type == Type::Const) {
        statement = ParseConstVal();
    }
    else if (Current.Type == Type::If) {
        statement = ParseIf();
    }
    else if (Current.Type == Type::LeftBrace) {
        if (Context.IsInFn){
            auto es = std::make_unique<AST::ExpresionStatement>(
                Current.Location
            );
            Advance();
            es->Value = ParseBlock();
            Advance();
            statement = std::move(es);
        }
        else {
            ErrorAtCurrent(u8"Invalid statement");
            return nullptr;
        }
    }
    else {
        if (Context.IsInFn) {
            statement = ParseExpresionStatement();
        }
        else {
            ErrorAtCurrent(u8"Invalid statement");
            return nullptr;
        }
    }

    return statement;
}

AttributeInfo Parser::ParseAttribs() {
    AttributeInfo attribs = {};

    while (IsAttribute(Current.Type)) {
        Advance();

        switch (Last.Type) {
        case Type::ExternAttr:
            attribs.Attributes |= AttribExtern;
            if (!Expected(Type::ConstString, u8"A library name was expected")) {
                continue;
            }

            attribs.Library = std::move(Last.Value.Str);
            break;
        }
    }

    return attribs;
}

AST::Program Parser::ParseContent(const char* FileName, const StringView& Content) {
    String name = (Str)FileName;
    name = name.substr(0, name.find_last_of('.'));

    AST::Program program = AST::Program(name);
    SourceLexer.Set(FileName, Content);

    Advance();
    Advance();

    while (Current.Type != Type::EndOfFile) {
        program.Statements.emplace_back(ParseStatement());
    }

    return program;
}
