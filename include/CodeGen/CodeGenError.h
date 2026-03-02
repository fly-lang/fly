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

#include "CodeGenVar.h"

#include <llvm/ADT/StringRef.h>

namespace llvm {
    class StructType;
    class LLVMContext;
}

namespace fly {

    class CodeGenModule;
    class ASTExpr;
    class SemaVar;

    class CodeGenError : public CodeGenVar {

        SemaVar *Sema = nullptr;

        size_t Index;

        llvm::Value *ErrorHandler = nullptr;

        llvm::LoadInst *LoadI = nullptr;

        llvm::StringRef BlockID;

        static std::string ERROR_NAME;

    public:

        CodeGenError(CodeGenModule *CGM, SemaVar *Sema, llvm::Value *ErrorHandler);

        static llvm::StructType *GenErrorType(llvm::LLVMContext &LLVMCtx);

        llvm::Type *getType();

        llvm::StoreInst *StoreErrorHandler(llvm::Value *Val);

//        llvm::AllocaInst *Alloca() override;

        llvm::StoreInst *StoreInt(llvm::Value *Val);

        llvm::StoreInst *StoreString(llvm::Value *Val);

        llvm::StoreInst *StoreObject(llvm::Value *Val);

        size_t getIndex();

        llvm::StoreInst *Store(llvm::Value *Val);

        llvm::LoadInst *Load();

        llvm::Value *getValue() override;

        llvm::Value *getPointer();

        void setPointer(llvm::Value *Pointer);

    	void Init();
	};
}

#endif //FLY_CODEGEN_ERROR_H
