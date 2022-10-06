//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CGVar.h - Code Generator of Statements
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CG_VAR_H
#define FLY_CG_VAR_H

#include "CodeGenVarBase.h"

namespace llvm {
    class Type;
    class Value;
    class LoadInst;
    class StringRef;
}

namespace fly {

    class CodeGenModule;
    class ASTLocalVar;
    class ASTParam;
    class ASTVar;

    class CodeGenVar : public CodeGenVarBase {

    protected:

        CodeGenModule *CGM = nullptr;

        ASTVar *Var = nullptr;

        llvm::Type *T = nullptr;

        llvm::Value *Pointer = nullptr;

        llvm::LoadInst *LoadI = nullptr;

        bool needLoad = false;

        bool isStored = false;

        llvm::StringRef BlockID;

    public:
        CodeGenVar(CodeGenModule *CGM, ASTVar *Var);

        void Init() override;

        llvm::StoreInst *Store(llvm::Value *Val) override;

        llvm::Value *Load() override;

        bool needReload();

        llvm::Value *getValue() override;

        llvm::Value *getPointer() override;
    };
}

#endif //FLY_CG_VAR_H
