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

#include "llvm/IR/Instructions.h"

namespace fly {

    class CodeGenModule;
    class VarDeclStmt;
    class FuncParam;

    class CodeGenVar {

        CodeGenModule *CGM;
        bool Constant;
        llvm::AllocaInst *AllocaI;
        llvm::LoadInst *LoadI;
        bool needLoad;
        bool isStored;

    public:
        CodeGenVar(CodeGenModule *CGM, VarDeclStmt *S);

        CodeGenVar(CodeGenModule *CGM, FuncParam *P);

        llvm::UnaryInstruction *get();

        llvm::StoreInst *Store(llvm::Value *Val);

        llvm::LoadInst *Load();
    };
}

#endif //FLY_CGLOCALVAR_H
