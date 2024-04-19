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
#include "Sema/SemaBuilder.h"
#include "AST/ASTFunctionBase.h"
#include "AST/ASTCall.h"
#include "AST/ASTType.h"
#include "AST/ASTError.h"

using namespace fly;

llvm::StructType *CodeGenFail::GenErrorType(CodeGenModule *CGM) {
    llvm::SmallVector<llvm::Type *, 4> ErrorStructVector;
    ErrorStructVector.push_back(CGM->Int8Ty); // Error Type: 0=none, 1=integer, 2=string, 3=struct, 4=class
    ErrorStructVector.push_back(CGM->Int32Ty); // Error Integer
    ErrorStructVector.push_back(CGM->Int8PtrTy); // Error String or StructType or Class Instance
    return llvm::StructType::create(CGM->LLVMCtx, ErrorStructVector, "error");
}

void CodeGenFail::GenSTMT(CodeGenFunctionBase *CGF, ASTCall *Call) {
    CodeGenModule *CGM = CGF->getCodeGenModule();

    // set error param
    llvm::Value *ErrorVar = CGF->getErrorVar();
    llvm::Value *Zero = llvm::ConstantInt::get(CGM->Int32Ty, 0);
    llvm::Value *ErrorKind = CGM->Builder->CreateInBoundsGEP(CGM->ErrorType, ErrorVar, {Zero, Zero});

    const std::vector<ASTArg *> &Args = Call->getArgs();    
    if (Args.empty()) {
        CGM->Builder->CreateStore(llvm::ConstantInt::get(CGM->Int8Ty, 1), ErrorKind);
    } else if (Args[0]->getExpr()->getType()->isBool()) {
        llvm::Value *Val = CGM->GenExpr(CGF, SemaBuilder::CreateByteType(SourceLocation()),Args[0]->getExpr());
        CGM->Builder->CreateStore(Val, ErrorKind);
    } else if (Args[0]->getExpr()->getType()->isInteger()) {
        llvm::Value *Val = CGM->GenExpr(CGF, SemaBuilder::CreateIntType(SourceLocation()), Args[0]->getExpr());
        CGM->Builder->CreateStore(llvm::ConstantInt::get(CGM->Int8Ty, 2), ErrorKind);
        llvm::Value *NumberPtr = CGM->Builder->CreateInBoundsGEP(CGM->ErrorType, ErrorVar,
                                                                 {Zero, llvm::ConstantInt::get(CGM->Int32Ty, 1)});
        CGM->Builder->CreateStore(Val, NumberPtr);
    } else if (Args[0]->getExpr()->getType()->isString()) {
        llvm::Value *Val = CGM->GenExpr(CGF, Args[0]->getExpr());
        CGM->Builder->CreateStore(llvm::ConstantInt::get(CGM->Int8Ty, 3), ErrorKind);
        llvm::Value *StringPtr = CGM->Builder->CreateInBoundsGEP(CGM->ErrorType, ErrorVar,
                                                                 {Zero, llvm::ConstantInt::get(CGM->Int32Ty, 2)});
        CGM->Builder->CreateStore(Val, StringPtr);
    } else if (Args[0]->getExpr()->getType()->isIdentity()) {
        CGM->Builder->CreateStore(llvm::ConstantInt::get(CGM->Int8Ty, 4), ErrorKind);
        llvm::Value *ClassPtr = CGM->Builder->CreateInBoundsGEP(CGM->ErrorType, ErrorVar, {Zero, llvm::ConstantInt::get(CGM->Int32Ty, 2)});
    } else {
        // Error:
    }

    // Return null value
    llvm::Type *LLVMReturnType = CGF->getFunctionType()->getReturnType();
    llvm::Value *LLVMReturnValue = llvm::Constant::getNullValue(LLVMReturnType);
    CGM->Builder->CreateRet(LLVMReturnValue);
}
