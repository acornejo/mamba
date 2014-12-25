#ifndef __AST_H__
#define __AST_H__

#include <vector>
#include <map>
#include <string>

namespace ast {
    typedef int64_t integer_t;
    typedef uint64_t address_t;
    typedef double real_t;

    class Node;
    typedef std::vector<Node *> NodeList;

    class Node {
        public:
            Node *parentNode;
            Node *nextSibling;
            Node *firstChild;
            Node *lastChild;
            size_t num_children;
            NodeList childNodes;
            std::vector<std::string *> slist;

            Node(): parentNode(NULL), nextSibling(NULL), firstChild(NULL), lastChild(NULL), num_children(0) { }

            virtual ~Node();
            void appendChild(Node *);
            void addString(std::string *);
            void extend(Node *);
    };

    class True: public Node {
        public:
            True(): Node() { }
    };

    class False: public Node {
        public:
            False(): Node() { }
    };

    class Const: public Node {
        public:
            int idx;
            Const(): Node() { }
    };

    class Integer: public Const {
        public:
            integer_t val;
            Integer(integer_t _val): Const(), val(_val) { }
    };

    class Real: public Const {
        public:
            real_t val;
            Real(real_t _val): Const(), val(_val) { }
    };


    class String: public Const {
        public:
            std::string *val;
            String(std::string *_val): Const(), val(_val) {
                addString(val);
            }
    };

    class Variable: public Const {
        public:
            bool write;
            std::string *val;
            Variable(std::string *_val): Const(), write(false), val(_val) {
                addString(val);
            }
    };

    class Binary: public Node {
        public:
            int type;
            Node *left, *right;
            Binary(int _type, Node *_left, Node *_right): Node(), type(_type), left(_left), right(_right) {
                appendChild(left);
                appendChild(right);
            }
    };

    class Unary: public Node {
        public:
            int type;
            Node *down;
            Unary(int _type, Node *_down): Node(), type(_type), down(_down) {
                appendChild(down);
            }
    };

    class And: public Node {
        public:
            Node *left, *right;
            And(Node *_left, Node *_right): Node(), left(_left), right(_right) {
                appendChild(left);
                appendChild(right);
            }
    };

    class Or: public Node {
        public:
            Node *left, *right;
            Or(Node *_left, Node *_right): Node(), left(_left), right(_right) {
                appendChild(left);
                appendChild(right);
            }
    };

    class ExprList: public Node {
        public:
            ExprList(): Node() { }
    };

    class Array: public Node {
        public:
            Node *elems;
            Array(Node *_elems): Node(), elems(_elems) {
                appendChild(elems);
            }
    };

    class Call: public Node {
        public:
            Node *parent, *params;
            Call(Node *_parent, Node *_params): Node(), parent(_parent), params(_params) {
                appendChild(parent);
                appendChild(params);
            }
    };

    class Subscript: public Node {
        public:
            Node *var, *idx;
            Subscript(Node *_var, Node *_idx): Node(), var(_var), idx(_idx) {
                appendChild(var);
                appendChild(idx);
            }
    };

    class StmtList: public Node {
        public:
            StmtList(): Node() { }
    };


    class ExprStatement: public Node {
        public:
            Node *e;
            ExprStatement(Node *_e): Node(), e(_e) {
                appendChild(e);
            }
    };

    class Assign: public Node {
        public:
            Node *lval, *rval;
            int dups;
            Assign(Node *_lval, Node *_rval): Node(), lval(_lval), rval(_rval), dups(0) {
                if (Variable *v = dynamic_cast<Variable*>(lval))
                    v->write = true;
                if (Assign *a = dynamic_cast<Assign*>(rval))
                    a->chain();
                appendChild(lval);
                appendChild(rval);
            }

            void chain() { if (Assign *a=dynamic_cast<Assign*>(rval)) a->chain(); else dups++; }
    };

