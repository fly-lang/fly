//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CGGlobalVar.h - Code Generator of Global Var
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CGGLOBALVAR_H
#define FLY_CGGLOBALVAR_H

#include <AST/TypeBase.h>
#include "llvm/IR/GlobalVariable.h"
#include "llvm/ADT/StringRef.h"

namespace fly {

    class CodeGenModule;

    class CGGlobalVar {

        llvm::GlobalVariable *GVar;

    public:
        CGGlobalVar(CodeGenModule *CGM, const TypeBase *Ty, llvm::StringRef StrVal, const bool isConstant);

        llvm::GlobalVariable *getGlobalVar() const;
    };
}

#endif //FLY_CGGLOBALVAR_H
