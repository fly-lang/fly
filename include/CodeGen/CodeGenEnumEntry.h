//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenEnumEntry.h - Code Generator of Enum Entry
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_ENUMENTRY_H
#define FLY_CODEGEN_ENUMENTRY_H

#include "CodeGenExpr.h"

#include <llvm/IR/Type.h>

namespace llvm {
    class Value;
}

namespace fly {

    class CodeGenModule;
    class SemaEnumEntry;
    class SemaEnumType;

    class CodeGenEnumEntry : public CodeGenExpr {

        CodeGenModule *CGM;

        llvm::Value *Value;

        llvm::Type *T;

        llvm::Value *Instance = nullptr;

        size_t Index;

    public:
        CodeGenEnumEntry(CodeGenModule *CGM, SemaEnumEntry *Sema);

//        llvm::AllocaInst *Alloca() override;

        llvm::Type *getType();

        llvm::Value *getValue() override;

        size_t getIndex();
    };
}

#endif //FLY_CODEGEN_ENUMENTRY_H
