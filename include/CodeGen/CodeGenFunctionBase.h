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
#include "llvm/ADT/SmallVector.h"
#include "AST/ASTParams.h"

namespace llvm {
    class Function;
    class FunctionType;
    class Type;
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
        llvm::FunctionType *FnTy = nullptr;
        llvm::BasicBlock *Entry = nullptr;

    public:
        CodeGenFunctionBase(CodeGenModule *CGM, ASTFunctionBase *AST);

        virtual llvm::Function *Create();

        ASTFunctionBase *getAST();

        llvm::StringRef getName() const;

        llvm::Function *getFunction();

        llvm::FunctionType *getFunctionType();

        void setInsertPoint();

        void AllocaVars();

        void GenBody();

    protected:

        void GenTypes(SmallVector<llvm::Type *, 8> &Types, const ASTParams *Params);

        CodeGenVarBase *newCodeGen(ASTVar* Var);
    };
}

#endif //FLY_CODEGEN_FUNCTIONBASE_H
