//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/Class.h - Code Generator of Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_CLASSFUNCTION_H
#define FLY_CODEGEN_CLASSFUNCTION_H

#include "CodeGenFunctionBase.h"

namespace llvm {
    class Value;
    class PointerType;
}

namespace fly {

    class CodeGenModule;
    class CodeGenClass;
    class ASTClassMethod;

    class CodeGenClassFunction : public CodeGenFunctionBase {

        friend class CodeGenClass;

        CodeGenClassFunction(CodeGenModule *CGM, ASTClassMethod *AST, llvm::PointerType *TypePtr = nullptr);

    public:

        void GenBody() override;
    };
}

#endif //FLY_CODEGEN_CLASSFUNCTION_H
