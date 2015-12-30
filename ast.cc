#include "ast.h"

namespace ast {

Node::Node(): parentNode(NULL), nextSibling(NULL), firstChild(NULL), lastChild(NULL), num_children(0) {
/* empty */
}

Node::~Node() {
    Node *n = firstChild;
    while (n != NULL) {
        Node *next = n->nextSibling;
        delete n;
        n = next;
    }
    childNodes.clear();
    for (size_t i = 0; i < slist.size(); i++)
        delete slist[i];
    slist.clear();
}

void Node::appendChild(Node *n) {
    if (firstChild != NULL) {
        lastChild->nextSibling = n;
    } else {
        firstChild = n;
    }
    lastChild = n;
    n->parentNode = this;
    n->nextSibling = NULL;
    num_children++;
    childNodes.push_back(n);
}

void Node::addString(std::string *s) {
    slist.push_back(s);
}

// void Node::prependChild(Node *n) {
//     childNodes.insert(childNodes.begin(), n);
//     n->nextSibling = firstChild;
//     n->parentNode = this;
//     firstChild = n;
// }

void Node::extend(Node *n) {
    Node *c = n->firstChild;
    while (c != NULL) {
        Node *next = c->nextSibling;
        appendChild(c);
        c = next;
    }
    n->firstChild = NULL;
    n->num_children = 0;
    delete n;
}

void True::accept(Visitor *v) { v->visit(this); }
void False::accept(Visitor *v) { v->visit(this); }
void Integer::accept(Visitor *v) { v->visit(this); }
void Real::accept(Visitor *v) { v->visit(this); }
void String::accept(Visitor *v) { v->visit(this); }
void Variable::accept(Visitor *v) { v->visit(this); }
void Binary::accept(Visitor *v) { v->visit(this); }
void Unary::accept(Visitor *v) { v->visit(this); }
void And::accept(Visitor *v) { v->visit(this); }
void Or::accept(Visitor *v) { v->visit(this); }
void Array::accept(Visitor *v) { v->visit(this); }
void Call::accept(Visitor *v) { v->visit(this); }
void Subscript::accept(Visitor *v) { v->visit(this); }
void Attribute::accept(Visitor *v) { v->visit(this); }
void Expr::accept(Visitor *v) { v->visit(this); }
void Assign::accept(Visitor *v) { v->visit(this); }
void IfElse::accept(Visitor *v) { v->visit(this); }
void For::accept(Visitor *v) { v->visit(this); }
void While::accept(Visitor *v) { v->visit(this); }
void Break::accept(Visitor *v) { v->visit(this); }
void Continue::accept(Visitor *v) { v->visit(this); }
void Return::accept(Visitor *v) { v->visit(this); }
void Function::accept(Visitor *v) { v->visit(this); }
void ArrayType::accept(Visitor *v) { v->visit(this); }
void RefType::accept(Visitor *v) { v->visit(this); }
void PtrType::accept(Visitor *v) { v->visit(this); }
void TupleType::accept(Visitor *v) { v->visit(this); }
void FuncType::accept(Visitor *v) { v->visit(this); }
void SimpleType::accept(Visitor *v) { v->visit(this); }
void TypeList::accept(Visitor *v) { v->visit(this); }
void VarDecl::accept(Visitor *v) { v->visit(this); }
void FuncDecl::accept(Visitor *v) { v->visit(this); }
void UnionItem::accept(Visitor *v) { v->visit(this); }
void UnionList::accept(Visitor *v) { v->visit(this); }
void RecordDef::accept(Visitor *v) { v->visit(this); }
void UnionDef::accept(Visitor *v) { v->visit(this); }
void ExprList::accept(Visitor *v) { v->visit(this); }
void StmtList::accept(Visitor *v) { v->visit(this); }

}
