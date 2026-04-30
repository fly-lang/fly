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

        size_t Index;

        llvm::SmallVector<CodeGenVar *, 4> Attributes;

        llvm::Type *ClassType;

        llvm::PointerType *ClassTypePtr;

        bool Static;

        CodeGenClassMethod(CodeGenModule *CGM, SemaClassMethod *Sema, llvm::StructType *Type, size_t Index);

    public:

        size_t getIndex() const;

        bool isStatic() const;

        void GenBody() override;

        std::string toIdentifier(SemaClassMethod *ClassMethod);
    };
}

#endif //FLY_CODEGEN_CLASSMETHOD_H
