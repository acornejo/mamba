#ifndef __AST_H__
#define __AST_H__

#include <map>
#include <vector>
#include <string>
#include <stdint.h>

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

    /*
     * Type classes
     */

    class Type: public Node {
        public:
            virtual std::string type_name() const = 0;
    };

    class TypeList: public Type {
        public:
            std::vector<std::string*> names;
            std::vector<Type*> types;
            TypeList(): Type() { }
            virtual void accept(Visitor *v);
            void appendNamedChild(std::string *name, Type *type) {
                names.push_back(name);
                types.push_back(type);
                addString(name);
                appendChild(type);
            }
            virtual std::string type_name() const {
                std::string ret = "";
                if (!types.empty()) {
                    for (size_t i = 0; i < types.size(); i++) {
                        ret += types[i]->type_name();
                        if (i+1 < types.size())
                            ret += ",";
                    }
                }
                return ret;
            }
    };

    class SimpleType: public Type {
        public:
            std::string *tname;
            SimpleType(std::string *_tname): Type(), tname(_tname) {
                addString(tname);
            }
            virtual void accept(Visitor *v);
            virtual std::string type_name() const {
                return *tname;
            }
    };

    class RefType: public Type {
        public:
            Type *base_type;
            RefType(Type *_base_type): Type(), base_type(_base_type) {
                appendChild(base_type);
            }
            virtual void accept(Visitor *v);
            virtual std::string type_name() const {
                return "&" + base_type->type_name();
            }
    };

    class PtrType: public Type {
        public:
            Type *base_type;
            PtrType(Type *_base_type): Type(), base_type(_base_type) {
                appendChild(base_type);
            }
            virtual void accept(Visitor *v);
            virtual std::string type_name() const {
                return "*" + base_type->type_name();
            }
    };

    class ArrayType: public Type {
        public:
            Type *base_type;
            ArrayType(Type *_base_type): Type(), base_type(_base_type) {
                appendChild(base_type);
            }
            virtual void accept(Visitor *v);
            virtual std::string type_name() const {
                return "[" + base_type->type_name() + "]";
            }
    };

    class MapType: public Type {
        public:
            Type *key_type;
            Type *val_type;
            MapType(Type *_key_type, Type *_val_type):
              Type(), key_type(_key_type), val_type(_val_type) {
                appendChild(key_type);
                appendChild(val_type);
            }
            virtual void accept(Visitor *v);
            virtual std::string type_name() const {
                return "[" + key_type->type_name() + ":" + val_type->type_name() + "]";
            }
    };

    class TupleType: public Type {
        public:
            Type *base_type;
            TupleType(Type *_base_type): Type(), base_type(_base_type) {
                appendChild(base_type);
            }
            virtual void accept(Visitor *v);
            virtual std::string type_name() const {
                return "(" + base_type->type_name() + ")";
            }
    };

    class FuncType: public Type {
        public:
            TypeList *params;
            Type *ret;
            FuncType(TypeList *_params, Type *_ret): Type(), params(_params), ret(_ret) {
                appendChild(params);
                if (ret) appendChild(ret);
            }
            virtual void accept(Visitor *v);
            virtual std::string type_name() const {
                return "(" + params->type_name() + ")->" + (ret ? ret->type_name() : "");
            }
    };


    /*
     * Literals
     */

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

    /*
     * Expressions
     */

    class Variable: public Node {
        public:
            std::string *val;
            Variable(std::string *_val): Node(), val(_val) {
                addString(val);
            }
            virtual void accept(Visitor *v);
    };

    class Binary: public Node {
        public:
            int op;
            Node *left, *right;
            Binary(int _op, Node *_left, Node *_right): Node(), op(_op), left(_left), right(_right) {
                appendChild(left);
                appendChild(right);
            }
            virtual void accept(Visitor *v);
    };

    class Unary: public Node {
        public:
            int op;
            Node *down;
            Unary(int _op, Node *_down): Node(), op(_op), down(_down) {
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

    class Function: public Node {
        public:
            FuncType *proto;
            Node *body;
            Function(FuncType *_proto, Node *_body): Node(), proto(_proto), body(_body) {
                appendChild(proto);
                appendChild(body);
            }
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

    class Call: public Node {
        public:
            Node *parent, *params;
            Call(Node *_parent, Node *_params): Node(), parent(_parent), params(_params) {
                appendChild(parent);
                appendChild(params);
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

    class Subscript: public Node {
        public:
            Node *var, *idx;
            Subscript(Node *_var, Node *_idx): Node(), var(_var), idx(_idx) {
                appendChild(var);
                appendChild(idx);
            }
            virtual void accept(Visitor *v);
    };

    class Attribute: public Node {
        public:
            Node *var;
            std::string *aname;
            Attribute(Node *_var, std::string *_aname): Node (), var(_var), aname(_aname) {
                addString(aname);
                appendChild(var);
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
            Node *expr;
            NodeList vars;
            Assign(Node *var, Node *_expr): Node(), expr(_expr) {
                vars.push_back(var);
                appendChild(expr);
                appendChild(var);
            }

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
            Node *iterable, *body;
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

    class VarDecl: public Node {
        public:
            std::string *name;
            Node *expr;
            VarDecl(std::string *_name, Node *_expr): Node(), name(_name), expr(_expr) {
                addString(name);
                appendChild(expr);
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
            virtual void visit(VarDecl *) = 0;
            virtual void visit(FuncDecl *) = 0;
            virtual void visit(Assign *) = 0;
            virtual void visit(Unary *) = 0;
            virtual void visit(Binary *) = 0;
            virtual void visit(And *) = 0;
            virtual void visit(Or *) = 0;
            virtual void visit(IfElse *) = 0;
            virtual void visit(While *) = 0;
            virtual void visit(For *) = 0;
            virtual void visit(Break *) = 0;
            virtual void visit(Continue *) = 0;
            virtual void visit(Return *) = 0;
            virtual void visit(Function *) = 0;
            virtual void visit(Call *) = 0;
            virtual void visit(Array *) = 0;
            virtual void visit(Subscript *) = 0;
            virtual void visit(Attribute*) = 0;
            virtual void visit(Expr *) = 0;
            virtual void visit(UnionItem *) = 0;
            virtual void visit(UnionList *) = 0;
            virtual void visit(RecordDef *) = 0;
            virtual void visit(UnionDef *) = 0;
            virtual void visit(ExprList *) = 0;
            virtual void visit(StmtList *) = 0;
            virtual void visit(SimpleType *) = 0;
            virtual void visit(RefType *) = 0;
            virtual void visit(PtrType *) = 0;
            virtual void visit(ArrayType *) = 0;
            virtual void visit(MapType *) = 0;
            virtual void visit(TupleType *) = 0;
            virtual void visit(FuncType *) = 0;
            virtual void visit(TypeList *) = 0;
    };
}

#endif//__AST_H__
