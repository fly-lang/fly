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
    class Type;
}

namespace fly {

    class CodeGenModule;
    class ASTEnum;
    class CodeGenEnumEntry;

    class CodeGenEnum {

        CodeGenModule * CGM = nullptr;

        llvm::Type *Type = nullptr;

        ASTEnum *AST = nullptr;

        llvm::StringMap<CodeGenEnumEntry *> Vars;

    public:
        CodeGenEnum(CodeGenModule *CGM, ASTEnum *Enum, bool isExternal = false);

        void Generate();

        llvm::Type *getType() const;
    };
}

#endif //FLY_CODEGEN_ENUM_H
