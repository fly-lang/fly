//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CGClass.h - Code Generator of Class
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

}

namespace fly {

    class CodeGenModule;
    class CodeGenClass;
    class ASTClassFunction;

    class CodeGenClassFunction : public CodeGenFunctionBase {

        friend class CodeGenClass;

        CodeGenClassFunction(CodeGenModule *CGM, ASTClassFunction *AST);

    public:
        llvm::Function *Create() override;
    };
}

#endif //FLY_CODEGEN_CLASSFUNCTION_H
