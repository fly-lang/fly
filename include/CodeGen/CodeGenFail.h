//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenFail.h - Code Generator of Fail
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_CODEGEN_FAIL_H
#define FLY_CODEGEN_FAIL_H

#include "llvm/IR/DerivedTypes.h"

namespace fly {

    class CodeGenModule;
    class ASTCall;

    class CodeGenFail {

    public:

        static llvm::StructType *GenErrorType(CodeGenModule *CGM);

        static void GenSTMT(CodeGenModule *CGM, ASTCall *Call);
    };
}

#endif //FLY_CODEGEN_FAIL_H
