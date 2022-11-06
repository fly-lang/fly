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

namespace fly {

    class CodeGenModule;
    class ASTVar;

    class CodeGenVar : public CodeGenVarBase {

        llvm::StringRef BlockID;

    public:
        CodeGenVar(CodeGenModule *CGM, ASTVar *Var);

        void Init() override;

        llvm::StoreInst *Store(llvm::Value *Val) override;

        llvm::LoadInst *Load() override;

        llvm::Value *getValue() override;
    };
}

#endif //FLY_CG_VAR_H
