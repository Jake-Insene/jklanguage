#pragma once
#include "jkc/Lexer/Lexer.h"
#include "jkc/AST/Program.h"
#include "jkc/AST/Type.h"

enum AttributeType {
    AttribNone = 0,
    AttribPub = 0x1,
    AttribPriv = 0x2,
    AttribExtern = 0x4,
    AttribImport = 0x8,
    AttribExport = 0x10,
};

struct AttributeInfo {
    UInt16 Attributes;
    String Library;
};

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

    constexpr Parser(FILE* ErrorStream) :
        ErrorStream(ErrorStream), SourceLexer(SourceLexer) {}

    constexpr ~Parser() {}

    AST::Program ParseContent(const char* FileName, const StringView& Content);

    constexpr bool Success() const { return !IsPanicMode; }

    constexpr bool IsType() const {
        if (Current.Type == Type::TypeAny ||
            Current.Type == Type::TypeVoid ||
            Current.Type == Type::TypeByte ||
            Current.Type == Type::TypeInt ||
            Current.Type == Type::TypeUInt ||
            Current.Type == Type::TypeFloat)
            return true;

        return false;
    }

    constexpr bool IsTypeAttrib() const {
        if (Current.Type == Type::Const ||
            Current.Type == Type::Star ||
            Current.Type == Type::LeftKey)
            return true;

        return false;
    }

    constexpr bool IsAttribute(Type& Tk) {
        switch (Tk) {
        case Type::ExternAttr:
            return true;
        default:
            break;
        }

        return false;
    }

    void Advance();
    void ErrorAtCurrent(Str Format, ...);
    bool Expected(Type Type, Str Format, ...);
    void Syncronize();

    AST::TypeDecl                   ParseType();

    std::unique_ptr<AST::Expresion> ParseConstantValue(bool CanAssign, 
                                                   std::unique_ptr<AST::Expresion>);
    std::unique_ptr<AST::Expresion> ParseIdentifier(bool CanAssign, 
                                                std::unique_ptr<AST::Expresion>);
    std::unique_ptr<AST::Expresion> ParseGroup(bool CanAssign, 
                                           std::unique_ptr<AST::Expresion>);
    std::unique_ptr<AST::Expresion> ParseCall(bool CanAssign, 
                                          std::unique_ptr<AST::Expresion> Left);
    std::unique_ptr<AST::Expresion> ParseDot(bool CanAssign, 
                                         std::unique_ptr<AST::Expresion> Left);
    std::unique_ptr<AST::Expresion> ParseUnary(bool CanAssign, 
                                           std::unique_ptr<AST::Expresion>);
    std::unique_ptr<AST::Expresion> ParseBinaryOp(bool CanAssign, 
                                              std::unique_ptr<AST::Expresion> Left);
    std::unique_ptr<AST::Expresion> ParseArrayList(bool CanAssign, 
                                               std::unique_ptr<AST::Expresion>);
    std::unique_ptr<AST::Expresion> ParseArrayAccess(bool CanAssign, 
                                                 std::unique_ptr<AST::Expresion>);
    std::unique_ptr<AST::Expresion> ParseIncDec(bool CanAssign, 
                                                std::unique_ptr<AST::Expresion>);
    std::unique_ptr<AST::Expresion> ParseAssignment(bool CanAssign,
                                                std::unique_ptr<AST::Expresion>);

    std::unique_ptr<AST::Expresion> ParseExpresion(ParsePrecedence Precedence);
    std::unique_ptr<AST::Block>     ParseBlock();
    void                            ParseFunctionParameters(AST::Function& Function);
    std::unique_ptr<AST::Statement> ParseFunction(const AttributeInfo& Attribs);
    std::unique_ptr<AST::Statement> ParseReturn();
    std::unique_ptr<AST::Statement> ParseConstVal();
    std::unique_ptr<AST::Statement> ParseVar();
    std::unique_ptr<AST::If>        ParseIf();
    std::unique_ptr<AST::Statement> ParseExpresionStatement();
    std::unique_ptr<AST::Statement> ParseStatement();

    AttributeInfo ParseAttribs();

    struct {
        bool IsInFn = false;
        bool IsInReturn = false;
        UInt8 ReturnCount = 0;
    } Context;

    FILE* ErrorStream;
    
    Token Last{};
    Token Current{};
    Token Next{};
    Lexer SourceLexer;

    bool IsPanicMode = false;
    bool MustSyncronize = false;

    static constexpr UInt32 MaxParameterSize = 32;
};
