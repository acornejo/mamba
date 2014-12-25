#include "ast.h"

namespace ast {

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

}
