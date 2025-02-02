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
#include "llvm/ADT/StringMap.h"

namespace llvm {
    class Type;
    class StringRef;
}

namespace fly {

    class CodeGenModule;
    class ASTVar;

    class CodeGenVar : public CodeGenVarBase {

    protected:

        CodeGenModule *CGM = nullptr;

        CodeGenVar *Parent = nullptr;

        llvm::StringMap<CodeGenVar *> Vars;

        llvm::Type *T = nullptr;

        llvm::AllocaInst *Pointer = nullptr;

        llvm::LoadInst *LoadI = nullptr;

        llvm::StringRef BlockID;

        uint32_t Index = 0;

    public:
//        CodeGenVar(CodeGenModule *CGM, ASTVar *Var);

        CodeGenVar(CodeGenModule *CGM, llvm::Type *T);

        CodeGenVar(CodeGenModule *CGM, llvm::Type *Ty, CodeGenVar *Parent, uint32_t Index);

        CodeGenVar *getParent();

        CodeGenVarBase *getVar(llvm::StringRef Name);

        llvm::Type *getType() override;

        llvm::AllocaInst *Alloca();

        llvm::StoreInst *Store(llvm::Value *Val) override;

        llvm::LoadInst *Load() override;

        llvm::Value *getValue() override;

        llvm::Value *getPointer() override;

        void addVar(llvm::StringRef Name, CodeGenVar *CGV);
    };
}

#endif //FLY_CODEGEN_VAR_H
