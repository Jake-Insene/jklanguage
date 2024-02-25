#pragma once
#include "jkc/Lexer/Lexer.h"
#include "jkc/AST/Program.h"
#include "jkc/AST/Type.h"

struct Parser {
    enum class ParsePrecedence {
        None,
        Assignment,  // =
        Or,          // or
        And,         // and
        Equality,    // == !=
        Comparision,  // < > <= >=
        Term,        // + -
        Factor,      // * /
        Unary,       // ! -
        Call,        // . ()
        Primary
    };

    static Parser New(StreamOutput& ErrorStream) {
        return Parser{
            .ErrorStream = ErrorStream,
            .SourceLexer = Lexer::New(ErrorStream, nullptr, Slice<Char>()),
        };
    }

    void Destroy(this Parser& Self) {
        Self.SourceLexer.Destroy();
    }

    AST::Program ParseContent(Str FileName, Slice<Char> Content);

    constexpr bool Success() const { return !IsPanicMode; }

    constexpr bool IsType() const {
        if (Current.Type == TokenType::TypeAny ||
            Current.Type == TokenType::TypeVoid ||
            Current.Type == TokenType::TypeByte ||
            Current.Type == TokenType::TypeInt ||
            Current.Type == TokenType::TypeUInt ||
            Current.Type == TokenType::TypeFloat)
            return true;

        return false;
    }

    constexpr bool IsTypeAttrib() const {
        if (Current.Type == TokenType::Const ||
            Current.Type == TokenType::Star ||
            Current.Type == TokenType::LeftKey)
            return true;

        return false;
    }

    constexpr bool IsAttribute(Token& Tk) {
        if (Tk.Value.Str == STR("native")) {
            return true;
        }

        return false;
    }

    constexpr void Advance() {
        Last = std::move(Current);
        Current = std::move(Next);
        Next = SourceLexer.GetNext();

        if (!SourceLexer.Success()) {
            IsPanicMode = true;
        }
    }

    void ErrorAtCurrent(Str Format, ...);

    constexpr bool Expected(TokenType Type, Str Format, ...) {
        if (Current.Type != Type) {
            IsPanicMode = true;
            MustSyncronize = true;
            
            ErrorStream.Print(STR("{s}:{u}\n\tError: "), Current.Location.FileName, Current.Location.Line);
            va_list args;
            va_start(args, Format);
            ErrorStream.PrintlnVa(Format, args);
            va_end(args);

            return false;
        }
        Advance();
        return true;
    }

    constexpr void Syncronize() {
        while (Current.Type != TokenType::EndOfFile) {
            MustSyncronize = false;

            switch (Current.Type) {
            case TokenType::Fn:
            case TokenType::Return:
            case TokenType::Var:
                return;
            default:
                Advance();
                break;
            }
        }
    }

    AST::TypeDecl            ParseType();

    mem::Ptr<AST::Expresion> ParseConstantValue(List<Attribute> Attribs, bool CanAssign, mem::Ptr<AST::Expresion>);
    mem::Ptr<AST::Expresion> ParseIdentifier(List<Attribute> Attribs, bool CanAssign, mem::Ptr<AST::Expresion>);
    mem::Ptr<AST::Expresion> ParseGroup(List<Attribute> Attribs, bool CanAssign, mem::Ptr<AST::Expresion>);
    mem::Ptr<AST::Expresion> ParseCall(List<Attribute> Attribs, bool CanAssign, mem::Ptr<AST::Expresion> Left);
    mem::Ptr<AST::Expresion> ParseDot(List<Attribute> Attribs, bool CanAssign, mem::Ptr<AST::Expresion> Left);
    mem::Ptr<AST::Expresion> ParseUnary(List<Attribute> Attribs, bool CanAssign, mem::Ptr<AST::Expresion>);
    mem::Ptr<AST::Expresion> ParseBinaryOp(List<Attribute> Attribs, bool CanAssign, mem::Ptr<AST::Expresion> Left);
    mem::Ptr<AST::Expresion> ParseArrayList(List<Attribute> Attribs, bool CanAssign, mem::Ptr<AST::Expresion>);

    mem::Ptr<AST::Expresion> ParseExpresion(ParsePrecedence Precedence);
    mem::Ptr<AST::Block>     ParseBlock(List<Attribute> Attribs);
    void                     ParseFunctionParameters(mem::Ptr<AST::Function>& Function);
    mem::Ptr<AST::Statement> ParseFunction(List<Attribute> Attribs);
    mem::Ptr<AST::Statement> ParseReturn(List<Attribute> Attribs);
    mem::Ptr<AST::Statement> ParseConstVal(List<Attribute> Attribs);
    mem::Ptr<AST::Statement> ParseVar(List<Attribute> Attribs);
    mem::Ptr<AST::Statement> ParseIf(List<Attribute> Attribs);
    mem::Ptr<AST::Statement> ParseExpresionStatement(List<Attribute> Attribs);
    mem::Ptr<AST::Statement> ParseStatement();

    List<Attribute> ParseCompilerAttributes();

    StreamOutput& ErrorStream;
    
    Token Last{};
    Token Current{};
    Token Next{};
    Lexer SourceLexer;

    bool IsPanicMode = false;
    bool MustSyncronize = false;

    static constexpr UInt32 MaxParameterSize = 32;
};
