//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CGClass.h - Code Generator of Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_CLASS_H
#define FLY_CODEGEN_CLASS_H

#include "CodeGenClassVar.h"
#include "CodeGenClassFunction.h"

namespace llvm {
    class StructType;
    class AllocaInst;
}

namespace fly {

    class CodeGenModule;
    class ASTClass;

    class CodeGenClass {

        CodeGenModule * CGM = nullptr;

        ASTClass *AST = nullptr;

        llvm::StructType *Type = nullptr;

    public:
        CodeGenClass(CodeGenModule *CGM, ASTClass *Class, bool isExternal = false);

        llvm::StructType *getType();

        void InvokeDefaultConstructor(llvm::Value *Instance);
    };
}

#endif //FLY_CODEGEN_CLASS_H
