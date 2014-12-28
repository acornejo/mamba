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
    class Visitor;
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

            Node();
            virtual ~Node();
            virtual void accept(Visitor *v) = 0;
            void appendChild(Node *);
            void addString(std::string *);
            void extend(Node *);
    };

    class True: public Node {
        public:
            True(): Node() { }
            virtual void accept(Visitor *v);
    };

    class False: public Node {
        public:
            False(): Node() { }
            virtual void accept(Visitor *v);
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
            virtual void accept(Visitor *v);
    };

    class Real: public Const {
        public:
            real_t val;
            Real(real_t _val): Const(), val(_val) { }
            virtual void accept(Visitor *v);
    };


    class String: public Const {
        public:
            std::string *val;
            String(std::string *_val): Const(), val(_val) {
                addString(val);
            }
            virtual void accept(Visitor *v);
    };

    class Variable: public Const {
        public:
            bool write;
            std::string *val;
            Variable(std::string *_val): Const(), write(false), val(_val) {
                addString(val);
            }
            virtual void accept(Visitor *v);
    };

    class Binary: public Node {
        public:
            int type;
            Node *left, *right;
            Binary(int _type, Node *_left, Node *_right): Node(), type(_type), left(_left), right(_right) {
                appendChild(left);
                appendChild(right);
            }
            virtual void accept(Visitor *v);
    };

    class Unary: public Node {
        public:
            int type;
            Node *down;
            Unary(int _type, Node *_down): Node(), type(_type), down(_down) {
                appendChild(down);
            }
            virtual void accept(Visitor *v);
    };

    class And: public Node {
        public:
            Node *left, *right;
            And(Node *_left, Node *_right): Node(), left(_left), right(_right) {
                appendChild(left);
                appendChild(right);
            }
            virtual void accept(Visitor *v);
    };

    class Or: public Node {
        public:
            Node *left, *right;
            Or(Node *_left, Node *_right): Node(), left(_left), right(_right) {
                appendChild(left);
                appendChild(right);
            }
            virtual void accept(Visitor *v);
    };

    class ExprList: public Node {
        public:
            ExprList(): Node() { }
            virtual void accept(Visitor *v);
    };


    class StmtList: public Node {
        public:
            StmtList(): Node() { }
            virtual void accept(Visitor *v);
    };

    class Array: public Node {
        public:
            Node *elems;
            Array(Node *_elems): Node(), elems(_elems) {
                appendChild(elems);
            }
            virtual void accept(Visitor *v);
    };

    class Call: public Node {
        public:
            Node *parent, *params;
            Call(Node *_parent, Node *_params): Node(), parent(_parent), params(_params) {
                appendChild(parent);
                appendChild(params);
            }
            virtual void accept(Visitor *v);
    };

    class Subscript: public Node {
        public:
            Node *var, *idx;
            Subscript(Node *_var, Node *_idx): Node(), var(_var), idx(_idx) {
                appendChild(var);
                appendChild(idx);
            }
            virtual void accept(Visitor *v);
    };

    class Expr: public Node {
        public:
            Node *e;
            Expr(Node *_e): Node(), e(_e) {
                appendChild(e);
            }
            virtual void accept(Visitor *v);
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
            virtual void accept(Visitor *v);
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
            virtual void accept(Visitor *v);
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
            virtual void accept(Visitor *v);
    };

    class While: public Loop {
        public:
            Node *expr, *body;
            While(Node *_expr, Node *_body): Loop(), expr(_expr), body(_body) {
                appendChild(expr);
                appendChild(body);
            }
            virtual void accept(Visitor *v);
    };

    class Break: public Node {
        public:
            Break(): Node() { }
            virtual void accept(Visitor *v);
    };

    class Continue: public Node {
        public:
            Continue(): Node() { }
            virtual void accept(Visitor *v);
    };

    class Return: public Node {
        public:
            Node *e;
            Return(Node *_e): Node(), e(_e) {
                if (e)
                    appendChild(e);
            }
            virtual void accept(Visitor *v);
    };

    class Function: public Node {
        public:
            Node *params, *body, *ret;
            Function(Node* _params, Node *_body, Node *_ret): Node(), params(_params), body(_body), ret(_ret) {
                appendChild(params);
                appendChild(body);
                if (ret)
                    appendChild(ret);
            }
            virtual void accept(Visitor *v);
    };

    class TypeSpec: public Node {
        public:
            std::string *type_name;
            std::vector<int> array;
            bool pointer;
            TypeSpec(std::string *_type_name): Node(), type_name(_type_name) {
                addString(type_name);
            }
            void pushArray(const int size) {
                array.push_back(size);
            }
            virtual void accept(Visitor *v);
    };

    class TypeDecl: public Node {
        public:
            Node *type_spec;
            std::string *name;
            TypeDecl(Node *_type_spec, std::string *_name): Node(), type_spec(_type_spec), name(_name) {
                appendChild(type_spec);
                addString(name);
            }
            virtual void accept(Visitor *v);
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
            virtual void accept(Visitor *v);
    };

    class TypeDeclList: public Node {
        public:
            TypeDeclList(): Node() { }
            virtual void accept(Visitor *v);
    };

    class RecordDef: public Node {
        public:
            std::string *name;
            Node *decl_list;
            RecordDef(std::string *_name, Node *_decl_list): Node(), name(_name), decl_list(_decl_list) {
                addString(name);
                appendChild(decl_list);
            }
            virtual void accept(Visitor *v);
    };

    class TupleTypes: public Node {
        public:
            TupleTypes(): Node() { } 
            virtual void accept(Visitor *v);
    };

    class TupleDef: public Node {
        public:
            std::string *name;
            Node *type_list;
            TupleDef(std::string *_name, Node *_type_list): Node(), name(_name), type_list(_type_list) {
                addString(name);
                appendChild(type_list);
            }
            virtual void accept(Visitor *v);
    };

    class UnionItem: public Node {
        public:
            std::string *name;
            Node *type_spec;
            UnionItem(std::string *_name, Node *_type_spec): Node(), name(_name), type_spec(_type_spec) {
                addString(name);
                if (type_spec)
                    appendChild(type_spec);
            }
            virtual void accept(Visitor *v);
    };

    class UnionList: public Node {
        public:
            UnionList(): Node() { } 
            virtual void accept(Visitor *v);
    };

    class UnionDef: public Node {
        public:
            std::string *name;
            Node *type_list;
            UnionDef(std::string *_name, Node *_type_list): Node(), name(_name), type_list(_type_list) {
                addString(name);
                appendChild(type_list);
            }
            virtual void accept(Visitor *v);
    };

    class Visitor {
        public:
            virtual void visit(True *) = 0;
            virtual void visit(False *) = 0;
            virtual void visit(Integer *) = 0;
            virtual void visit(Real *) = 0;
            virtual void visit(String *) = 0;
            virtual void visit(Variable *) = 0;
            virtual void visit(Binary *) = 0;
            virtual void visit(Unary *) = 0;
            virtual void visit(And *) = 0;
            virtual void visit(Or *) = 0;
            virtual void visit(Array *) = 0;
            virtual void visit(Call *) = 0;
            virtual void visit(Subscript *) = 0;
            virtual void visit(Expr *) = 0;
            virtual void visit(Assign *) = 0;
            virtual void visit(IfElse *) = 0;
            virtual void visit(For *) = 0;
            virtual void visit(While *) = 0;
            virtual void visit(Break *) = 0;
            virtual void visit(Continue *) = 0;
            virtual void visit(Return *) = 0;
            virtual void visit(Function *) = 0;
            virtual void visit(TypeSpec *) = 0;
            virtual void visit(TypeDecl *) = 0;
            virtual void visit(Declaration *) = 0;
            virtual void visit(TypeDeclList *) = 0;
            virtual void visit(TupleTypes *) = 0;
            virtual void visit(UnionItem *) = 0;
            virtual void visit(UnionList *) = 0;
            virtual void visit(RecordDef *) = 0;
            virtual void visit(TupleDef *) = 0;
            virtual void visit(UnionDef *) = 0;
            virtual void visit(ExprList *) = 0;
            virtual void visit(StmtList *) = 0;
    };
}

#endif//__AST_H__
