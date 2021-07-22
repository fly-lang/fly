//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CGFunction.h - Code Generator of Function
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGENFUNCTION_H
#define FLY_CODEGENFUNCTION_H

#include "AST/ASTFunc.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"

namespace fly {

    class CodeGenModule;

    class CodeGenFunction {

        CodeGenModule * CGM;
        llvm::Function *Fn;
        llvm::StringRef Name;
        llvm::BasicBlock *Entry;

        llvm::FunctionType *GenFuncType(const ASTType *RetTyData, const ASTFuncHeader *Params);

    public:
        CodeGenFunction(CodeGenModule *CGM, const llvm::StringRef FName, const ASTType *FType,
                        const ASTFuncHeader *FParams, const ASTBlock *FBody);

        const llvm::StringRef &getName() const;

        llvm::BasicBlock *getEntry();

        llvm::Function *getFunction();
    };
}

#endif //FLY_CODEGENFUNCTION_H
