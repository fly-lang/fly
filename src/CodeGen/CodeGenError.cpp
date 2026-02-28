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

#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>

using namespace fly;

std::string CodeGenError::ERROR_NAME = "error";

CodeGenError::CodeGenError(CodeGenModule *CGM, SemaVar *Sema, llvm::Value *ErrorHandler) :
    CodeGenVar(CGM, Sema, CodeGen::ErrorTy), ErrorHandler(ErrorHandler) {
}

llvm::StructType *CodeGenError::GenErrorType(llvm::LLVMContext &LLVMCtx) {
    llvm::SmallVector<llvm::Type *, 4> ErrorStructVector;
    llvm::IntegerType *Int8Ty = llvm::Type::getInt8Ty(LLVMCtx);
    ErrorStructVector.push_back(llvm::Type::getInt32Ty(LLVMCtx)); // Error Integer
    ErrorStructVector.push_back(Int8Ty->getPointerTo(0));   // Error String
	ErrorStructVector.push_back(Int8Ty->getPointerTo(0));   // Error Class Instance
    llvm::StructType *ErrorType = llvm::StructType::create(LLVMCtx, ErrorStructVector, ERROR_NAME);
	return ErrorType;
}

llvm::Type *CodeGenError::getType() {
    return T;
}

llvm::Value *CodeGenError::getPointer() {
    return ErrorHandler;
}

size_t CodeGenError::getIndex() {
	return Index;
}

llvm::StoreInst *CodeGenError::StoreErrorHandler(llvm::Value *Val) {
    return CGM->Builder->CreateStore(Val, ErrorHandler);
}

llvm::StoreInst *CodeGenError::StoreInt(llvm::Value *Val) {
    // errorType: 1=integer
	// Error: {errorInt: i32, errorString *i8, errorObject: *i8}

    // Store Error Type
	// llvm::Type *ErrorType = llvm::Type::getInt8Ty(CGM->LLVMCtx); // TODO LLVM 15
	// llvm::Value *ErrorVar = CGM->Builder->CreateLoad(ErrorType, Pointer); // TODO LLVM 15
	llvm::Value *ErrorVar = Load();

	// Store Error Value
	llvm::Value *ValuePtr = CGM->Builder->CreateInBoundsGEP(T, ErrorVar, {CodeGen::Zero, CodeGen::Zero});
	return CGM->Builder->CreateStore(Val, ValuePtr);
}

llvm::StoreInst *CodeGenError::StoreString(llvm::Value *Val) {
    this->Store(Val);
    // errorType: 2=string
    // Error: {errorInt: i32, errorPointer: *i8, errorObject: *i8}
	llvm::Value *One = llvm::ConstantInt::get(CodeGen::Int32Ty, 1);
    // Store Error Type
    llvm::Value *ErrorVar = Load();

    // Store Error Value
    llvm::Value *ValuePtr = CGM->Builder->CreateInBoundsGEP(T, ErrorVar, {CodeGen::Zero, One});
    return CGM->Builder->CreateStore(Val, ValuePtr);
}

llvm::StoreInst *CodeGenError::StoreObject(llvm::Value *Val) {
    this->Store(Val);
    // errorType: 3=object
    // Error: {errorInt: i32, errorPointer: *i8, errorObject: *i8}
    llvm::Value *Two = llvm::ConstantInt::get(CodeGen::Int32Ty, 2);
    // Store Error Type
    llvm::Value *ErrorVar = Load();

    // Store Error Value
    llvm::Value *ValuePtr = CGM->Builder->CreateInBoundsGEP(T, ErrorVar, {CodeGen::Zero, Two});
    return CGM->Builder->CreateStore(Val, ValuePtr);
}

llvm::StoreInst *CodeGenError::Store(llvm::Value *Val) {
    this->BlockID = CGM->Builder->GetInsertBlock()->getName();
    this->LoadI = nullptr;
    return nullptr;
}

llvm::LoadInst *CodeGenError::Load() {
    this->BlockID = CGM->Builder->GetInsertBlock()->getName();
    this->LoadI = CGM->Builder->CreateLoad(ErrorHandler);
    return this->LoadI;
}

llvm::Value *CodeGenError::getValue() {
    if (!this->LoadI || this->BlockID != CGM->Builder->GetInsertBlock()->getName()) {
        return Load();
    }
    return this->LoadI;
}

void CodeGenError::setPointer(llvm::Value *Pointer) {
	this->ErrorHandler = ErrorHandler;
	this->LoadI = nullptr;
}
