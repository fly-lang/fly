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

#include "CodeGenVarBase.h"
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

        llvm::GlobalVariable *GVar = nullptr;

        llvm::LoadInst *LoadI = nullptr;

        bool needLoad;

    public:
        CodeGenGlobalVar(CodeGenModule *CGM, ASTGlobalVar* AST, bool isExternal = false);

        void Init() override;

        llvm::Value *getPointer() override;

        llvm::StoreInst *Store(llvm::Value *Val) override;

        llvm::Value *Load() override;

        llvm::Value *getValue() override;
    };
}

#endif //FLY_CODEGENGLOBALVAR_H
