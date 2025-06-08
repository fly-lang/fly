//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenVar.h - Code Generator of Statements
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_VAR_H
#define FLY_CODEGEN_VAR_H

#include "CodeGenVarBase.h"
#include "llvm/ADT/StringRef.h"

namespace llvm {
    class Type;
    class StringRef;
    class ConstantInt;
}

namespace fly {

    class CodeGenModule;
    class ASTVar;
    class SemaVar;

    class CodeGenVar : public CodeGenVarBase {

    protected:

        CodeGenModule *CGM = nullptr;

        SemaVar *Sema = nullptr;

        llvm::Type *T = nullptr;

        llvm::Value *Pointer = nullptr;

        llvm::LoadInst *LoadI = nullptr;

        llvm::StringRef BlockID;

    public:
//        CodeGenVar(CodeGenModule *CGM, ASTVar *Var);

        CodeGenVar(CodeGenModule *CGM, SemaVar *Sema, llvm::Type *T, llvm::Value *Pointer);

        llvm::Type *getType() override;

        llvm::StoreInst *Store(llvm::Value *Val) override;

        llvm::LoadInst *Load() override;

        llvm::Value *getValue() override;

        llvm::Value *getPointer() override;

    };
}

#endif //FLY_CODEGEN_VAR_H
