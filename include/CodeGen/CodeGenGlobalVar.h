//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CGGlobalVar.h - Code Generator of Global Var
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_GLOBALVAR_H
#define FLY_CODEGEN_GLOBALVAR_H

#include "CodeGenVarBase.h"

namespace llvm {
	class GlobalVariable;
}

namespace fly {

    class SemaGlobalVar;
    class CodeGenModule;

    class CodeGenGlobalVar : public CodeGenVarBase {

        CodeGenModule *CGM;

        SemaGlobalVar *Sym;

        llvm::Type *T = nullptr;

        llvm::Value *Pointer = nullptr;

        llvm::Value *Instance = nullptr;

        llvm::LoadInst *LoadI = nullptr;

    public:
        CodeGenGlobalVar(CodeGenModule *CGM, SemaGlobalVar* Sym, bool isExternal = false);

//        llvm::AllocaInst *Alloca() override;

        llvm::Type *getType() override;

        llvm::StoreInst *Store(llvm::Value *Val) override;

        llvm::LoadInst *Load() override;

        llvm::Value *getValue() override;

        llvm::Value *getPointer() override;
    };
}

#endif //FLY_CODEGEN_GLOBALVAR_H
