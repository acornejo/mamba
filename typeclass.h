#ifndef _TYPECLASS_H__
#define _TYPECLASS_H__

class TypeClass {
protected:
    std::map<std::string,> func_list;
    std::string signature;

public:
    bool has_function(std::string func_name, std::vector<TypeClass*> params) const;
    void apply_function(std::string func_name);
    std::string get_signature() const;
};

class FuncTypeClass: public TypeClass {
protected:
    TypeClass *return_type;

public:
    std::string get_name() const;
    size_t get_num_params() const;
    TypeClass *get_nth_param(size_t i);
    TypeClass *get_return_type();
};

class BoolExpr {
private:
    bool val;
public:
    BoolExpr(bool _val): Expr("Bool"), val(_val) {}
    Value *value() {
        return ConstantInt::get(getGlobalContext(), APInt(val ? 1 : 0));
    }
};

class IntExpr {
private:
    int val;
public:
    IntExpr(int _val): Expr("Int"), val(_val) {}
    Value *value() {
        return ConstnatInt::get(getGlobalContext(), APInt(val));
    }
};

class FloatExpr {
private:
    float val;
public:
    IntExpr(int _val): Expr("Float"), val(_val) {}
    Value *value() {
        return ConstnatInt::get(getGlobalContext(), APFloat(val));
    }
};

class StringExpr {
private:
    std::string val;
public:
    StringExpr(int _val): Expr("String"), val(_val) {}
    Value *value() {
        return ConstantArray::get(getGlobalContext(), val);
    }
};



#endif//_TYPECLASS_H__
