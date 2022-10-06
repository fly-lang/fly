//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CGVar.h - Code Generator of Statements
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_VARBASE_H
#define FLY_CODEGEN_VARBASE_H

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Instructions.h"

namespace fly {

    class CodeGenModule;
    class ASTVar;

    class CodeGenVarBase {

    protected:

        CodeGenVarBase(CodeGenModule *CGM, ASTVar *Var);

        CodeGenModule *CGM = nullptr;

        ASTVar *Var = nullptr;

        llvm::Type *T = nullptr;

        llvm::Value *Pointer = nullptr;

        llvm::LoadInst *LoadI = nullptr;

        bool doLoad = false;

    public:

        virtual void Init() = 0;

        virtual llvm::StoreInst *Store(llvm::Value *Val);

        virtual llvm::LoadInst *Load();

        virtual llvm::Value *getValue();

        virtual llvm::Value *getPointer();
    };
}

#endif //FLY_CODEGEN_VARBASE_H
