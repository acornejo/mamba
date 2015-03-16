#include "ast.h"
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
using std::unique_ptr;

struct Expr {
    std::string type_name;
    llvm::Type *type;
    llvm::Value *value;
};

typedef std::map<std::string, Expr*> env_t;

class Codegen: public ast::Visitor {
private:
    std::stack<Expr*> stack;
    std::vector<env_t> env;
    std::stack<BasicBlock*> continue_blocks;
    std::stack<BasicBlock*> break_blocks;

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
    }

    static void init() { llvm::InitializeNativeTarget(); }

    void dump() {
        module->dump();
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

    virtual void visit(ast::True *v) {
        stack.push(new Expr("Bool", builder->getTrue()));
    }

    virtual void visit(ast::False *v) {
        stack.push(new Expr("Bool", builder->getFalse()));
	}

    virtual void visit(ast::Integer *v) {
        stack.push(new Expr("Int", builder->getInt32(v->val)));
	}

    virtual void visit(ast::Real *v) {
        stack.push(new Expr("Float", ConstantFP::get(builder->getContext(), v->val)));
	}

    virtual void visit(ast::String *v) {
        Value *gs = builder->CreateGlobalString(v->val.c_str(), "globalstring");
        stack.push(new Expr("String", builder->CreateConstGEP2_32(gs, 0, 0, "cast")));
	}

    virtual void visit(ast::Variable *v) {
        Expr *L = getvar(*(v->val));
        if (L != nullptr) {
            Value *val = builder->CreateLoad(L->value, name);
            stack.push(new Expr(L->type_name, val));
        } else
            error("variable %s not found!",v->val->c_str());
	}

    virtual void visit(ast::Declaration *v) {
        v->expr->accept(this);
        assert(stack.size() >= 1);

        Expr *V = stack.top();
        stack.pop();

        AllocaInst *alloca = builder->CreateAlloca(V->value->getType(), 0, v->name->c_str());
        builder->CreateStore(V->value, alloca);

        env.back().insert(std::make_pair(*(v->name), new Expr(V->type_name, alloca)));
	}

    virtual void visit(ast::Assign *v) {
        v->expr->accept(this);
        assert(stack.size() >= 1);

        Expr *R = stack.top();
        stack.pop();

        for (auto &n : v->vars) {
            n->accept(this);
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

        Env *env = Env::byname(V->type_name);
        assert(env != nullptr);
        assert(env->has_function(v->op, {V->type_name}));
        Expr *result = env->apply_function(v->op, {V});
        assert(result != nullptr);
        stack.push(result);
	}

    virtual void visit(ast::Binary *v) {
        v->left->accept(this);
        v->right->accept(this);
        assert(stack.size() >= 2);

        Expr *R = stack.top();
        stack.pop();
        Expr *L = stack.top();
        stack.pop();

        Env *env = Env::byname(R->type_name);
        assert(env != NULL);
        assert(env->has_function(v->op, {R->type_name, L->type_name}));
        Expr *result = env->apply_function(v->op, {R, L});
        assert(result != NULL);
        stack.push(result);
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
        PHINode *node = builder->CreatePHI(builder->getInt1Ty(), 2, "and_tmp");
        node->addIncoming(L->value, and_lhs);
        node->addIncoming(R->value, and_rhs);
        stack.push(new Expr("Bool", node));
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
        PHINode *node = builder->CreatePHI(builder->getInt1Ty(), 2, "or_tmp");
        node->addIncoming(L->value, or_lhs);
        node->addIncoming(R->value, or_rhs);
        stack.push(new Expr("Bool", node));
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
            v->body->accept(this);
            builder->CreateBr(if_end);

            builder->SetInsertPoint(if_false);
            v->ifelse->accept(this);
            builder->CreateBr(if_end);

            builder->SetInsertPoint(if_end);
        } else {
            BasicBlock *if_true = BasicBlock::Create(ctx, "if_true", func);
            BasicBlock *if_end = BasicBlock::Create(ctx, "if_end", func);
            builder->CreateCondBr(cond->value, if_true, if_end);

            builder->SetInsertPoint(if_true);
            v->body->accept(this);
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
        v->body->accept(this);
        builder->CreateBr(while_start);

        builder->SetInsertPoint(while_end);
        continue_blocks.pop();
        break_blocks.pop();
	}

    virtual void visit(ast::Break *v) {
        assert(break_blocks.size() > 0);
        builder->CreateBr(break_block.top());
	}

    virtual void visit(ast::Continue *v) {
        assert(continue_blocks.size() > 0);
        builder->CreateBr(break_block.top());
        v ->
	}

    virtual void visit(ast::For *v) {
	}

    virtual void visit(ast::Function *v) {
	}

    virtual void visit(ast::Return *v) {
        if (v->e) {
            v->e->accept(this);
            assert(stack.size() >= 1);

            Expr *V = stack.top();
            stack.pop();

            builder->CreateRet(V->value);
        } else {
            builder->CreateRetVoid();
        }
	}

    virtual void visit(ast::Call *v) {
        // TODO: get function
        // TODO: match arguments against function
        std::vector<Value*> arg_values;

        for (auto &n: v->params.childNodes) {
            n->accept(this);
            assert(stack.size() >= 1);

            Expr *V = stack.top();
            stack.pop();
            arg_values.push_back(V->values);
        }

        builder->CreateCall(callee_func, arg_values, "calltmp");
	}

    virtual void visit(ast::Array *v) {
	}

    virtual void visit(ast::Subscript *v) {
	}
    virtual void visit(ast::Expr *v) {
	}
    virtual void visit(ast::FuncDecl *v) {
	}
    virtual void visit(ast::UnionItem *v) {
	}
    virtual void visit(ast::UnionList *v) {
	}
    virtual void visit(ast::RecordDef *v) {
	}
    virtual void visit(ast::UnionDef *v) {
	}
    virtual void visit(ast::ExprList *v) {
	}
    virtual void visit(ast::StmtList *v) {
	}
    virtual void visit(ast::SimpleType *) { }
    virtual void visit(ast::RefType *) { }
    virtual void visit(ast::PtrType *) { }
    virtual void visit(ast::ArrayType *) { }
    virtual void visit(ast::TupleType *) { }
    virtual void visit(ast::FuncType *) { }
    virtual void visit(ast::TypeList *) { }
};
