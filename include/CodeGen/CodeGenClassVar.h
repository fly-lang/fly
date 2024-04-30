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
    class StringRef;
}

namespace fly {

    class CodeGenModule;
    class ASTClassAttribute;

    class CodeGenClassVar : public CodeGenVarBase {

        friend class CodeGenClass;

        CodeGenModule *CGM = nullptr;

        ASTClassAttribute *Var = nullptr;

        llvm::Type *T = nullptr;

        llvm::Value *Pointer = nullptr;

        llvm::StringRef BlockID;

        llvm::Type *ClassType = nullptr;

        llvm::Value *Index = nullptr;

        llvm::Value *Zero = nullptr;

        llvm::LoadInst *LoadI = nullptr;

        llvm::Value *Instance = nullptr;

    public:
        CodeGenClassVar(CodeGenModule *CGM, ASTClassAttribute *Var, llvm::Type *ClassType, uint32_t Index);

        void Init() override;

        llvm::StoreInst *Store(llvm::Value *Val) override;

        llvm::LoadInst *Load() override;

        llvm::Value *getValue() override;

        llvm::Value *getPointer() override;

        ASTVar *getVar() override;

        llvm::Value *getIndex();

        void setInstance(llvm::Value *Inst);
    };
}

#endif //FLY_CODEGEN_CLASSVAR_H
