//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenFail.cpp - Code Generator Fail
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenFail.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenFunction.h"
#include "AST/ASTFunctionBase.h"
#include "AST/ASTCall.h"
#include "AST/ASTType.h"
#include "AST/ASTError.h"

using namespace fly;

llvm::StructType *CodeGenFail::GenErrorType(CodeGenModule *CGM) {
    llvm::SmallVector<llvm::Type *, 4> ErrorStructVector;
    ErrorStructVector.push_back(CGM->Int8Ty); // Error Type: 0=empty, 1=integer, 2=string, 3=struct
    ErrorStructVector.push_back(CGM->Int32Ty); // Error Integer
    ErrorStructVector.push_back(CGM->Int8PtrTy); // Error String or Struct
    return llvm::StructType::create(CGM->LLVMCtx, ErrorStructVector, "error");
}

void CodeGenFail::GenSTMT(CodeGenModule *CGM, ASTCall *Call) {
    CodeGenFunctionBase *CGF = ((ASTBlock *) Call->getParent())->getTop()->getCodeGen();

    // set error param
    llvm::Value *ErrorVar = CGF->getErrorVar();
    llvm::Value *Zero = llvm::ConstantInt::get(CGM->Int32Ty, 0);
    llvm::Value *ErrorKind = CGM->Builder->CreateInBoundsGEP(CGM->ErrorType, ErrorVar, {Zero, Zero});

    const std::vector<ASTArg *> &Args = Call->getArgs();    
    if (Args.empty()) {
        
    } else if (Args[0]->getExpr()->getType()->isInteger()) {
        llvm::Value *Val = CGM->GenExpr(CGF, Args[0]->getExpr()->getType(), Args[0]->getExpr());
        CGM->Builder->CreateStore(llvm::ConstantInt::get(CGM->Int8Ty, 1), ErrorKind);
        llvm::Value *NumberPtr = CGM->Builder->CreateInBoundsGEP(CGM->ErrorType, ErrorVar,
                                                                 {Zero, llvm::ConstantInt::get(CGM->Int32Ty, 1)});
        CGM->Builder->CreateStore(Val, NumberPtr);
    } else if (Args[0]->getExpr()->getType()->isString()) {
        llvm::Value *Val = CGM->GenExpr(CGF, Args[0]->getExpr()->getType(), Args[0]->getExpr());
        CGM->Builder->CreateStore(llvm::ConstantInt::get(CGM->Int8Ty, 2), ErrorKind);
        llvm::Value *StringPtr = CGM->Builder->CreateInBoundsGEP(CGM->ErrorType, ErrorVar,
                                                                 {Zero, llvm::ConstantInt::get(CGM->Int32Ty, 2)});
        CGM->Builder->CreateStore(Val, StringPtr);
    }
    // else if () {
    // CGM->Builder->CreateStore(llvm::ConstantInt::get(CGM->Int8Ty, 3), ErrorKind);
    // llvm::Value *ClassPtr = CGM->Builder->CreateInBoundsGEP(CGM->ErrorType, ErrorVar, {Zero, llvm::ConstantInt::get(CGM->Int32Ty, 2)});
    // }
    else {
        // Error:
    }

    // Return null value
    llvm::Type *LLVMReturnType = CGF->getFunctionType()->getReturnType();
    llvm::Value *LLVMReturnValue = llvm::Constant::getNullValue(LLVMReturnType);
    CGM->Builder->CreateRet(LLVMReturnValue);
}
