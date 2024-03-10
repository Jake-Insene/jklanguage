#include "jkc/Parser/Parser.h"
#include "jkc/AST/Expresions.h"
#include "jkc/AST/Statements.h"

#include <stdarg.h>

using ParseExpresionFn = AST::Expresion* (Parser::*)(List<Attribute>, bool, AST::Expresion*);

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
    { TokenType::LeftKey,       nullptr,                    &Parser::ParseArrayAccess,  Parser::ParsePrecedence::Call },
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

void Parser::ErrorAtCurrent(Str Format, ...) {
    ErrorStream.Print(STR("{s}:{u}: Error: "), Current.Location.FileName, Current.Location.Line);
    va_list args;
    va_start(args, Format);
    ErrorStream.PrintlnVa(Format, args);
    va_end(args);

    IsPanicMode = true;
    MustSyncronize = true;
}

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
                type.ArrayLen = (UInt32)Current.Value.Unsigned;
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

AST::Expresion* Parser::ParseConstantValue(List<Attribute> Attribs, bool, AST::Expresion* /*RHS*/) {
    auto constVal = mem::New<AST::Constant>(
        AST::ExpresionType::Constant, 
        SourceLocation(Last.Location.FileName, Last.Location.Line),
        Attribs
    );

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
        
        constVal->ValueType.ArrayLen = (UInt32)Last.Value.Str.size();
        constVal->ValueType.SizeInBits = 8;

        constVal->String = std::move(Last.Value.Str);
    }

    return Cast<AST::Expresion*>(constVal);
}

AST::Expresion* Parser::ParseIdentifier(List<Attribute> Attribs, bool /*CanAssign*/, AST::Expresion*) {
    auto id = mem::New<AST::Identifier>(
        AST::ExpresionType::Identifier,
        SourceLocation(Last.Location.FileName, Last.Location.Line),
        Attribs
    );
    id->ID = std::move(Last.Value.Str);
    return Cast<AST::Expresion*>(id);
}

AST::Expresion* Parser::ParseGroup(List<Attribute> Attribs, bool, AST::Expresion*) {
    auto group = mem::New<AST::Group>(
        AST::ExpresionType::Group,
        SourceLocation(Last.Location.FileName, Last.Location.Line),
        Attribs
    );

    group->Value = ParseExpresion(ParsePrecedence::Assignment);
    Expected(TokenType::RightParent, STR("')' was expected"));
    return Cast<AST::Expresion*>(group);
}

AST::Expresion* Parser::ParseCall(List<Attribute> Attribs, bool, AST::Expresion* Expresion) {
    auto call = mem::New<AST::Call>(
        AST::ExpresionType::Call,
        SourceLocation(Current.Location.FileName, Current.Location.Line),
        Attribs
    );

    call->Target = Expresion;

    Advance(); // (

    while (Current.Type != TokenType::RightParent && Current.Type != TokenType::EndOfFile) {
        AST::Expresion*& expr = call->Arguments.Push();
        expr = ParseExpresion(ParsePrecedence::Assignment);

        if (Current.Type != TokenType::RightParent) {
            Expected(TokenType::Comma, STR("',' was expected"));
        }
    }
    Advance(); // )
    
    return Cast<AST::Expresion*>(call);
}

AST::Expresion* Parser::ParseDot(List<Attribute> Attribs, bool, AST::Expresion*) {
    auto dot = mem::New<AST::Dot>(
        AST::ExpresionType::Dot,
        SourceLocation(Current.Location.FileName, Current.Location.Line),
        Attribs
    );
    Advance();
    return Cast<AST::Expresion*>(dot);
}

AST::Expresion* Parser::ParseUnary(List<Attribute> Attribs, bool, AST::Expresion*) {
    auto unary = mem::New<AST::Unary>(
        AST::ExpresionType::Unary,
        SourceLocation(Last.Location.FileName, Last.Location.Line),
        Attribs
    );

    switch (Last.Type) {
    case TokenType::Minus: unary->Op = AST::UnaryOperation::Negate; break;
    case TokenType::Not: unary->Op = AST::UnaryOperation::LogicalNegate; break;
    }

    unary->Value = ParseExpresion(ParsePrecedence::Assignment);

    return Cast<AST::Expresion*>(unary);
}

