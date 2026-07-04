//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenFunction.h - function code generation
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
    class SemaParam;
    class CodeGenModule;

    class CodeGenFunction : public CodeGenFunctionBase {

        bool isExternal;

        bool isMain;

        // C-ABI runtime function: a fly.runtime-namespace function with a real body,
        // emitted as an unmangled C symbol (no error param, const params by value,
        // first non-const param -> return value) to match the fly.runtime call-site ABI.
        bool isCABI = false;
        SemaParam *CABIOutParam = nullptr;

        std::string toIdentifier(SemaFunction *Function);

        void GenMainArgs();

        void GenCABIBody();

    public:
        CodeGenFunction(CodeGenModule *CGM, SemaFunction *Sema, bool isExternal = false);

        void GenBody() override;

        static bool isMainFunction(SemaFunction *Sema);

        // True for a fly.runtime function with a non-empty body (emitted as a C-ABI symbol).
        static bool isCABIRuntime(SemaFunction *Sema);
    };
}

#endif //FLY_CODEGEN_FUNCTION_H
