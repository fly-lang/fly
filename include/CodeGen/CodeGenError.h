//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenFail.h - Code Generator of Fail
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_CODEGEN_ERROR_H
#define FLY_CODEGEN_ERROR_H

#include "CodeGenVarBase.h"
#include "llvm/IR/DerivedTypes.h"

namespace fly {

    class CodeGenModule;
    class CodeGenFunctionBase;
    class ASTExpr;
    class ASTVar;

    class CodeGenError : public CodeGenVarBase {

        CodeGenModule *CGM = nullptr;

        ASTVar *Error = nullptr;

        llvm::Type *T = nullptr;

        llvm::Value *Pointer = nullptr;

    public:

        CodeGenError(CodeGenModule *CGM, ASTVar *Error);

        static llvm::StructType *GenErrorType(llvm::LLVMContext &LLVMCtx);

        void Init() override;

        llvm::Type *getType();

        llvm::StoreInst *Store(llvm::Value *Val) override;

        llvm::LoadInst *Load() override;

        llvm::Value *getValue() override;

        llvm::Value *getPointer() override;

        ASTVar *getVar() override;

        void Store(ASTExpr *Expr);
    };
}

#endif //FLY_CODEGEN_ERROR_H