    class IfElse: public Node {
        public:
            Node *expr, *body, *ifelse;
            IfElse(Node *_expr, Node *_body, Node *_ifelse): Node(), expr(_expr), body(_body), ifelse(_ifelse) {
                appendChild(expr);
                appendChild(body);
                if (ifelse != NULL)
                    appendChild(ifelse);
            }
    };

    class Loop: public Node {
        public:
            address_t start_loop;
            std::vector<address_t> end_loop;
            Loop(): Node() { }
    };

    class For: public Loop {
        public:
            Node *var, *iterable, *body;
            For(Node *_var, Node *_iterable, Node *_body): Loop(), var(_var), iterable(_iterable), body(_body) {
                ((Variable*)var)->write = true;
                appendChild(var);
                appendChild(iterable);
                appendChild(body);
            }
    };

    class While: public Loop {
        public:
            Node *expr, *body;
            While(Node *_expr, Node *_body): Loop(), expr(_expr), body(_body) {
                appendChild(expr);
                appendChild(body);
            }
    };

    class Break: public Node {
        public:
            Break(): Node() { }
    };

    class Continue: public Node {
        public:
            Continue(): Node() { }
    };

    class Return: public Node {
        public:
            Node *e;
            Return(Node *_e): Node(), e(_e) {
                if (e)
                    appendChild(e);
            }
    };

    class TypeSpec: public Node {
        public:
            std::string *name;
            std::vector<int> array;
            bool pointer;
            TypeSpec(std::string *_name): Node(), name(_name) {
                addString(name);
            }
            void pushArray(const int size) {
                array.push_back(size);
            }
    };

    class FunctionExpr: public Node {
        public:
            Node *params, *body, *ret;
            FunctionExpr(Node* _params, Node *_body, Node *_ret): Node(), params(_params), body(_body), ret(_ret) {
                appendChild(params);
                appendChild(body);
                if (ret)
                    appendChild(ret);
            }
    };

    class TypeDecl: public Node {
        public:
            Node *type_spec;
            std::string *type_name;
            TypeDecl(Node *_type_spec, std::string *_type_name): Node(), type_spec(_type_spec), type_name(_type_name) {
                appendChild(type_spec);
                addString(type_name);
            }
    };

    class Declaration: public Node {
        public:
            std::string *name;
            Node *expr;
            Node *type_spec;
            Declaration(std::string *_name, Node *_expr, Node *_type_spec): Node(), name(_name), expr(_expr), type_spec(_type_spec) {
                addString(name);
                appendChild(expr);
                if (type_spec)
                    appendChild(type_spec);
            }
    };

    class TypeDeclList: public Node {
        public:
            TypeDeclList(): Node() { }
    };

    class RecordDef: public Node {
        public:
            std::string *type_name;
            Node *decl_list;
            RecordDef(std::string *_type_name, Node *_decl_list): Node(), type_name(_type_name), decl_list(_decl_list) {
                addString(type_name);
                appendChild(decl_list);
            }
    };

    class TupleTypes: public Node {
        public:
            TupleTypes(): Node() { } 
    };

    class TupleDef: public Node {
        public:
            std::string *type_name;
            Node *type_list;
            TupleDef(std::string *_type_name, Node *_type_list): Node(), type_name(_type_name), type_list(_type_list) {
                addString(type_name);
                appendChild(type_list);
            }
    };

    class UnionItem: public Node {
        public:
            std::string *type_name;
            Node *type_spec;
            UnionItem(std::string *_type_name, Node *_type_spec): Node(), type_name(_type_name), type_spec(_type_spec) {
                addString(type_name);
                if (type_spec)
                    appendChild(type_spec);
            }
    };

    class UnionList: public Node {
        public:
            UnionList(): Node() { } 
    };

    class UnionDef: public Node {
        public:
            std::string *type_name;
            Node *type_list;
            UnionDef(std::string *_type_name, Node *_type_list): Node(), type_name(_type_name), type_list(_type_list) {
                addString(type_name);
                appendChild(type_list);
            }
    };
}

#endif//__AST_H__
