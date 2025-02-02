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

    class SymFunction;
    class CodeGenModule;

    class CodeGenFunction : public CodeGenFunctionBase {

        bool isExternal;

        bool isMain;

    public:
        CodeGenFunction(CodeGenModule *CGM, SymFunction *Sym, bool isExternal = false);

        void GenBody() override;

        static bool isMainFunction(SymFunction *Sym);
    };
}

#endif //FLY_CODEGEN_FUNCTION_H
