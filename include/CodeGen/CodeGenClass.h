//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CGClass.h - Code Generator of Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_CLASS_H
#define FLY_CODEGEN_CLASS_H

namespace llvm {
    class StringRef;
}

namespace fly {

    class CodeGenModule;
    class ASTClass;

    class CodeGenClass {

        CodeGenModule * CGM = nullptr;
        ASTClass *AST = nullptr;

    public:
        CodeGenClass(CodeGenModule *CGM, ASTClass *AST, bool isExternal = false);

//        const llvm::StringRef getName() const;

    };
}

#endif //FLY_CODEGEN_CLASS_H
