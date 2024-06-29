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

#include "CodeGenVar.h"
#include "AST/ASTType.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/ADT/StringRef.h"
#include <llvm/IR/Instructions.h>
#include <AST/ASTExpr.h>

namespace fly {

    class CodeGenModule;
    class ASTGlobalVar;

    class CodeGenGlobalVar : public CodeGenVarBase {

        CodeGenModule *CGM = nullptr;

        ASTVar *Var = nullptr;

        llvm::Type *T = nullptr;

        llvm::Value *Pointer = nullptr;

        llvm::Value *Instance = nullptr;

        llvm::LoadInst *LoadI = nullptr;

    public:
        CodeGenGlobalVar(CodeGenModule *CGM, ASTGlobalVar* Var, bool isExternal = false);

//        llvm::AllocaInst *Alloca() override;

        llvm::StoreInst *Store(llvm::Value *Val) override;

        llvm::LoadInst *Load() override;

        llvm::Value *getValue() override;

        llvm::Value *getPointer() override;

        CodeGenVarBase *getVar(llvm::StringRef Name);
    };
}

#endif //FLY_CODEGENGLOBALVAR_H
