#include "jkc/AST/Statement.h"
#include "jkc/AST/Statements.h"
#include "jkc/AST/Expresions.h"

namespace AST {

void Statement::Destroy(this Statement& Self) {
    Self.Attribs.Destroy();

    if (Self.Type == StatementType::Function) {
        Function* fn = Cast<Function*>(&Self);
        fn->Name.~basic_string();
        fn->Body.Destroy();
        fn->Parameters.Destroy();
    }
    else if (Self.Type == StatementType::Return) {
        Return* rt = Cast<Return*>(&Self);
        rt->Value.Destroy();
    }
    else if (Self.Type == StatementType::Var) {
        Var* v = Cast<Var*>(&Self);
        v->Name.~basic_string();
        v->Value.Destroy();
    }
    else if (Self.Type == StatementType::ConstVal) {
        ConstVal* cv = Cast<ConstVal*>(&Self);
        cv->Name.~basic_string();
        cv->Value.Destroy();
    }
    else if (Self.Type == StatementType::If) {
        If* _if = Cast<If*>(&Self);
        _if->Body.Destroy();
        _if->Elif.Destroy();
        _if->ElseBlock.Destroy();
        _if->Expr.Destroy();
    }
    else if (Self.Type == StatementType::ExpresionStatement) {
        ExpresionStatement* es = Cast<ExpresionStatement*>(&Self);
        es->Value.Destroy();
    }
}

}
