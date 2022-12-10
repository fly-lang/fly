//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CGFunction.h - Code Generator of Function
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_FUNCTIONBASE_H
#define FLY_CODEGEN_FUNCTIONBASE_H

#include "llvm/ADT/StringRef.h"

namespace llvm {
    class Function;
    class FunctionType;
    class BasicBlock;
}

namespace fly {

    class ASTFunctionBase;
    class ASTParams;
    class ASTType;
    class CodeGenModule;

    class CodeGenFunctionBase {

        ASTFunctionBase *AST = nullptr;

    protected:
        CodeGenModule * CGM = nullptr;
        llvm::Function *Fn = nullptr;
        llvm::BasicBlock *Entry = nullptr;

    public:
        CodeGenFunctionBase(CodeGenModule *CGM, ASTFunctionBase *AST);

        llvm::StringRef getName() const;

        llvm::Function *getFunction();

        void GenBody();

    private:
        llvm::FunctionType *GenFuncType(const ASTType *RetTyData, const ASTParams *Params);

    };
}

#endif //FLY_CODEGEN_FUNCTIONBASE_H