AST::Expresion* Parser::ParseBinaryOp(List<Attribute> Attribs, bool, AST::Expresion* Expresion) {
    auto binOp = mem::New<AST::BinaryOp>(
        AST::ExpresionType::BinaryOp,
        SourceLocation(Last.Location.FileName, Last.Location.Line),
        Attribs
    );

    binOp->Left = Expresion;

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
    binOp->Right = ParseExpresion((ParsePrecedence)((UInt32)rule.Precedence + 1));

    return Cast<AST::Expresion*>(binOp);
}

AST::Expresion* Parser::ParseArrayList(List<Attribute> Attribs, bool, AST::Expresion*) {
    auto arrayList = mem::New<AST::ArrayList>(
        AST::ExpresionType::ArrayList,
        SourceLocation(Last.Location.FileName, Last.Location.Line),
        Attribs
    );
    
    while (Current.Type != TokenType::RightBrace && Current.Type != TokenType::EndOfFile) {
        AST::Expresion*& expr = arrayList->Elements.Push();
        expr = ParseExpresion(ParsePrecedence::Assignment);

        if (Current.Type != TokenType::RightBrace) {
            Expected(TokenType::Comma, STR("',' was expected"));
        }
    }

    Advance();
    return Cast<AST::Expresion*>(arrayList);
}

AST::Expresion* Parser::ParseArrayAccess(List<Attribute> Attribs, bool /*CanAssign*/, AST::Expresion* Expr) {
    auto arrayAccess = mem::New<AST::ArrayAccess>(
        AST::ExpresionType::ArrayAccess,
        SourceLocation(Last.Location.FileName, Last.Location.Line),
        Attribs
    );

    Advance(); // [
    arrayAccess->Expr = Expr;
    arrayAccess->IndexExpr = ParseExpresion(ParsePrecedence::Assignment);
    Advance(); // ]

    return Cast<AST::Expresion*>(arrayAccess);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Expresions
//////////////////////////////////////////////////////////////////////////////////////////

AST::Expresion* Parser::ParseExpresion(ParsePrecedence Precedence) {
    AST::Expresion* expr = {};

    ParseExpresionFn prefix = ParserRuler::GetRule(Current).Prefix;
    Advance();
    if (prefix == nullptr) {
        ErrorAtCurrent(STR("A expresion was expected"));
        return expr;
    }

    bool canAssign = Precedence <= ParsePrecedence::Assignment;
    expr = (this->*prefix)({}, canAssign, nullptr);

    while (Precedence < ParserRuler::GetRule(Current).Precedence) {
        expr = (this->*ParserRuler::GetRule(Current).Infix)({}, canAssign, expr);
    }

    if (canAssign && Current.Type == TokenType::Equal) {
        ErrorAtCurrent(STR("Invalid assignment"));
    }

    return expr;
}

AST::Block* Parser::ParseBlock(List<Attribute> Attribs) {
    auto block = mem::New<AST::Block>(
        AST::ExpresionType::Block,
        SourceLocation(Current.Location.FileName, Current.Location.Line),
        Attribs
    );

    while (Current.Type != TokenType::RightBrace && Current.Type != TokenType::EndOfFile) {
        AST::Statement*& statement = block->Statements.Push();
        statement = ParseStatement();
    }

    return block;
}

void Parser::ParseFunctionParameters(AST::Function* Function) {
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
        ErrorAtCurrent(STR("{d} is the max parameter size"), MaxParameterSize);
    }
}

