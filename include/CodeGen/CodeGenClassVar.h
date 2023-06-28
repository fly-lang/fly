//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/Class.h - Code Generator of Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_CLASSVAR_H
#define FLY_CODEGEN_CLASSVAR_H

#include "CodeGenVarBase.h"
#include "AST/ASTClassVar.h"

namespace llvm {
    class StringRef;
    class StructType;
    class AllocaInst;
}

namespace fly {

    class CodeGenModule;
    class CodeGenClass;
    class ASTClass;
    class ASTClassVar;
    class ASTClassFunction;

    class CodeGenClassVar : public CodeGenVarBase {

        friend class CodeGenClass;

        llvm::Type *ClassType = nullptr;

        llvm::Value *Index = nullptr;

        llvm::Value *Zero = nullptr;

        llvm::Value *Instance = nullptr;

    protected:
        llvm::StringRef BlockID;

    public:
        CodeGenClassVar(CodeGenModule *CGM, ASTClassVar *Var, llvm::Type *ClassType,  uint32_t Index);

        void Init();

        llvm::StoreInst *Store(llvm::Value *Val);

        llvm::LoadInst *Load();

        llvm::Value *getValue();

        llvm::Value *getPointer();

        llvm::Value *getIndex();

        void setInstance(llvm::Value *Inst);
    };
}

#endif //FLY_CODEGEN_CLASSVAR_H
