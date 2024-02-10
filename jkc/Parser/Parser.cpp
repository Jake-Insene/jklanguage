#include "jkc/Parser/Parser.h"
#include "jkc/AST/Expresions.h"
#include "jkc/AST/Statements.h"

#include <stdarg.h>

using ParseExpresionFn = mem::Ptr<AST::Expresion> (Parser::*)(bool, mem::Ptr<AST::Expresion>);

struct ParseRule {
    TokenType TokenType;
    ParseExpresionFn Prefix;
    ParseExpresionFn Infix;
    Parser::ParsePrecedence Precedence;
};

class ParserRuler {
public:
    static constexpr ParseRule GetRule(const Token& Tk) {
        for (auto& rule : Rules) {
            if (rule.TokenType == Tk.Type) {
                return rule;
            }
        }

        return ParseRule();
    }

static constexpr ParseRule Rules[] = {
    { TokenType::Identifier,    &Parser::ParseIdentifier,   nullptr,                    Parser::ParsePrecedence::None },
    { TokenType::ConstInteger,  &Parser::ParseConstantValue,nullptr,                    Parser::ParsePrecedence::None },
    { TokenType::ConstUInteger, &Parser::ParseConstantValue,nullptr,                    Parser::ParsePrecedence::None },
    { TokenType::ConstFloat,    &Parser::ParseConstantValue,nullptr,                    Parser::ParsePrecedence::None },
    { TokenType::ConstString,   &Parser::ParseConstantValue,nullptr,                    Parser::ParsePrecedence::None },
    { TokenType::Dot,           nullptr,                    &Parser::ParseDot,          Parser::ParsePrecedence::Call },
    { TokenType::LeftParent,    &Parser::ParseGroup,        &Parser::ParseCall,         Parser::ParsePrecedence::Call },
    { TokenType::LeftBrace,     &Parser::ParseArrayList,    nullptr,                    Parser::ParsePrecedence::None },
    { TokenType::Compare,       nullptr,                    &Parser::ParseBinaryOp,     Parser::ParsePrecedence::Equality },
    { TokenType::Not,           &Parser::ParseUnary,        nullptr,                    Parser::ParsePrecedence::None },
    { TokenType::NotEqual,      nullptr,                    &Parser::ParseBinaryOp,     Parser::ParsePrecedence::Equality },
    { TokenType::Less,          nullptr,                    &Parser::ParseBinaryOp,     Parser::ParsePrecedence::Comparision },
    { TokenType::LessEqual,     nullptr,                    &Parser::ParseBinaryOp,     Parser::ParsePrecedence::Comparision },
    { TokenType::Greater,       nullptr,                    &Parser::ParseBinaryOp,     Parser::ParsePrecedence::Comparision },
    { TokenType::Equal,         nullptr,                    &Parser::ParseBinaryOp,     Parser::ParsePrecedence::Assignment },
    { TokenType::Plus,          nullptr,                    &Parser::ParseBinaryOp,     Parser::ParsePrecedence::Term },
    { TokenType::Plus,          nullptr,                    &Parser::ParseBinaryOp,     Parser::ParsePrecedence::Term },
    { TokenType::PlusEqual,     nullptr,                    &Parser::ParseBinaryOp,     Parser::ParsePrecedence::Term },
    { TokenType::Inc,           nullptr,                    &Parser::ParseBinaryOp,     Parser::ParsePrecedence::Term },
    { TokenType::Minus,         &Parser::ParseUnary,        &Parser::ParseBinaryOp,     Parser::ParsePrecedence::Term },
    { TokenType::MinusEqual,    nullptr,                    &Parser::ParseBinaryOp,     Parser::ParsePrecedence::Term },
    { TokenType::Dec,           nullptr,                    &Parser::ParseBinaryOp,     Parser::ParsePrecedence::Term },
    { TokenType::Star,          nullptr,                    &Parser::ParseBinaryOp,     Parser::ParsePrecedence::Factor },
    { TokenType::StarEqual,     nullptr,                    &Parser::ParseBinaryOp,     Parser::ParsePrecedence::Factor },
    { TokenType::Slash,         nullptr,                    &Parser::ParseBinaryOp,     Parser::ParsePrecedence::Factor },
    { TokenType::SlashEqual,    nullptr,                    &Parser::ParseBinaryOp,     Parser::ParsePrecedence::Factor },
};

};

