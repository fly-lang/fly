//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CGFunction.h - Code Generator of Function
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CGFUNCTION_H
#define FLY_CGFUNCTION_H

#include "AST/FuncDecl.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"

namespace fly {

    class CodeGenModule;

    class CGFunction {

        CodeGenModule * CGM;
        llvm::Function *Fn;
        llvm::StringRef Name;

        llvm::FunctionType *GenFuncType(const TypeBase *RetTyData, const ParamsFuncDecl *Params);

    public:
        CGFunction(CodeGenModule *CGM, const llvm::StringRef FName, const TypeBase *FType,
                   const ParamsFuncDecl *FParams, const BlockStmt *FBlock);

        const llvm::StringRef &getName() const;

        llvm::Function *getFunction();

        void *Call();
    };
}

#endif //FLY_CGFUNCTION_H
