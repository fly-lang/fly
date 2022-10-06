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

namespace llvm {

}

namespace fly {

    class CodeGenModule;
    class CodeGenClass;
    class ASTClassFunction;

    class CodeGenClassFunction {

        friend class CodeGenClass;

        CodeGenModule * CGM = nullptr;

        ASTClassFunction *AST = nullptr;
    };
}

#endif //FLY_CODEGEN_CLASSFUNCTION_H
