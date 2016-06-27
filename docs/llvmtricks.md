# Finding the current function you are in:
    llvm::Function *func =
    Builder.GetInsertBlock()->getParent();
# External function
    llvm::Function::Create(
            llvm::FunctionType::get(returnTy, args, false),
            llvm::Function::ExternalLinkage,
            Name,
            TheModule
            );

# promote boolean (i1ty) to integer (i32ty)
    llvm::Value *promo =
    Builder.CreateZExt(*i, Builder.getInt32Ty(), "zexttmp")
