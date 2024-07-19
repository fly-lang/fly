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

        llvm::Value *Instance = nullptr;

        llvm::LoadInst *LoadI = nullptr;

        llvm::StringRef BlockID;

    public:

        CodeGenError(CodeGenModule *CGM, ASTVar *Error, llvm::Value *Pointer);

        static llvm::StructType *GenErrorType(llvm::LLVMContext &LLVMCtx);

        llvm::Type *getType() override;

        llvm::StoreInst *StorePointer(llvm::Value *Val);

//        llvm::AllocaInst *Alloca() override;

        llvm::StoreInst *StoreDefault();

        llvm::StoreInst *StoreInt(llvm::Value *Val);

        llvm::StoreInst *StoreString(llvm::Value *Val);

        llvm::StoreInst *StoreObject(llvm::Value *Val);

        llvm::StoreInst *Store(llvm::Value *Val) override;

        llvm::LoadInst *Load() override;

        llvm::Value *getValue() override;

        llvm::Value *getPointer() override;

        void Store(ASTExpr *Expr);

        CodeGenVarBase *getVar(llvm::StringRef Name);
    };
}

#endif //FLY_CODEGEN_ERROR_H
