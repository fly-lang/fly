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
    class StructType;
}

namespace fly {

    class ASTFunctionBase;
    class ASTParams;
    class ASTType;
    class CodeGenModule;

    class CodeGenFunctionBase {

    protected:
        ASTFunctionBase *AST = nullptr;
        CodeGenModule *CGM = nullptr;
        llvm::Function *Fn = nullptr;
        llvm::Type *RetType = nullptr;
        llvm::FunctionType *FnType = nullptr;
        llvm::BasicBlock *Entry = nullptr;
        llvm::Value *ErrorVar = nullptr;

    public:
        CodeGenFunctionBase(CodeGenModule *CGM, ASTFunctionBase *AST);

        CodeGenModule *getCodeGenModule();

        void GenReturnType();

        void GenParamTypes(CodeGenModule * CGM, SmallVector<llvm::Type *, 8> &Types, const ASTParams *Params);

        ASTFunctionBase *getAST();

        llvm::StringRef getName() const;

        llvm::Function *getFunction();

        llvm::FunctionType *getFunctionType();

        void setInsertPoint();

        void AllocaVars();

        void StoreParams(bool isMain);

        llvm::Value *getErrorVar();

        virtual void GenBody() = 0;
    };
}

#endif //FLY_CODEGEN_FUNCTIONBASE_H