AST::Statement* Parser::ParseFunction(List<Attribute> Attribs) {
    auto function = mem::New<AST::Function>(
        AST::StatementType::Function, 
        SourceLocation(Current.Location.FileName, Current.Location.Line),
        Attribs
    );
    Advance(); // fn

    bool isNative = false;
    for (auto& attr : Attribs) {
        if (attr.Type == Attribute::Native)
            isNative = true;
    }

    Expected(TokenType::Identifier, STR("A identifier was expected"));
    function->Name = std::move(Last.Value.Str);

    Expected(TokenType::LeftParent, STR("'(' was expected"));
    ParseFunctionParameters(function);
    Expected(TokenType::RightParent, STR("')' was expected"));

    if (Current.Type == TokenType::Semicolon || Current.Type == TokenType::LeftBrace) {
        function->FunctionType = AST::TypeDecl::Void();
    }
    else {
        function->FunctionType = ParseType();
        if (function->FunctionType.Primitive == AST::TypeDecl::Type::Unknown) {
            ErrorAtCurrent(STR("A type was expected"));
            Advance();
        }
    }

    if (Current.Type == TokenType::Semicolon) {
        function->IsDefined = false;
        Advance();
    }
    else {
        if (isNative) {
            ErrorAtCurrent(STR("A native function can't be defined"));
            goto return_fn;
        }

        if (!Expected(TokenType::LeftBrace, STR("'{'}' or ';' was expected"))) {
            Advance();
            goto return_fn;
        }
        function->IsDefined= true;

        Context.IsInFn = true;
        function->Body = ParseBlock({});
        Context.IsInFn = false;
        Expected(TokenType::RightBrace, STR("'}' was expected"));

        if (Context.ReturnCount == 0) {
            if (function->FunctionType.IsVoid()) {
                AST::Statement*& statement = function->Body->Statements.Push();
                statement = mem::New<AST::Return>(
                    AST::StatementType::Return,
                    SourceLocation(Last.Location.FileName, Last.Location.Line),
                    Attribs
                );
            }
            else {
                ErrorAtCurrent(
                    STR("\'{s}\' must to return a value"),
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
    return Cast<AST::Statement*>(function);
}

AST::Statement* Parser::ParseReturn(List<Attribute> Attribs) {
    Context.ReturnCount++;

    auto ret = mem::New<AST::Return>(
        AST::StatementType::Return,
        SourceLocation(Current.Location.FileName, Current.Location.Line),
        Attribs
    );
    Advance();

    if (Current.Type == TokenType::Semicolon) {
        Advance();
    }
    else {
        Context.IsInReturn = true;
        ret->Value = ParseExpresion(ParsePrecedence::Assignment);
        Context.IsInReturn = false;
        Expected(TokenType::Semicolon, STR("';' was expected"));
    }
    return Cast<AST::Statement*>(ret);
}

AST::Statement* Parser::ParseConstVal(List<Attribute> Attribs) {
    auto constVal = mem::New<AST::ConstVal>(
        AST::StatementType::ConstVal, 
        SourceLocation(Current.Location.FileName, Current.Location.Line),
        Attribs
    );
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

    return Cast<AST::Statement*>(constVal);
}

AST::Statement* Parser::ParseVar(List<Attribute> Attribs) {
    auto var = mem::New<AST::Var>(
        AST::StatementType::Var,
        SourceLocation(Current.Location.FileName, Current.Location.Line),
        Attribs
    );
    Advance();

    Expected(TokenType::Identifier, STR("A identifier was expected"));
    var->Name = std::move(Last.Value.Str);

    if (Current.Type != TokenType::Equal)
    {
        Expected(TokenType::Colon, STR("':' was expected"));
        var->VarType = ParseType();
        if (var->VarType.Primitive == AST::TypeDecl::Type::Unknown) {
            ErrorAtCurrent(STR("A type was expected"));
            Advance();
        }
    }
    else {
        var->VarType = AST::TypeDecl();
    }

    if (Current.Type == TokenType::Equal) {
        Advance();

        var->IsDefined = true;
        var->Value = ParseExpresion(ParsePrecedence::Assignment);
    }

    Expected(TokenType::Semicolon, STR("';' was expected"));
    return Cast<AST::Statement*>(var);
}

AST::If* Parser::ParseIf(List<Attribute> Attribs) {
    auto _if = mem::New<AST::If>(
        AST::StatementType::If,
        SourceLocation(Current.Location.FileName, Current.Location.Line),
        Attribs
    );
    Advance(); // if

    Expected(TokenType::LeftParent, STR("'(' was expected"));
    _if->Expr = ParseExpresion(ParsePrecedence::Assignment);
    Expected(TokenType::RightParent, STR("')' was expected"));

    Expected(TokenType::LeftBrace, STR("'{' was expected"));
    AST::Block* bodyRef = nullptr;
    _if->Body = (bodyRef = ParseBlock({}));

    Expected(TokenType::RightBrace, STR("'}' was expected"));

    if (Current.Type == TokenType::Elif) {
        _if->Elif = ParseIf({});
    }
    else if (Current.Type == TokenType::Else) {
        Advance(); // else
        Expected(TokenType::LeftBrace, STR("'{' was expected"));
        _if->ElseBlock = ParseBlock({});
        Expected(TokenType::RightBrace, STR("'}' was expected"));
    }

    return _if;
}

AST::Statement* Parser::ParseExpresionStatement(List<Attribute> Attribs) {
    auto es = mem::New<AST::ExpresionStatement>(
        AST::StatementType::ExpresionStatement, 
        SourceLocation(Current.Location.FileName, Current.Location.Line),
        Attribs
    );
    es->Value = ParseExpresion(ParsePrecedence::Assignment);
    Expected(TokenType::Semicolon, STR("';' was expected"));
    return Cast<AST::Statement*>(es);
}

AST::Statement* Parser::ParseStatement() {
    AST::Statement* statement = {};

check:
    if (MustSyncronize) {
        Syncronize();
        if (Current.Type == TokenType::EndOfFile)
            return statement;
    }

    List<Attribute> Attribs = ParseCompilerAttributes();
    if (MustSyncronize)
        goto check;

    if (Current.Type == TokenType::Fn) {
        statement = ParseFunction(Attribs);
    }
    else if (Current.Type == TokenType::Return) {
        if(Context.IsInFn)
        {
            statement = ParseReturn(Attribs);
        }
        else {
            ErrorAtCurrent(STR("Invalid statement"));
            Attribs.Destroy();
            return nullptr;
        }
    }
    else if (Current.Type == TokenType::Var) {
        statement = ParseVar(Attribs);
    }
    else if (Current.Type == TokenType::Const) {
        statement = ParseConstVal(Attribs);
    }
    else if (Current.Type == TokenType::If) {
        statement = ParseIf(Attribs);
    }
    else if (Current.Type == TokenType::LeftBrace) {
        if (Context.IsInFn){
            auto es = mem::New<AST::ExpresionStatement>(
                AST::StatementType::ExpresionStatement,
                SourceLocation(Current.Location.FileName, Current.Location.Line),
                Attribs
            );
            Advance();
            es->Value = Cast<AST::Expresion*>(ParseBlock({}));
            Advance();
            statement = Cast<AST::Statement*>(es);
        }
        else {
            ErrorAtCurrent(STR("Invalid statement"));
            Attribs.Destroy();
            return nullptr;
        }
    }
    else {
        if (Context.IsInFn) {
            statement = ParseExpresionStatement(Attribs);
        }
        else {
            ErrorAtCurrent(STR("Invalid statement"));
            Attribs.Destroy();
            return nullptr;
        }
    }

    return statement;
}

List<Attribute> Parser::ParseCompilerAttributes() {
    List<Attribute> attribs = {};

    while (Current.Type == TokenType::CompilerAttribute) {
        if (Current.Value.Str == STR("Native")) {
            auto& attrib = attribs.Push();
            attrib.Type = Attribute::Native;
            if (Next.Type != TokenType::LeftParent) {
                Advance();
                continue;
            }

            Advance(); // CompilerAttribute
            Advance(); // (

            if (!Expected(TokenType::ConstString, STR("A string was expected"))) {
                continue;
            }

            attrib.S = std::move(Current.Value.Str);
        }
        else {
            ErrorAtCurrent(STR("Unknown compiler attribute '{s}'"), Current.Value.Str.c_str());
        }
    }

    return attribs;
}

AST::Program Parser::ParseContent(Str FileName, Slice<Char> Content) {
    std::u8string name = FileName;
    name = name.substr(0, name.find_last_of('.'));

    AST::Program program = AST::Program::New(std::move(name));
    SourceLexer.Set(FileName, Content);

    Advance();
    Advance();

    while (Current.Type != TokenType::EndOfFile) {
        auto statement = ParseStatement();
        AST::Statement*& programStat = program.Statements.Push();
        programStat = statement;
    }

    return program;
}
