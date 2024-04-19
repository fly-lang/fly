//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/Class.h - Code Generator of Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_ENUMENTRY_H
#define FLY_CODEGEN_ENUMENTRY_H

#include "CodeGenVarBase.h"

namespace llvm {
    class Value;
}

namespace fly {

    class CodeGenModule;
    class CodeGenEnum;
    class ASTEnumEntry;

    class CodeGenEnumEntry : public CodeGenVarBase {

        friend class CodeGenEnum;

        llvm::Value *Value;

    public:
        CodeGenEnumEntry(CodeGenModule *CGM, ASTEnumEntry *EnumEntry);

        llvm::Value *getValue() override;

        void Init() override;

        llvm::StoreInst *Store(llvm::Value *Val) override;

        llvm::LoadInst *Load() override;

        llvm::Value *getPointer() override;

        ASTVar *getVar() override;
    };
}

#endif //FLY_CODEGEN_ENUMENTRY_H
