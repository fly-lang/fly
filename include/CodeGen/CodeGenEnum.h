//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenEnum.h - Code Generator of Enum
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_CODEGEN_ENUM_H
#define FLY_CODEGEN_ENUM_H

#include "llvm/ADT/StringMap.h"

namespace llvm {
    class Value;
}

namespace fly {

    class CodeGenModule;
    class ASTClass;
    class CodeGenVar;

    class CodeGenEnum {

        CodeGenModule * CGM = nullptr;

        ASTClass *AST = nullptr;

        llvm::StringMap<llvm::Value *> Vars;

    public:
        CodeGenEnum(CodeGenModule *CGM, ASTClass *Class, bool isExternal = false);
    };
}

#endif //FLY_CODEGEN_ENUM_H
