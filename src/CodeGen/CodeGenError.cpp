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
#include "AST/ASTIdentityType.h"
#include "AST/ASTExpr.h"
#include "AST/ASTVar.h"

using namespace fly;

CodeGenError::CodeGenError(CodeGenModule *CGM, ASTVar *Error, llvm::Value *Pointer) :
    CGM(CGM), Error(Error), Pointer(Pointer), T(CGM->ErrorTy) {

}

llvm::StructType *CodeGenError::GenErrorType(llvm::LLVMContext &LLVMCtx) {
    llvm::SmallVector<llvm::Type *, 4> ErrorStructVector;
    llvm::IntegerType *Int8Ty = llvm::Type::getInt8Ty(LLVMCtx);
    ErrorStructVector.push_back(Int8Ty); // Error Type: 1=integer, 2=string, 3=enum, 4=class
    ErrorStructVector.push_back(llvm::Type::getInt32Ty(LLVMCtx)); // Error Integer
    ErrorStructVector.push_back(Int8Ty->getPointerTo(0)); // Error String or Class Instance
    return llvm::StructType::create(LLVMCtx, ErrorStructVector, "error");
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

llvm::StoreInst *CodeGenError::StoreDefault() {
    // Error Type: 1=integer
    // Error: {errorType: i8, errorInt: i32, errorPointer: *i8}
    ConstantInt *TypeValue = llvm::ConstantInt::get(CGM->Int8Ty, 1);
    llvm::Value *Zero = llvm::ConstantInt::get(CGM->Int32Ty, 0);
    llvm::Value *One = llvm::ConstantInt::get(CGM->Int32Ty, 1);
    // Store Error Type
    llvm::Value *ErrorVar = CGM->Builder->CreateLoad(Pointer);
    llvm::Value *TypeValuePtr = CGM->Builder->CreateInBoundsGEP(T, ErrorVar, {Zero, Zero});
    return CGM->Builder->CreateStore(TypeValue, TypeValuePtr);
}

llvm::StoreInst *CodeGenError::StoreInt(llvm::Value *Val) {
    this->Store(Val);
    // Error Type: 1=integer
    // Error: {errorType: i8, errorInt: i32, errorPointer: *i8}
    ConstantInt *TypeValue = llvm::ConstantInt::get(CGM->Int8Ty, 1);
    llvm::Value *Zero = llvm::ConstantInt::get(CGM->Int32Ty, 0);
    llvm::Value *One = llvm::ConstantInt::get(CGM->Int32Ty, 1);
    // Store Error Type
    llvm::Value *ErrorVar = CGM->Builder->CreateLoad(Pointer);
    llvm::Value *TypeValuePtr = CGM->Builder->CreateInBoundsGEP(T, ErrorVar, {Zero, Zero});
    CGM->Builder->CreateStore(TypeValue, TypeValuePtr);
    // Store Error Value
    llvm::Value *ValuePtr = CGM->Builder->CreateInBoundsGEP(T, ErrorVar, {Zero, One});
    return CGM->Builder->CreateStore(Val, ValuePtr);
}

llvm::StoreInst *CodeGenError::StoreString(llvm::Value *Val) {
    this->Store(Val);
    // Error Type: 2=string
    // Error: {errorType: i8, errorInt: i32, errorPointer: *i8}
    ConstantInt *TypeValue = llvm::ConstantInt::get(CGM->Int8Ty, 2);
    llvm::Value *Zero = llvm::ConstantInt::get(CGM->Int32Ty, 0);
    llvm::Value *Two = llvm::ConstantInt::get(CGM->Int32Ty, 2);
    // Store Error Type
    llvm::Value *ErrorVar = CGM->Builder->CreateLoad(Pointer);
    llvm::Value *TypeValuePtr = CGM->Builder->CreateInBoundsGEP(T, ErrorVar, {Zero, Zero});
    CGM->Builder->CreateStore(TypeValue, TypeValuePtr);
    // Store Error Value
    llvm::Value *ValuePtr = CGM->Builder->CreateInBoundsGEP(T, ErrorVar, {Zero, Two});
    return CGM->Builder->CreateStore(Val, ValuePtr);
}

llvm::StoreInst *CodeGenError::StoreObject(llvm::Value *Val) {
    this->Store(Val);
    // Error Type: 3=object
    // Error: {errorType: i8, errorInt: i32, errorPointer: *i8}
    ConstantInt *TypeValue = llvm::ConstantInt::get(CGM->Int8Ty, 3);
    llvm::Value *Zero = llvm::ConstantInt::get(CGM->Int32Ty, 0);
    llvm::Value *Two = llvm::ConstantInt::get(CGM->Int32Ty, 2);
    // Store Error Type
    llvm::Value *ErrorVar = CGM->Builder->CreateLoad(Pointer);
    llvm::Value *TypeValuePtr = CGM->Builder->CreateInBoundsGEP(T, ErrorVar, {Zero, Zero});
    CGM->Builder->CreateStore(TypeValue, TypeValuePtr);
    // Store Error Value
    llvm::Value *ValuePtr = CGM->Builder->CreateInBoundsGEP(T, ErrorVar, {Zero, Two});
    return CGM->Builder->CreateStore(Val, ValuePtr);
}

llvm::StoreInst *CodeGenError::Store(llvm::Value *Val) {
    this->BlockID = CGM->Builder->GetInsertBlock()->getName();
    this->LoadI = nullptr;
    return nullptr;
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

CodeGenVarBase *CodeGenError::getVar(llvm::StringRef Name) {
    return nullptr;
}
