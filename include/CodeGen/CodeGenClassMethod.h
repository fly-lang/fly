//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/Class.h - Code Generator of Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_CLASSMETHOD_H
#define FLY_CODEGEN_CLASSMETHOD_H

#include "CodeGenFunctionBase.h"

namespace llvm {
    class Value;
    class PointerType;
    class AllocaInst;
}

namespace fly {

    class CodeGenModule;
    class CodeGenClass;
    class CodeGenVar;
    class SemaClassType;
    class SemaClassMethod;

    class CodeGenClassMethod : public CodeGenFunctionBase {

        friend class CodeGenClass;

        llvm::SmallVector<CodeGenVar *, 4> Attributes;

        llvm::PointerType *ClassTypePtr;

        CodeGenClassMethod(CodeGenModule *CGM, SemaClassMethod *Sema);

    public:

        void GenBody() override;
    };
}

#endif //FLY_CODEGEN_CLASSMETHOD_H
