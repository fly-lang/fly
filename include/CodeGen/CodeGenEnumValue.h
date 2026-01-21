//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/Class.h - Code Generator of Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_ENUMVALUE_H
#define FLY_CODEGEN_ENUMVALUE_H

#include "CodeGenExpr.h"

#include <llvm/IR/Type.h>

namespace llvm {
    class Value;
}

namespace fly {

    class CodeGenModule;
    class SemaEnumValue;
    class SemaEnumType;

    class CodeGenEnumValue : public CodeGenExpr {

        CodeGenModule *CGM;

        llvm::Value *Value;

        llvm::Type *T;

        llvm::Value *Instance = nullptr;

        size_t Index;

    public:
        CodeGenEnumValue(CodeGenModule *CGM, SemaEnumValue *Sema);

//        llvm::AllocaInst *Alloca() override;

        llvm::Type *getType();

        llvm::Value *getValue() override;

        size_t getIndex();
    };
}

#endif //FLY_CODEGEN_ENUMVALUE_H
