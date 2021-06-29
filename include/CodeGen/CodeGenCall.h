//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CGFunction.h - Code Generator of Function
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGENCALL_H
#define FLY_CODEGENCALL_H

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"

namespace fly {

    class CodeGenModule;

    class CodeGenCall {

        CodeGenModule * CGM;
        llvm::Function *Fn;
        const llvm::ArrayRef<llvm::Value *> Args;

    public:
        CodeGenCall(CodeGenModule *CGM, llvm::Function *Fn, const llvm::ArrayRef<llvm::Value *> &Args);

        llvm::CallInst *Call() const;
    };
}

#endif //FLY_CODEGENCALL_H
