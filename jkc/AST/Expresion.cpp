#include "jkc/AST/Expresion.h"
#include "jkc/AST/Expresions.h"
#include "jkc/AST/Statements.h"

namespace AST {

void Expresion::Destroy(this Expresion& Self) {
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
        g->Value.Destroy();
    }
    else if (Self.Type == ExpresionType::Call) {
        Call* c = Cast<Call*>(&Self);
        c->Arguments.Destroy();
        c->Target.Destroy();
    }
    else if (Self.Type == ExpresionType::BinaryOp) {
        BinaryOp* binOp = Cast<BinaryOp*>(&Self);
        binOp->Left.Destroy();
        binOp->Right.Destroy();
    }
    else if (Self.Type == ExpresionType::Unary) {
        Unary* un = Cast<Unary*>(&Self);
        un->Value.Destroy();
    }
    else if (Self.Type == ExpresionType::Dot) {
        Dot* d = Cast<Dot*>(&Self);
        d->Left.Destroy();
        d->Right.Destroy();
    }
    else if (Self.Type == ExpresionType::ArrayList) {
        ArrayList* arr = Cast<ArrayList*>(&Self);
        arr->Elements.Destroy();
    }
    else if (Self.Type == ExpresionType::Block) {
        Block* b = Cast<Block*>(&Self);
        b->Statements.Destroy();
    }
}

}
