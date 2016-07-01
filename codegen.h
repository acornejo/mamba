#include "ast.h"
#include <iostream>
#include <string>
#include <map>
#include <stack>
#include <vector>
#include <memory>
#include <llvm/PassManager.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRbuilder.h>
#include <llvm/Support/TargetSelect.h>

using ::llvm::IRBuilder;
using ::llvm::ExecutionEngine;
using ::llvm::FunctionPassManager;
using ::llvm::BasicBlock;
using ::llvm::Module;
using ::llvm::LLVMContext;
using ::llvm::Value;
using ::llvm::Type;
using ::llvm::AllocaInst;
using std::unique_ptr;

struct Expr {
    std::string type_name;
    Type *type;
    Value *value;
};

typedef std::map<std::string, Expr*> env_t;

class Codegen: public ast::Visitor {
private:
    std::stack<Expr*> stack;
    std::vector<env_t> env;
    std::stack<BasicBlock*> continue_blocks;
    std::stack<BasicBlock*> break_blocks;
    Type *boolType;
    Type *intType;
    Type *floatType;

    unique_ptr<Module> module;
    unique_ptr<ExecutionEngine> engine;
    unique_ptr<IRBuilder<> > builder;
    unique_ptr<FunctionPassManager> pass_manager;

public:
    Codegen():
        module(unique_ptr<Module>(new Module("jit", llvm::getGlobalContext()))),
        engine(unique_ptr<ExecutionEngine>(ExecutionEngine::createJIT(module.get()))),
        builder(unique_ptr<IRBuilder<>>(new IRBuilder<>(module->getContext()))),
        pass_manager(unique_ptr<FunctionPassManager>(new FunctionPassManager(module.get()))) {

        pass_manager->add(new llvm::DataLayoutPass(*engine->getDataLayout()));
        pass_manager->add(llvm::createBasicAliasAnalysisPass());
        pass_manager->add(llvm::createPromoteMemoryToRegisterPass());
        pass_manager->add(llvm::createInstructionCombiningPass());
        pass_manager->add(llvm::createReassociatePass());
        pass_manager->add(llvm::createGVNPass());
        pass_manager->add(llvm::createCFGSimplificationPass());
        pass_manager->doInitialization();

        boolType = builder->getInt1Ty();
        intType = builder->getInt32Ty();
        floatType = builder->getFloatTy();
    }

    static void init() { llvm::InitializeNativeTarget(); }

    void dump() {
        module->dump();
    }

    void error(std::string msg) {
        std::cout << msg << std::endl;
    }

    Expr *getvar(std::string name) {
        for (auto it = env.rbegin(); it != env.rend(); ++it) {
            env_t &e = *it;
            auto eit = e.find(name);
            if (eit != e.end())
                return eit->second;
        }
        return nullptr;
    }

    void addvar(std::string name, Expr *val) {
        env.back().insert(std::make_pair(name, val));
    }

    void push_scope() {
        env.push_back(env_t());
    }

    void pop_scope() {
        env.pop_back();
    }

    virtual void visit(ast::True *v) {
        stack.push(new Expr("Bool", boolType, builder->getTrue()));
    }

    virtual void visit(ast::False *v) {
        stack.push(new Expr("Bool", boolType, builder->getFalse()));
	}

    virtual void visit(ast::Integer *v) {
        stack.push(new Expr("Int", intType, builder->getInt32(v->val)));
	}

    virtual void visit(ast::Real *v) {
        stack.push(new Expr("Float", floatType, ConstantFP::get(builder->getContext(), v->val)));
	}

    virtual void visit(ast::String *v) {
        Value *gs = builder->CreateGlobalString(v->val.c_str());
        stack.push(new Expr("String", builder->getInt8PtrTy(), builder->CreateConstGEP2_32(gs, 0, 0, "cast")));
	}

    virtual void visit(ast::Variable *v) {
        Expr *L = getvar(*(v->val));
        if (L != nullptr) {
            Value *val = builder->CreateLoad(L->value, *(v->val));
            stack.push(new Expr(L->type_name, L->type, val));
        } else
            error("variable " + *v->val + " not found!");
	}

    virtual void visit(ast::VarDecl *v) {
        v->expr->accept(this);
        assert(stack.size() >= 1);
        Expr *V = stack.top();
        stack.pop();

        AllocaInst *alloca = builder->CreateAlloca(V->value->getType(), 0, v->name->c_str());
        builder->CreateStore(V->value, alloca);

        addvar(*(v->name), new Expr(V->type_name, V->type, alloca));
	}

