#include "jkc/AST/Expresion.h"
#include "jkc/AST/Expresions.h"
#include "jkc/AST/Statements.h"

namespace AST {

void Expresion::Destroy(this Expresion& Self) {
    Self.Attribs.Destroy();

    if (Self.Type == ExpresionType::Constant) {
        Constant* c = Cast<Constant*>(&Self);
        c->String.~basic_string();
    }
    else if (Self.Type == ExpresionType::Identifier) {
        Identifier* id = Cast<Identifier*>(&Self);
        id->ID.~basic_string();
    }
    else if (Self.Type == ExpresionType::Group) {
        Group* g = Cast<Group*>(&Self);
        mem::Destroy(g->Value);
    }
    else if (Self.Type == ExpresionType::Call) {
        Call* c = Cast<Call*>(&Self);
        c->Arguments.Destroy();
        mem::Destroy(c->Target);
    }
    else if (Self.Type == ExpresionType::BinaryOp) {
        BinaryOp* binOp = Cast<BinaryOp*>(&Self);
        mem::Destroy(binOp->Left);
        mem::Destroy(binOp->Right);
    }
    else if (Self.Type == ExpresionType::Unary) {
        Unary* un = Cast<Unary*>(&Self);
        mem::Destroy(un->Value);
    }
    else if (Self.Type == ExpresionType::Dot) {
        Dot* d = Cast<Dot*>(&Self);
        mem::Destroy(d->Left);
        mem::Destroy(d->Right);
    }
    else if (Self.Type == ExpresionType::ArrayList) {
        ArrayList* arr = Cast<ArrayList*>(&Self);
        arr->Elements.Destroy();
    }
    else if (Self.Type == ExpresionType::Block) {
        Block* b = Cast<Block*>(&Self);
        b->Statements.Destroy();
    }
    else if (Self.Type == ExpresionType::ArrayAccess) {
        ArrayAccess* arr = Cast<ArrayAccess*>(&Self);
        mem::Destroy(arr->Expr);
        mem::Destroy(arr->IndexExpr);
    }

    mem::Deallocate(&Self);
}

}
