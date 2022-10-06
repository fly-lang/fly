//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CGVar.h - Code Generator of Statements
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CGVAR_H
#define FLY_CGVAR_H

#include "llvm/IR/Instructions.h"

namespace fly {

    class CodeGenVarBase {

    public:

        virtual void Init() = 0;

        virtual llvm::StoreInst *Store(llvm::Value *Val) = 0;

        virtual llvm::Value *Load() = 0;

        virtual llvm::Value *getValue() = 0;

        virtual llvm::Value *getPointer() = 0;
    };
}

#endif //FLY_CGVAR_H
