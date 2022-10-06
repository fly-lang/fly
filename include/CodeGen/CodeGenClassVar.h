//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CGClass.h - Code Generator of Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_CLASSVAR_H
#define FLY_CODEGEN_CLASSVAR_H

#include "CodeGenVar.h"

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

    class CodeGenClassVar : public CodeGenVar {

        friend class CodeGenClass;

        llvm::Value *ClassInstance = nullptr;

        llvm::Type *ClassType = nullptr;

        llvm::Value *Zero = nullptr;

        llvm::Value *Index = nullptr;

        CodeGenClassVar(CodeGenModule *CGM, ASTClassVar *Var, llvm::Type *ClassType, uint32_t Index);

        void setClassInstance(llvm::Value *Instance);

        void Init() override;

        llvm::Value *Load() override;

//        llvm::Value *getValue() override;

        llvm::Value *getPointer() override;

    };
}

#endif //FLY_CODEGEN_CLASSVAR_H
