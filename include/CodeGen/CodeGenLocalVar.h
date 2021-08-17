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
    class ASTFuncParam;

    class CodeGenLocalVar : public CodeGenVar {

        CodeGenModule *CGM;
        bool Constant;
        llvm::AllocaInst *AllocaI;
        llvm::LoadInst *LoadI = nullptr;
        bool Reload;
        bool isStored;
        llvm::StringRef BlockID;

    public:
        CodeGenLocalVar(CodeGenModule *CGM, ASTLocalVar *S);

        CodeGenLocalVar(CodeGenModule *CGM, ASTFuncParam *P);

        llvm::Value *getPointer() override;

        llvm::Value *getValue() override;

        llvm::StoreInst *Store(llvm::Value *Val);

        llvm::LoadInst *Load();

        bool needReload();
    };
}

#endif //FLY_CGLOCALVAR_H
