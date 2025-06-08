//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenClassVar.h - Code Generator of Class Var
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_CLASSVAR_H
#define FLY_CODEGEN_CLASSVAR_H

#include "CodeGenVarBase.h"
#include "llvm/ADT/StringRef.h"

namespace llvm {
    class StructType;
    class AllocaInst;
    class Type;
}

namespace fly {

    class CodeGenModule;
    class SemaClassAttribute;

    class CodeGenClassVar : public CodeGenVarBase {

        friend class CodeGenClass;

        CodeGenModule *CGM = nullptr;

        llvm::Type *T = nullptr;

        llvm::Value *Pointer = nullptr;

        llvm::StringRef BlockID;

        llvm::Type *Type = nullptr;

        llvm::Value *Index = nullptr;

        llvm::Value *Zero = nullptr;

        llvm::LoadInst *LoadI = nullptr;

        llvm::Value *InstancePtr = nullptr;

    public:
        CodeGenClassVar(CodeGenModule *CGM, SemaClassAttribute *Sema, uint64_t Index);

        void setInstancePtr(llvm::Value *Val);

        llvm::StoreInst *Store(llvm::Value *Val) override;

        llvm::LoadInst *Load() override;

        llvm::Value *getValue() override;

        llvm::Value *getIndex();

        llvm::Type* getType() override;

        llvm::Value *getPointer() override;

    };
}

#endif //FLY_CODEGEN_CLASSVAR_H
