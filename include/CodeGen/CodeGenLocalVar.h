//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CGVar.h - Code Generator of Statements
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CGLOCALVAR_H
#define FLY_CGLOCALVAR_H

#include "CodeGenVar.h"
#include "llvm/IR/Instructions.h"

namespace fly {

    class CodeGenModule;
    class ASTLocalVar;
    class ASTParam;
    class ASTVar;

    class CodeGenLocalVar : public CodeGenVar {

        CodeGenModule *CGM;
        ASTVar *Var;
        llvm::AllocaInst *AllocaI = nullptr;
        llvm::LoadInst *LoadI = nullptr;
        bool Reload = false;
        bool isStored = false;
        llvm::StringRef BlockID;

    public:
        CodeGenLocalVar(CodeGenModule *CGM, ASTVar *Var);

        llvm::Value *getPointer() override;

        llvm::Value *getValue() override;

        llvm::AllocaInst *Alloca();

        llvm::StoreInst *Store(llvm::Value *Val) override;

        llvm::LoadInst *Load() override;

        bool needReload();
    };
}

#endif //FLY_CGLOCALVAR_H