    virtual void visit(ast::FuncDecl *v) {
        v->func->accept(this);
        assert(stack.size() >= 1);
        Expr *V = stack.top();
        stack.pop();

        addvar(*(v->name), V);
    }

    virtual void visit(ast::Assign *v) {
        v->expr->accept(this);
        assert(stack.size() >= 1);
        Expr *R = stack.top();
        stack.pop();

        for (auto &var : v->vars) {
            var->accept(this);
            assert(stack.size() >= 1);
            Expr *L = stack.top();
            stack.pop();

            assert(L->type_name == R->type_name);
            builder->CreateStore(R->value, L->value);
        }
	}

    virtual void visit(ast::Unary *v) {
        v->down->accept(this);
        assert(stack.size() >= 1);
        Expr *V = stack.top();
        stack.pop();

        if (V->type_name == "Int" && method_name == "~") {
            Value *R = builder->CreateXOR(V->value, IntNot, "nottmp");
            stack.push(new Expr(V->type_name, intType, R);
            return;
        }

        if (V->type_name == "Bool" && method_name == "not") {
            Value *R = builder->CreateNot(V->value, "nottmp");
            stack.push(new Expr(V->type_name, boolType, R));
            return;
        }

        error("unsuported operation " + v ->opname);
        // TODO: implement static method
        /*
        Expr *result = applystaticmethod(V, v->opname, {V});
        assert(result != nullptr);
        stack.push(result);*/
	}

    virtual void visit(ast::Binary *v) {
        v->left->accept(this);
        v->right->accept(this);
        assert(stack.size() >= 2);

        Expr *R = stack.top();
        stack.pop();
        Expr *L = stack.top();
        stack.pop();

        // TODO: missing POW (**) operator
        if (R->type_name == "Int" && L->type_name == "Int") {
            if (v->opname == "+") {
                Value *res = builder->CreateAdd(L->value, R->value);
                stack.push(new Expr(R->type_name, intType, res));
                return;
            } else if (v->opname == "-")
                Value *res = builder->CreateSub(L->value, R->value);
                stack.push(new Expr(R->type_name, intType, res));
                return;
            } else if (v->opname == "*")
                Value *res = builder->CreateMul(L->value, R->value);
                stack.push(new Expr(R->type_name, intType, res));
                return;
            } else if (v->opname == "/")
                Value *res = builder->CreateSDiv(L->value, R->value);
                stack.push(new Expr(R->type_name, intType, res));
                return;
            } else if (v->opname == "%")
                Value *res = builder->CreateRem(L->value, R->value);
                stack.push(new Expr(R->type_name, intType, res));
                return;
            } else if (v->opname == "&")
                Value *res = builder->CreateAnd(L->value, R->value);
                stack.push(new Expr(R->type_name, intType, res));
                return;
            } else if (v->opname == "|")
                Value *res = builder->CreateOr(L->value, R->value);
                stack.push(new Expr(R->type_name, intType, res));
                return;
            } else if (v->opname == "^")
                Value *res = builder->CreateXor(L->value, R->value);
                stack.push(new Expr(R->type_name, intType, res));
                return;
            } else if (v->opname == "<<")
                Value *res = builder->CreateShl(L->value, R->value);
                stack.push(new Expr(R->type_name, intType, res));
                return;
            } else if (v->opname == ">>")
                Value *res = builder->CreateShr(L->value, R->value);
                stack.push(new Expr(R->type_name, intType, res));
                return;
            } else if (v->opname == "<")
                Value *res = builder->CreateICmpSLT(L->value, R->value);
                stack.push(new Expr(R->type_name, intType, res));
                return;
            } else if (v->opname == "<=")
                Value *res = builder->CreateICmpSLE(L->value, R->value);
                stack.push(new Expr(R->type_name, intType, res));
                return;
            } else if (v->opname == ">")
                Value *res = builder->CreateICmpSGT(L->value, R->value);
                stack.push(new Expr(R->type_name, intType, res));
                return;
            } else if (v->opname == ">=")
                Value *res = builder->CreateICmpSGE(L->value, R->value);
                stack.push(new Expr(R->type_name, intType, res));
                return;
            } else if (v->opname == "==")
                Value *res = builder->CreateICmpEQ(L->value, R->value);
                stack.push(new Expr(R->type_name, intType, res));
                return;
            } else if (v->opname == "!=")
                Value *res = builder->CreateICmpNE(L->value, R->value);
                stack.push(new Expr(R->type_name, intType, res));
                return;
            }
        }

        if (R->type_name == "Float" && L->type_name == "Float") {
            if (v->opname == "+") {
                Value *res = builder->CreateFAdd(L->value, R->value);
                stack.push(new Expr(R->type_name, floatType, res));
                return;
            } else if (v->opname == "-") {
                Value *res = builder->CreateFSub(L->value, R->value);
                stack.push(new Expr(R->type_name, floatType, res));
                return;
            } else if (v->opname == "*") {
                Value *res = builder->CreateFMul(L->value, R->value);
                stack.push(new Expr(R->type_name, floatType, res));
                return;
            } else if (v->opname == "/") {
                Value *res = builder->CreateFDiv(L->value, R->value);
                stack.push(new Expr(R->type_name, floatType, res));
                return;
            } else if (v->opname == "%") {
                Value *res = builder->CreateFRem(L->value, R->value);
                stack.push(new Expr(R->type_name, floatType, res));
                return;
            } else if (v->opname == "<")
                Value *res = builder->CreateFCmpOLT(L->value, R->value);
                stack.push(new Expr(R->type_name, intType, res));
                return;
            } else if (v->opname == "<=")
                Value *res = builder->CreateFCmpOLE(L->value, R->value);
                stack.push(new Expr(R->type_name, intType, res));
                return;
            } else if (v->opname == ">")
                Value *res = builder->CreateFCmpOGT(L->value, R->value);
                stack.push(new Expr(R->type_name, intType, res));
                return;
            } else if (v->opname == ">=")
                Value *res = builder->CreateFCmpOGE(L->value, R->value);
                stack.push(new Expr(R->type_name, intType, res));
                return;
            } else if (v->opname == "==")
                Value *res = builder->CreateFCmpOEQ(L->value, R->value);
                stack.push(new Expr(R->type_name, intType, res));
                return;
            } else if (v->opname == "!=")
                Value *res = builder->CreateFCmpONE(L->value, R->value);
                stack.push(new Expr(R->type_name, intType, res));
                return;
            }
        }

        error("unsuported operation " + v ->opname);
        // TODO: implement apply static methods
        /*
        Expr *result = applystaticmethod(R, v->opname, {R, L});
        assert(result != nullptr);
        stack.push(result);
        */
	}

    virtual void visit(ast::And *v) {
        v->left->accept(this);
        v->right->accept(this);
        assert(stack.size() >= 2);

        Expr *L = stack.top();
        assert(L->type_name == "Bool");
        stack.pop();

        Expr *R = stack.top();
        assert(R->type_name == "Bool");
        stack.pop();

        LLVMContext &ctx = builder->getContext();
        Function *func = builder->GetInsertBlock()->getParent();
        BasicBlock *and_lhs = builder->GetInsertBlock();
        BasicBlock *and_rhs = BasicBlock::Create(ctx, "and_rhs", func);
        BasicBlock *and_end = BasicBlock::Create(ctx, "and_end");

        builder->SetInsertPoint(and_lhs);
        builder->CreateCondBr(L->value, and_rhs, and_end);

        builder->SetInsertPoint(and_rhs);
        builder->CreateBr(and_end);

        builder->SetInsertPoint(and_end);
        PHINode *node = builder->CreatePHI(boolType, 2, "and_tmp");
        node->addIncoming(L->value, and_lhs);
        node->addIncoming(R->value, and_rhs);
        stack.push(new Expr("Bool", boolType, node));
	}

    virtual void visit(ast::Or *v) {
        v->left->accept(this);
        v->right->accept(this);
        assert(stack.size() >= 2);

        Expr *L = stack.top();
        assert(L->type_name == "Bool");
        stack.pop();

        Expr *R = stack.top();
        assert(R->type_name == "Bool");
        stack.pop();

        LLVMContext &ctx = builder->getContext();
        Function *func = builder->GetInsertBlock()->getParent();
        BasicBlock *or_lhs = builder->GetInsertBlock();
        BasicBlock *or_rhs = BasicBlock::Create(ctx, "or_rhs", func);
        BasicBlock *or_end = BasicBlock::Create(ctx, "or_end", func);

        builder->SetInsertPoint(or_lhs);
        builder->CreateCondBr(L->value, or_end, or_rhs);

        builder->SetInsertPoint(or_rhs);
        builder->CreateBr(or_end);

        builder->SetInsertPoint(or_end);
        PHINode *node = builder->CreatePHI(boolType, 2, "or_tmp");
        node->addIncoming(L->value, or_lhs);
        node->addIncoming(R->value, or_rhs);
        stack.push(new Expr("Bool", boolType, node));
	}

    virtual void visit(ast::IfElse *v) {
        v->expr->accept(this);
        assert(stack.size() >= 1);

        Expr *cond = stack.top();
        assert(cond->type_name == "Bool");
        stack.pop();

        LLVMContext &ctx = builder->getContext();
        Function *func = builder->GetInsertBlock()->getParent();
        if (v->ifelse) {
            BasicBlock *if_true = BasicBlock::Create(ctx, "if_true", func);
            BasicBlock *if_false = BasicBlock::Create(ctx, "if_false", func);
            BasicBlock *if_end = BasicBlock::Create(ctx, "if_end", func);
            builder->CreateCondBr(cond->value, if_true, if_false);

            builder->SetInsertPoint(if_true);
            push_scope();
            v->body->accept(this);
            pop_scope();
            builder->CreateBr(if_end);

            builder->SetInsertPoint(if_false);
            push_scope();
            v->ifelse->accept(this);
            pop_scope();
            builder->CreateBr(if_end);

            builder->SetInsertPoint(if_end);
        } else {
            BasicBlock *if_true = BasicBlock::Create(ctx, "if_true", func);
            BasicBlock *if_end = BasicBlock::Create(ctx, "if_end", func);
            builder->CreateCondBr(cond->value, if_true, if_end);

            builder->SetInsertPoint(if_true);
            push_scope();
            v->body->accept(this);
            pop_scope();
            builder->CreateBr(if_end);

            builder->SetInsertPoint(if_end);
        }
	}

    virtual void visit(ast::While *v) {
        v->expr->accept(this);
        assert(stack.size() >= 1);

        Expr *cond = stack.top();
        assert(cond->type_name == "Bool");
        stack.pop();

        LLVMContext &ctx = builder->getContext();
        Function *func = builder->GetInsertBlock()->getParent();
        BasicBlock *while_start = BasicBlock::Create(ctx, "while_start", func);
        BasicBlock *while_body = BasicBlock::Create(ctx, "while_body", func);
        BasicBlock *while_end = BasicBlock::Create(ctx, "while_end", func);

        continue_blocks.push(while_start);
        break_blocks.push(while_end);

        builder->SetInsertPoint(while_start);
        builder->CreateCondBr(cond->value, while_body, while_end);

        builder->SetInsertPoint(while_body);
        push_scope();
        v->body->accept(this);
        pop_scope()
        builder->CreateBr(while_start);

        builder->SetInsertPoint(while_end);
        continue_blocks.pop();
        break_blocks.pop();
	}

    virtual void visit(ast::For *v) {
        if (getvar(*(v->vname))) {
            error("variable " + *v->vname + " is shadowed in for loop");
            return;
        }

        v->iterable->accept(this);
        assert(stack.size() >= 1);
        Expr *iterable = stack.top();

#if 0
        Expr *iter = applymethod(iterable, "iter");
        assert(iter != nullptr);
        assert(hasmethod(iter, "hasNext"));
        assert(hasmethod(iter, "next"));

        push_scope();

        // TODO: fix this, need the correct type
        Str type_name = get_str_type_of_element_being_iterated;
        Type *type = get_llvm_type_of_element_being_iterated;
        AllocaInst *alloca = builder->CreateAlloca(type, 0, v->vname->c_str());
        addvar(*(v->vname), new Expr(type_name, type, alloca));

        LLVMContext &ctx = builder->getContext();
        Function *func = builder->GetInsertBlock()->getParent();
        BasicBlock *for_start = BasicBlock::Create(ctx, "for_start", func);
        BasicBlock *for_body = BasicBlock::Create(ctx, "for_body", func);
        BasicBlock *for_end = BasicBlock::Create(ctx, "for_end", func);

        continue_blocks.push(for_start);
        break_blocks.push(for_end);

        builder->SetInsertPoint(for_start);
        Expr *has_next = applymethod(iter, "hasNext");
        assert(has_next != nullptr);
        assert(has_next->type_name == "Bool");
        builder->CreateCondBr(has_next->value, for_body, for_end);

        builder->SetInsertPoint(for_body);
        // TODO: fix this, next returns Maybe{T}, must unwrap
        Expr *next = applymethod(iter, "next");
        assert(next != nullptr);
        builder->CreateStore(next->value, alloca);
        v->body->accept(this);
        builder->CreateBr(for_start);

        builder->SetInsertPoint(for_end);
        continue_blocks.pop();
        break_blocks.pop();
        pop_scope();
#endif
	}

    virtual void visit(ast::Break *v) {
        assert(break_blocks.size() > 0);
        builder->CreateBr(break_block.top());
	}

    virtual void visit(ast::Continue *v) {
        assert(continue_blocks.size() > 0);
        builder->CreateBr(break_block.top());
	}

    virtual void visit(ast::Return *v) {
        if (v->e) {
            v->e->accept(this);
            assert(stack.size() >= 1);
            Expr *V = stack.top();
            stack.pop();

            // TODO: check that return type matches function signature
            builder->CreateRet(V->value);
        } else {
            builder->CreateRetVoid();
        }
	}

    virtual void visit(ast::Function *v) {
        ast::FuncType proto = v->proto;
        Type *ret_type = translate_type(proto->ret->type_name());
        std::vector<Type *> args;

        for (auto &arg: proto->params.childNodes) {
            if (ast::Type *targ = dynamic_cast<ast::Type*>(arg))
                args.push_back(translate_type(targ->type_name()))
        }

        const bool isVarArg = false;
        FunctionType *func_type = FunctionType::get(ret_type, args, isVarArg);
        Function *func = Function::Create(func_type, Function::ExternalLinkage, "");

        size_t i = 0;
        for (auto &arg : func->args()) {
            arg.setName(*proto->params.names[i]);
            i++;
        }

        LLVMContext &ctx = builder->getContext();
        BasicBlock *func_entry = BasicBlock::Create(ctx, "entry", func);
        builder->SetInsertPoint(func_entry);
        push_scope();

        i = 0;
        for (auto &arg : func->args()) {
            ast::Type *targ = dynamic_cast<ast::Type*>(proto->params.childNodes[i]);
            AllocaInst *alloca = builder->CreateAlloca(arg.getType(), 0, arg.getName());
            builder->CreateStore(arg, alloca);
            addvar(arg.getName(), new Expr(targ->type_name(), arg.getType(), alloca));
            i++;
        }
        v->body->accept(this);
        // call verifyFunction
        // TODO: missing stmtlist, which shouldn't push anything..
        pop_scope()

        stack.push(new Expr(proto->type_name, func_type, func));
	}

    virtual void visit(ast::Call *v) {
        v->parent->accept(this);
        assert(stack.size() >= 1);
        Expr *callee_func = stack.top();
        stack.pop();

        FunctionType *func_type = dynamic_cast<FunctionType*>(callee_func->type);
        assert(func_type);
        assert(v->params.childNodes.size() == func_type->getNumParams());

        std::vector<Value*> arg_values;
        int i = 0;
        for (auto &param: v->params.childNodes) {
            param->accept(this);
            assert(stack.size() >= 1);
            Expr *V = stack.top();
            stack.pop();

            assert(V->type == func_type->getParamType (i));
            arg_values.push_back(V->value);
            i++;
        }

        std::string func_type_name = getreturntype(callee_func->type_name);
        Value *call = builder->CreateCall(callee_func->value, arg_values, "calltmp");
        stack.push(new Expr(func_type_name, func_type->getReturnType(), call))
	}

    virtual void visit(ast::Array *v) { }
    virtual void visit(ast::Subscript *v) { }
    virtual void visit(ast::Attribute *v) { }
    virtual void visit(ast::Expr *v) { }
    virtual void visit(ast::UnionItem *v) { }
    virtual void visit(ast::UnionList *v) { }
    virtual void visit(ast::RecordDef *v) { }
    virtual void visit(ast::UnionDef *v) { }
    virtual void visit(ast::ExprList *v) { }
    virtual void visit(ast::StmtList *v) { }
    virtual void visit(ast::SimpleType *) { }
    virtual void visit(ast::RefType *) { }
    virtual void visit(ast::PtrType *) { }
    virtual void visit(ast::ArrayType *) { }
    virtual void visit(ast::TupleType *) { }
    virtual void visit(ast::FuncType *) { }
    virtual void visit(ast::TypeList *) { }
};
