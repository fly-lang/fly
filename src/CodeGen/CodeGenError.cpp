//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenFail.cpp - Code Generator Fail
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenError.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenFunction.h"
#include "AST/ASTFunctionBase.h"
#include "AST/ASTIdentity.h"
#include "AST/ASTIdentityType.h"
#include "AST/ASTExpr.h"
#include "AST/ASTVar.h"

using namespace fly;

CodeGenError::CodeGenError(CodeGenModule *CGM, ASTVar *Error) : CGM(CGM), Error(Error), T(CGM->ErrorPtrTy) {

}

llvm::StructType *CodeGenError::GenErrorType(llvm::LLVMContext &LLVMCtx) {
    llvm::SmallVector<llvm::Type *, 4> ErrorStructVector;
    llvm::IntegerType *Int8Ty = llvm::Type::getInt8Ty(LLVMCtx);
    ErrorStructVector.push_back(Int8Ty); // Error Type: 1=integer, 2=string, 3=enum, 4=class
    ErrorStructVector.push_back(llvm::Type::getInt32Ty(LLVMCtx)); // Error Integer
    ErrorStructVector.push_back(Int8Ty->getPointerTo(0)); // Error String or Class Instance
    return llvm::StructType::create(LLVMCtx, ErrorStructVector, "error");
}

void CodeGenError::Init() {
    Pointer = CGM->Builder->CreateAlloca(T);
}

llvm::Type *CodeGenError::getType() {
    return T;
}

llvm::Value *CodeGenError::getPointer() {
    return Pointer;
}

llvm::StoreInst *CodeGenError::StorePointer(llvm::Value *Val) {
    return CGM->Builder->CreateStore(Val, Pointer);
}

llvm::StoreInst *CodeGenError::Store(llvm::Value *Val) {
    this->BlockID = CGM->Builder->GetInsertBlock()->getName();
    this->LoadI = nullptr;

    // Add error param
    llvm::Value *ErrorVar = CGM->Builder->CreateLoad(Pointer);
    llvm::Value *Zero = llvm::ConstantInt::get(CGM->Int32Ty, 0);
    llvm::Value *TypeValue = nullptr;

    if (Val == nullptr) {
        // Error Type: 1=integer
        TypeValue = llvm::ConstantInt::get(CGM->Int32Ty, 1);

        // Error Value: 1
        Val = llvm::ConstantInt::get(CGM->Int32Ty, 1);
    } else if (Val->getType()->isIntegerTy()) {
        // Error Type: 1=integer
        TypeValue = llvm::ConstantInt::get(CGM->Int32Ty, 1);

        // Error Value: IntVal
        llvm::ArrayRef<Value *> IdxList = {Zero, llvm::ConstantInt::get(CGM->Int32Ty, 1)};
        llvm::Value *Ptr = CGM->Builder->CreateInBoundsGEP(T, ErrorVar, IdxList);
        CGM->Builder->CreateStore(Val, Ptr);
    } else if (Val->getType()->isPointerTy()) {
        llvm::ArrayRef<Value *> IdxList = {Zero, llvm::ConstantInt::get(CGM->Int8PtrTy, 2)};
        llvm::Value *Ptr = CGM->Builder->CreateInBoundsGEP(T, ErrorVar, IdxList);
        CGM->Builder->CreateStore(Val, Ptr);
    }

    // store error type
    llvm::Value *TypeValuePtr = CGM->Builder->CreateInBoundsGEP(T, ErrorVar, {Zero, Zero});
    return CGM->Builder->CreateStore(TypeValue, TypeValuePtr);
}

llvm::LoadInst *CodeGenError::Load() {
    this->BlockID = CGM->Builder->GetInsertBlock()->getName();
    this->LoadI = CGM->Builder->CreateLoad(Pointer);
    return this->LoadI;
}

llvm::Value *CodeGenError::getValue() {
    if (!this->LoadI || this->BlockID != CGM->Builder->GetInsertBlock()->getName()) {
        return Load();
    }
    return this->LoadI;
}

ASTVar *CodeGenError::getVar() {
    return Error;
}

void CodeGenError::Store(ASTExpr *Expr) {

    // Add error param
    llvm::Value *ErrorVar = CGM->Builder->CreateLoad(Pointer);
    llvm::Value *Zero = llvm::ConstantInt::get(CGM->Int32Ty, 0);
    llvm::Value *TypeValue = nullptr;
    llvm::Value *IntValue = nullptr;
    llvm::Value *PtrValue = nullptr;

    if (Expr == nullptr || Expr->getType()->isBool()) {
        // Error Type: 1=integer
        TypeValue = llvm::ConstantInt::get(CGM->Int32Ty, 1);

        // Error Value: 1
        IntValue = llvm::ConstantInt::get(CGM->Int32Ty, 1);

    } else if (Expr->getType()->isInteger()) {
        // Error Type: 1=integer
        TypeValue = llvm::ConstantInt::get(CGM->Int32Ty, 1);

        // Error Value: IntVal
        IntValue = CGM->GenExpr(Expr);

    } else if (Expr->getType()->isString()) {
        // Error Type: 2=string
        TypeValue = llvm::ConstantInt::get(CGM->Int32Ty, 2);

        // Error Value: PtrVal
        PtrValue = CGM->GenExpr(Expr);

    } else if (Expr->getType()->isIdentity()) {
        ASTIdentity * Identity = (ASTIdentity *) Expr->getType();
        if (Identity->getTopDefKind() == ASTTopDefKind::DEF_ENUM) {
            // Error Type: 3=enum
            TypeValue = llvm::ConstantInt::get(CGM->Int32Ty, 3);

            // Error Value: PtrVal
            PtrValue = CGM->GenExpr(Expr); // FIXME need to store enum type and enum value
        } else if (Identity->getTopDefKind() == ASTTopDefKind::DEF_CLASS) {
            // Error Type: 4=class
            TypeValue = llvm::ConstantInt::get(CGM->Int32Ty, 4);

            // Error Value: PtrVal
            PtrValue = CGM->GenExpr(Expr);
        } else {
            // Error: unsupported identity type
        }
    } else {
        // Error: unsupported type
    }

    // store error type
    llvm::Value *TypeValuePtr = CGM->Builder->CreateInBoundsGEP(T, ErrorVar, {Zero, Zero});
    CGM->Builder->CreateStore(TypeValue, TypeValuePtr);

    // store error int value
    if (IntValue) {
        llvm::ArrayRef<Value *> IdxList = {Zero, llvm::ConstantInt::get(CGM->Int32Ty, 1)};
        llvm::Value *Ptr = CGM->Builder->CreateInBoundsGEP(T, ErrorVar, IdxList);
        CGM->Builder->CreateStore(IntValue, Ptr);
    }

    // store error pointer value
    if (PtrValue) {
        llvm::ArrayRef<Value *> IdxList = {Zero, llvm::ConstantInt::get(CGM->Int8PtrTy, 2)};
        llvm::Value *Ptr = CGM->Builder->CreateInBoundsGEP(T, ErrorVar, IdxList);
        CGM->Builder->CreateStore(PtrValue, Ptr);
    }
}
