//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CGGlobalVar.h - Code Generator of Global Var
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGENGLOBALVAR_H
#define FLY_CODEGENGLOBALVAR_H

#include "AST/TypeBase.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/ADT/StringRef.h"
#include <llvm/IR/Instructions.h>

namespace fly {

    class CodeGenModule;

    class CodeGenGlobalVar {

        CodeGenModule *CGM;
        llvm::GlobalVariable *GVar;
        llvm::LoadInst *LoadI;
        bool isStored;
        bool needLoad;

    public:
        CodeGenGlobalVar(CodeGenModule *CGM, llvm::StringRef Name, const TypeBase *Ty, llvm::StringRef StrVal,
                         const bool isConstant);

        llvm::GlobalVariable *getGlobalVar() const;

        llvm::User *get();

        llvm::StoreInst *Store(llvm::Value *Val);

        llvm::LoadInst *Load();
    };
}

#endif //FLY_CODEGENGLOBALVAR_H
