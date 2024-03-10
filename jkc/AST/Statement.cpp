#include "jkc/AST/Statement.h"
#include "jkc/AST/Statements.h"
#include "jkc/AST/Expresions.h"

namespace AST {

void Statement::Destroy(this Statement& Self) {
    Self.Attribs.Destroy();

    if (Self.Type == StatementType::Function) {
        Function* fn = Cast<Function*>(&Self);
        fn->Name.~basic_string();
        fn->Parameters.Destroy();
        mem::Destroy(fn->Body);
    }
    else if (Self.Type == StatementType::Return) {
        Return* rt = Cast<Return*>(&Self);
        mem::Destroy(rt->Value);
    }
    else if (Self.Type == StatementType::Var) {
        Var* v = Cast<Var*>(&Self);
        v->Name.~basic_string();
        mem::Destroy(v->Value);
    }
    else if (Self.Type == StatementType::ConstVal) {
        ConstVal* cv = Cast<ConstVal*>(&Self);
        cv->Name.~basic_string();
        mem::Destroy(cv->Value);
    }
    else if (Self.Type == StatementType::If) {
        If* _if = Cast<If*>(&Self);
        mem::Destroy(_if->Body);
        mem::Destroy(_if->Expr);
        mem::Destroy(_if->Elif);
        mem::Destroy(_if->ElseBlock);
    }
    else if (Self.Type == StatementType::ExpresionStatement) {
        ExpresionStatement* es = Cast<ExpresionStatement*>(&Self);
        es->Value->Destroy();
    }
}

}