//////////////////////////////////////////////////////////////////////////////////////////
// Variables
//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
// Utility
//////////////////////////////////////////////////////////////////////////////////////////

AST::TypeDecl Parser::ParseType() {
    AST::TypeDecl type{};
    type.Primitive = AST::TypeDecl::Type::Unknown;

    while (IsType() || IsTypeAttrib()) {
        switch (Current.Type) {
        case TokenType::TypeAny:
            type.SizeInBits = 64;
            type.Primitive = AST::TypeDecl::Type::Any;
            Advance();
            break;
        case TokenType::TypeVoid:
            type.SizeInBits = 0;
            type.Primitive = AST::TypeDecl::Type::Void;
            Advance();
            break;
        case TokenType::TypeByte:
            type.SizeInBits = 8;
            type.Primitive = AST::TypeDecl::Type::Byte;
            Advance();
            break;
        case TokenType::TypeInt:
            type.SizeInBits = 64;
            type.Primitive = AST::TypeDecl::Type::Int;
            Advance();
            break;
        case TokenType::TypeUInt:
            type.SizeInBits = 64;
            type.Primitive = AST::TypeDecl::Type::UInt;
            Advance();
            break;
        case TokenType::TypeFloat:
            type.SizeInBits = 64;
            type.Primitive = AST::TypeDecl::Type::Float;
            Advance();
            break;
        case TokenType::Const:
            type.Flags |= AST::TypeDecl::Const;
            Advance();
            break;
        case TokenType::Star:
            type.Flags |= AST::TypeDecl::Ptr;
            type.PointerDeep++;
            if (type.Primitive != AST::TypeDecl::Type::Unknown) {
                ErrorAtCurrent(STR("A type must to be before the star"));
            }
            Advance();
            break;
        case TokenType::LeftKey:
            type.Flags |= AST::TypeDecl::Array;
            if (type.Primitive != AST::TypeDecl::Type::Unknown) {
                ErrorAtCurrent(STR("A type must to be before the left key"));
            }
            Advance();

            if (Current.Type == TokenType::ConstInteger) {
                type.ArrayLen = (Uint32)Current.Value.Unsigned;
                Advance();
            }
            else {
                type.Flags |= AST::TypeDecl::Ptr;
            }

            Expected(TokenType::RightKey, STR("']' was expected"));
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

mem::Ptr<AST::Expresion> Parser::ParseConstantValue(bool, mem::Ptr<AST::Expresion> /*RHS*/) {
    auto constVal = mem::New<AST::Constant>(AST::ExpresionType::Constant, Last.Location.FileName, Last.Location.Line);

    if (Last.Type == TokenType::ConstInteger) {
        constVal->ValueType.Primitive = AST::TypeDecl::Type::Int;
        constVal->ValueType.SizeInBits = 64;
        constVal->Signed = Last.Value.Signed;
    }
    else if (Last.Type == TokenType::ConstFloat) {
        constVal->ValueType.Primitive = AST::TypeDecl::Type::Float;
        constVal->ValueType.SizeInBits = 64;
        constVal->Unsigned = Last.Value.Unsigned;
    }
    else if (Last.Type == TokenType::ConstUInteger) {
        constVal->ValueType.Primitive = AST::TypeDecl::Type::UInt;
        constVal->ValueType.SizeInBits = 64;
        constVal->Unsigned = Last.Value.Unsigned;
    }
    else if (Last.Type == TokenType::ConstString) {
        constVal->ValueType.Primitive = AST::TypeDecl::Type::Byte;
        constVal->ValueType.Flags |= AST::TypeDecl::Const;
        constVal->ValueType.Flags |= AST::TypeDecl::Array;
        constVal->ValueType.Flags |= AST::TypeDecl::Ptr;
        
        constVal->ValueType.ArrayLen = (Uint32)Last.Value.Str.size();
        constVal->ValueType.SizeInBits = 8;

        constVal->String = std::move(Last.Value.Str);
    }

    return mem::Ptr<AST::Expresion>::New(constVal.Data);
}

mem::Ptr<AST::Expresion> Parser::ParseIdentifier(bool /*CanAssign*/, mem::Ptr<AST::Expresion>) {
    auto id = mem::New<AST::Identifier>(AST::ExpresionType::Identifier, Last.Location.FileName, Last.Location.Line);
    id->ID = std::move(Last.Value.Str);
    return mem::Ptr<AST::Expresion>::New(id.Data);
}

mem::Ptr<AST::Expresion> Parser::ParseGroup(bool, mem::Ptr<AST::Expresion>) {
    auto group = mem::New<AST::Group>(AST::ExpresionType::Group, Last.Location.FileName, Last.Location.Line);

    group->Value = ParseExpresion(ParsePrecedence::Assignment);
    Expected(TokenType::RightParent, STR("')' was expected"));
    return mem::Ptr<AST::Expresion>::New(group.Data);
}

mem::Ptr<AST::Expresion> Parser::ParseCall(bool, mem::Ptr<AST::Expresion> Expresion) {
    auto call = mem::New<AST::Call>(AST::ExpresionType::Call, Current.Location.FileName, Current.Location.Line);

    call->Target = std::move(Expresion);

    Advance(); // (

    while (Current.Type != TokenType::RightParent && Current.Type != TokenType::EndOfFile) {
        call->Arguments.Push(nullptr) = std::move(ParseExpresion(ParsePrecedence::Assignment));

        if (Current.Type != TokenType::RightParent) {
            Expected(TokenType::Comma, STR("',' was expected"));
        }
    }
    Advance(); // )
    
    return mem::Ptr<AST::Expresion>::New(call.Data);
}

mem::Ptr<AST::Expresion> Parser::ParseDot(bool, mem::Ptr<AST::Expresion>) {
    auto dot = mem::New<AST::Dot>(AST::ExpresionType::Dot, Current.Location.FileName, Current.Location.Line);
    Advance();
    return mem::Ptr<AST::Expresion>::New(dot.Data);
}

mem::Ptr<AST::Expresion> Parser::ParseUnary(bool, mem::Ptr<AST::Expresion>) {
    auto unary = mem::New<AST::Unary>(AST::ExpresionType::Unary, Last.Location.FileName, Last.Location.Line);

    switch (Last.Type) {
    case TokenType::Minus: unary->Op = AST::UnaryOperation::Negate; break;
    case TokenType::Not: unary->Op = AST::UnaryOperation::LogicalNegate; break;
    }

    unary->Value = ParseExpresion(ParsePrecedence::Assignment);

    return mem::Ptr<AST::Expresion>::New(unary.Data);
}

mem::Ptr<AST::Expresion> Parser::ParseBinaryOp(bool, mem::Ptr<AST::Expresion> Expresion) {
    auto binOp = mem::New<AST::BinaryOp>(AST::ExpresionType::BinaryOp, Last.Location.FileName, Last.Location.Line);

    binOp->Left = std::move(Expresion);

    switch (Current.Type) {
    case TokenType::Equal: binOp->Op = AST::BinaryOperation::Assignment; break;
    case TokenType::Plus: binOp->Op = AST::BinaryOperation::Add; break;
    case TokenType::PlusEqual: binOp->Op = AST::BinaryOperation::AddEqual; break;
    case TokenType::Inc: binOp->Op = AST::BinaryOperation::Inc; break;
    case TokenType::Minus: binOp->Op = AST::BinaryOperation::Sub; break;
    case TokenType::MinusEqual: binOp->Op = AST::BinaryOperation::SubEqual; break;
    case TokenType::Dec: binOp->Op = AST::BinaryOperation::Dec; break;
    case TokenType::Star: binOp->Op = AST::BinaryOperation::Mul; break;
    case TokenType::StarEqual: binOp->Op = AST::BinaryOperation::MulEqual; break;
    case TokenType::Slash: binOp->Op = AST::BinaryOperation::Div; break;
    case TokenType::SlashEqual: binOp->Op = AST::BinaryOperation::DivEqual; break;
    case TokenType::Compare: binOp->Op = AST::BinaryOperation::Comparision; break;
    case TokenType::NotEqual: binOp->Op = AST::BinaryOperation::NotEqual; break;
    case TokenType::Less: binOp->Op = AST::BinaryOperation::Less; break;
    case TokenType::LessEqual: binOp->Op = AST::BinaryOperation::LessEqual; break;
    case TokenType::Greater: binOp->Op = AST::BinaryOperation::Greater; break;
    case TokenType::GreaterEqual: binOp->Op = AST::BinaryOperation::GreaterEqual; break;
    }

    ParseRule rule = ParserRuler::GetRule(Current);

    Advance();
    binOp->Right = ParseExpresion((ParsePrecedence)((Uint32)rule.Precedence + 1));

    return mem::Ptr<AST::Expresion>::New(binOp.Data);
}

mem::Ptr<AST::Expresion> Parser::ParseArrayList(bool, mem::Ptr<AST::Expresion>) {
    auto arrayList = mem::New<AST::ArrayList>(AST::ExpresionType::ArrayList, Last.Location.FileName, Last.Location.Line);
    
    while (Current.Type != TokenType::RightBrace && Current.Type != TokenType::EndOfFile) {
        arrayList->Elements.Push(nullptr) = std::move(ParseExpresion(ParsePrecedence::Assignment));

        if (Current.Type != TokenType::RightBrace) {
            Expected(TokenType::Comma, STR("',' was expected"));
        }
    }

    Advance();
    return mem::Ptr<AST::Expresion>::New(arrayList.Data);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Expresions
//////////////////////////////////////////////////////////////////////////////////////////

mem::Ptr<AST::Expresion> Parser::ParseExpresion(ParsePrecedence Precedence) {
    mem::Ptr<AST::Expresion> expr = {};

    ParseExpresionFn prefix = ParserRuler::GetRule(Current).Prefix;
    Advance();
    if (prefix == nullptr) {
        ErrorAtCurrent(STR("A expresion was expected"));
        return expr;
    }

    bool canAssign = Precedence <= ParsePrecedence::Assignment;
    expr = (this->*prefix)(canAssign, mem::Ptr<AST::Expresion>());

    while (Precedence < ParserRuler::GetRule(Current).Precedence) {
        expr = (this->*ParserRuler::GetRule(Current).Infix)(canAssign, std::move(expr));
    }

    if (canAssign && Current.Type == TokenType::Equal) {
        ErrorAtCurrent(STR("Invalid assignment"));
    }

    return expr;
}

mem::Ptr<AST::Block> Parser::ParseBlock() {
    auto block = mem::New<AST::Block>(AST::ExpresionType::Block, Current.Location.FileName, Current.Location.Line);

    while (Current.Type != TokenType::RightBrace && Current.Type != TokenType::EndOfFile) {
        mem::Ptr<AST::Statement> statement = ParseStatement();

        if (statement->Type == AST::StatementType::Return) {
            block->HasReturn = true;
        }
        else if (statement->Type == AST::StatementType::ExpresionStatement) {
            if (auto es = (AST::ExpresionStatement*)statement.Data) {
                if (es->Value->Type == AST::ExpresionType::Block) {
                    block->HasReturn = ((AST::Block*)es->Value.Data)->HasReturn;
                }
            }
        }

        block->Statements.Push(nullptr) = std::move(statement);
    }

    return block;
}

void Parser::ParseFunctionParameters(mem::Ptr<AST::Function>& Function) {
    while (Current.Type != TokenType::RightParent && Current.Type != TokenType::EndOfFile) {
        Expected(TokenType::Identifier, STR("A identifier was expected"));

        auto& param = Function->Parameters.Push();
        param.Name = std::move(Last.Value.Str);

        Expected(TokenType::Colon, STR("':' was expected"));

        param.Type = ParseType();
        if (param.Type.Primitive == AST::TypeDecl::Type::Unknown) {
            ErrorAtCurrent(STR("A type was expected"));
            Advance();
        }

        if (Current.Type != TokenType::RightParent) {
            Expected(TokenType::Comma, STR("',' was expected"));
        }
    }

    if (Function->Parameters.Size > MaxParameterSize) {
        ErrorAtCurrent(STR("{u} is the max parameter size"), (UInt)MaxParameterSize);
    }
}

mem::Ptr<AST::Statement> Parser::ParseFunction() {
    auto function = mem::New<AST::Function>(AST::StatementType::Function, Current.Location.FileName, Current.Location.Line);
    Advance(); // fn

    Expected(TokenType::Identifier, STR("A identifier was expected"));
    function->Name = std::move(Last.Value.Str);

    Expected(TokenType::LeftParent, STR("'(' was expected"));
    ParseFunctionParameters(function);
    Expected(TokenType::RightParent, STR("')' was expected"));

    function->FunctionType = ParseType();
    if (function->FunctionType.Primitive == AST::TypeDecl::Type::Unknown) {
        ErrorAtCurrent(STR("A type was expected"));
        Advance();
    }

    ParseAttributes(function);

    if (Current.Type == TokenType::Semicolon) {
        function->IsDefined = false;
        Advance();
    }
    else {
        if (function->IsNative) {
            ErrorAtCurrent(STR("A native function can't be defined"));
            goto return_fn;
        }

        if (!Expected(TokenType::LeftBrace, STR("'{' or ';' was expected"))) {
            Advance();
            goto return_fn;
        }
        function->IsDefined= true;

        function->Body = ParseBlock();
        Expected(TokenType::RightBrace, STR("'}' was expected"));

        if (!function->Body->HasReturn) {
            if (function->FunctionType.IsVoid()) {
                function->Body->Statements.Push(nullptr) = mem::Ptr<AST::Statement>::New(
                    mem::New<AST::Return>(
                        AST::StatementType::Return, Last.Location.FileName, Last.Location.Line
                    ).Data
                );
            }
            else {
                ErrorAtCurrent(
                    STR("\'{s}\' must to return a value"),
                    function->Name.c_str()
                );
            }
        }
    }

return_fn:
    return mem::Ptr<AST::Statement>::New(function.Data);
}

mem::Ptr<AST::Statement> Parser::ParseReturn() {
    auto ret = mem::New<AST::Return>(AST::StatementType::Return, Current.Location.FileName, Current.Location.Line);
    Advance();

    if (Current.Type == TokenType::Semicolon) {
        Advance();
    }
    else {
        ret->Value = ParseExpresion(ParsePrecedence::Assignment);
        Expected(TokenType::Semicolon, STR("';' was expected"));
    }
    return mem::Ptr<AST::Statement>::New(ret.Data);
}

mem::Ptr<AST::Statement> Parser::ParseConstVal() {
    auto constVal = mem::New<AST::ConstVal>(AST::StatementType::ConstVal, Current.Location.FileName, Current.Location.Line);
    Advance(); // const

    Expected(TokenType::Identifier, STR("A identifier was expected"));
    constVal->Name = std::move(Last.Value.Str);
    
    Expected(TokenType::Colon, STR("':' was expected"));
    constVal->ConstType = ParseType();
    if (constVal->ConstType.Primitive == AST::TypeDecl::Type::Unknown) {
        ErrorAtCurrent(STR("A type was expected"));
        Advance();
    }

    Expected(TokenType::Equal, STR("A const must to be initialized with a value"));
    constVal->Value = ParseExpresion(ParsePrecedence::Assignment);
    Expected(TokenType::Semicolon, STR("';' was expected"));

    return mem::Ptr<AST::Statement>::New(constVal.Data);
}

mem::Ptr<AST::Statement> Parser::ParseVar() {
    auto var = mem::New<AST::Var>(AST::StatementType::Var, Current.Location.FileName, Current.Location.Line);
    Advance();

    Expected(TokenType::Identifier, STR("A identifier was expected"));
    var->Name = std::move(Last.Value.Str);

    Expected(TokenType::Colon, STR("':' was expected"));

    var->VarType = ParseType();
    if (var->VarType.Primitive == AST::TypeDecl::Type::Unknown) {
        ErrorAtCurrent(STR("A type was expected"));
        Advance();
    }

    if (Current.Type == TokenType::Equal) {
        Advance();

        var->IsDefined = true;
        var->Value = ParseExpresion(ParsePrecedence::Assignment);
    }

    Expected(TokenType::Semicolon, STR("';' was expected"));
    return mem::Ptr<AST::Statement>::New(var.Data);
}

mem::Ptr<AST::Statement> Parser::ParseIf() {
    auto _if = mem::New<AST::If>(AST::StatementType::If, Current.Location.FileName, Current.Location.Line);
    Advance(); // if

    Expected(TokenType::LeftParent, STR("'(' was expected"));
    _if->Expr = ParseExpresion(ParsePrecedence::Assignment);
    Expected(TokenType::RightParent, STR("')' was expected"));

    Expected(TokenType::LeftBrace, STR("'{' was expected"));
    _if->Body = mem::Ptr<AST::Expresion>::New(ParseBlock().Data);
    Expected(TokenType::RightBrace, STR("'}' was expected"));

    if (Current.Type == TokenType::Elif) {
        _if->Elif = std::move(ParseIf());
    }
    else if (Current.Type == TokenType::Else) {
        Advance(); // else
        Expected(TokenType::LeftBrace, STR("'{' was expected"));
        _if->ElseBlock = mem::Ptr<AST::Expresion>::New(ParseBlock().Data);
        Expected(TokenType::RightBrace, STR("'}' was expected"));
    }

    return mem::Ptr<AST::Statement>::New(_if.Data);
}

mem::Ptr<AST::Statement> Parser::ParseExpresionStatement() {
    auto es = mem::New<AST::ExpresionStatement>(
        AST::StatementType::ExpresionStatement, Current.Location.FileName, Current.Location.Line
    );
    es->Value = ParseExpresion(ParsePrecedence::Assignment);
    Expected(TokenType::Semicolon, STR("';' was expected"));
    return mem::Ptr<AST::Statement>::New(es.Data);
}

mem::Ptr<AST::Statement> Parser::ParseStatement() {
    mem::Ptr<AST::Statement> statement = {};

    if (MustSyncronize) {
        Syncronize();
    }

    if (Current.Type == TokenType::Fn) {
        statement = ParseFunction();
    }
    else if (Current.Type == TokenType::Return) {
        statement = ParseReturn();
    }
    else if (Current.Type == TokenType::Var) {
        statement = ParseVar();
    }
    else if (Current.Type == TokenType::Const) {
        statement = ParseConstVal();
    }
    else if (Current.Type == TokenType::If) {
        statement = ParseIf();
    }
    else if (Current.Type == TokenType::LeftBrace) {
        auto es = mem::New<AST::ExpresionStatement>(
            AST::StatementType::ExpresionStatement, Current.Location.FileName, Current.Location.Line
        );
        Advance();
        es->Value = mem::Ptr<AST::Expresion>(ParseBlock().Data);
        Advance();
        statement = mem::Ptr<AST::Statement>(es.Data);
    }
    else {
        statement = ParseExpresionStatement();
    }

    return statement;
}

void Parser::ParseAttributes(mem::Ptr<AST::Function>& Function) {
    if (IsAttribute(Current)) {
        Advance();
        if (Last.Value.Str == STR("native")) {
            Function->IsNative = true;
            Expected(TokenType::ConstString, STR("A library name was expected"));
            Function->NativeInfo.Library = std::move(Last.Value.Str);
            Expected(TokenType::ConstString, STR("A entry name was expected"));
            Function->NativeInfo.Entry = std::move(Last.Value.Str);
        }
    }
}

AST::Program Parser::ParseContent(Str FileName, Slice<Char> Content) {
    std::u8string name = FileName;
    name = name.substr(0, name.find_last_of('.'));

    AST::Program program = AST::Program::New(name.c_str());
    SourceLexer.Set(FileName, Content);

    Advance();
    Advance();

    while (Current.Type != TokenType::EndOfFile) {
        auto statement = ParseStatement();
        program.Statements.Push(nullptr) = std::move(statement);
    }

    return program;
}
