//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CGFunction.h - Code Generator of Function
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_FUNCTION_H
#define FLY_CODEGEN_FUNCTION_H

#include "CodeGenFunctionBase.h"

namespace fly {

    class SemaFunction;
    class CodeGenModule;

    class CodeGenFunction : public CodeGenFunctionBase {

        std::string Id;

        bool isExternal;

        bool isMain;

        std::string toIdentifier(SemaFunction *Function);

    public:
        CodeGenFunction(CodeGenModule *CGM, SemaFunction *Sema, bool isExternal = false);

        void GenBody() override;

        static bool isMainFunction(SemaFunction *Sema);
    };
}

#endif //FLY_CODEGEN_FUNCTION_H
