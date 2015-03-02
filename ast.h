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

    class Integer: public Node{
        public:
            integer_t val;
            Integer(integer_t _val): Node(), val(_val) { }
            virtual void accept(Visitor *v);
    };

    class Real: public Node {
        public:
            real_t val;
            Real(real_t _val): Node(), val(_val) { }
            virtual void accept(Visitor *v);
    };

    class String: public Node {
        public:
            std::string *val;
            String(std::string *_val): Node(), val(_val) {
                addString(val);
            }
            virtual void accept(Visitor *v);
    };

    class Variable: public Node {
        public:
            bool write;
            std::string *val;
            Variable(std::string *_val): Node(), write(false), val(_val) {
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
            std::string *vname;
            Node *var, *iterable, *body;
            For(std::string *_vname, Node *_iterable, Node *_body): Loop(), vname(_vname), iterable(_iterable), body(_body) {
                addString(vname);
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

    class ReferenceType: public Node {
        public:
            Node *type_spec;
            ReferenceType(Node *_type_spec): Node(), type_spec(_type_spec) {
                appendChild(type_spec);
            }
            virtual void accept(Visitor *v);
    };

    class PointerType: public Node {
        public:
            Node *type_spec;
            PointerType(Node *_type_spec): Node(), type_spec(_type_spec) {
                appendChild(type_spec);
            }
            virtual void accept(Visitor *v);
    };

    class ArrayType: public Node {
        public:
            Node *type_spec;
            size_t size;
            ArrayType(Node *_type_spec, size_t _size): Node(), type_spec(_type_spec), size(_size) {
                appendChild(type_spec);
            }
            virtual void accept(Visitor *v);
    };

    class TupleType: public Node {
        public:
            Node *type_spec;
            TupleType(Node *_type_spec): Node(), type_spec(_type_spec) {
                appendChild(type_spec);
            }
            virtual void accept(Visitor *v);
    };

    class FuncType: public Node {
        public:
            Node *in_types;
            Node *out_type;
            FuncType(Node *_in_types, Node *_out_type): Node(), in_types(_in_types), out_type(_out_type) {
                appendChild(in_types);
                if (out_type)
                    appendChild(out_type);
            }
            virtual void accept(Visitor *v);
    };

    class SimpleType: public Node {
        public:
            std::string *type_name;
            SimpleType(std::string *_type_name): Node(), type_name(_type_name) {
                addString(type_name);
            }
            virtual void accept(Visitor *v);
    };

    class TypeList: public Node {
        public:
            TypeList(): Node() { }
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

    class FuncDecl: public Node {
        public:
            std::string *name;
            Node *func;
            FuncDecl(std::string *_name, Node *_func): Node(), name(_name), func(_func) {
                addString(name);
                appendChild(func);
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
            virtual void visit(TypeDecl *) = 0;
            virtual void visit(Declaration *) = 0;
            virtual void visit(FuncDecl *) = 0;
            virtual void visit(TypeDeclList *) = 0;
            virtual void visit(UnionItem *) = 0;
            virtual void visit(UnionList *) = 0;
            virtual void visit(RecordDef *) = 0;
            virtual void visit(UnionDef *) = 0;
            virtual void visit(ExprList *) = 0;
            virtual void visit(StmtList *) = 0;
            virtual void visit(SimpleType *) = 0;
            virtual void visit(ReferenceType *) = 0;
            virtual void visit(PointerType *) = 0;
            virtual void visit(ArrayType *) = 0;
            virtual void visit(TupleType *) = 0;
            virtual void visit(FuncType *) = 0;
            virtual void visit(TypeList *) = 0;
    };
}

#endif//__AST_H__
