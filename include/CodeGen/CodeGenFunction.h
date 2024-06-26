//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CGFunction.h - Code Generator of Function
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGENFUNCTION_H
#define FLY_CODEGENFUNCTION_H

#include "CodeGenFunctionBase.h"

namespace fly {

    class ASTFunction;
    class CodeGenModule;

    class CodeGenFunction : public CodeGenFunctionBase {

        ASTFunction *AST = nullptr;

        bool isExternal = false;

    public:
        CodeGenFunction(CodeGenModule *CGM, ASTFunction *AST, bool isExternal = false);

        void GenBody() override;

        static bool isMainFunction(ASTFunctionBase *FunctionBase);
    };
}

#endif //FLY_CODEGENFUNCTION_H
