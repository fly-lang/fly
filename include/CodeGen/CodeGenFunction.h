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

#include "AST/ASTFunction.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"

namespace fly {

    class CodeGenModule;

    class CodeGenFunction {

        CodeGenModule * CGM;
        ASTFunction *AST;
        llvm::Function *Fn;
        llvm::StringRef Name;
        llvm::BasicBlock *Entry;

    public:
        CodeGenFunction(CodeGenModule *CGM, ASTFunction *AST, bool isExternal = false);

        const llvm::StringRef &getName() const;

        llvm::Function *getFunction();

        void GenBody();

    private:
        llvm::FunctionType *GenFuncType(const ASTType *RetTyData, const ASTParams *Params);

    };
}

#endif //FLY_CODEGENFUNCTION_H
