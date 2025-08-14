//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/Class.h - Code Generator of Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_ENUMENTRY_H
#define FLY_CODEGEN_ENUMENTRY_H

#include "CodeGenVarBase.h"

namespace llvm {
    class Value;
}

namespace fly {

    class CodeGenModule;
    class SemaEnumEntry;

    class CodeGenEnumEntry : public CodeGenVarBase {

        llvm::Value *Value;

        llvm::Type *T;

        llvm::Value *Instance = nullptr;

        size_t Index;

    public:
        CodeGenEnumEntry(CodeGenModule *CGM, SemaEnumEntry *Sema);

//        llvm::AllocaInst *Alloca() override;

        llvm::Type *getType() override;

        llvm::StoreInst *Store(llvm::Value *Val) override;

        llvm::LoadInst *Load() override;

        llvm::Value *getValue() override;

        llvm::Value *getPointer() override;

        void setPointer(llvm::Value *Pointer) override;

        size_t getIndex() override;
    };
}

#endif //FLY_CODEGEN_ENUMENTRY_H
