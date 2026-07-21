//===--------------------------------------------------------------------------------------------------------------===//
// compiler/CodeGen/CodeGenError.cpp - error/fail code generation
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
    // Extract ptr and size from the string struct { i8*, i32 }
    llvm::Value *Ptr = CGM->Builder->CreateExtractValue(Val, 0);
    llvm::Value *Size = CGM->Builder->CreateExtractValue(Val, 1);
    this->Store(Ptr);

    // The error struct carries only the pointer, so the message must be a
    // NUL-terminated C string (err_print's contract) and must outlive the
    // failing function's locals (fail frees them via EmitAllocCleanup).
    // Copy it into a fresh malloc'd buffer with a terminator: malloc(size+1),
    // memcpy, buf[size] = 0. Leaked by design — the error path ends the run.
    llvm::Value *SizeExt = CGM->Builder->CreateZExt(Size, CodeGen::IntPtrTy, "err_msg_size");
    llvm::Value *BufSize = CGM->Builder->CreateAdd(SizeExt,
        llvm::ConstantInt::get(CodeGen::IntPtrTy, 1), "err_buf_size");
    llvm::FunctionCallee MallocFn = CGM->Module->getOrInsertFunction(
        "malloc",
        llvm::FunctionType::get(
            llvm::PointerType::getUnqual(CGM->LLVMCtx),
            {CodeGen::IntPtrTy}, false));
    llvm::Value *Buf = CGM->Builder->CreateCall(MallocFn, {BufSize}, "err_msg");
    CGM->Builder->CreateMemCpy(Buf, llvm::MaybeAlign(), Ptr, llvm::MaybeAlign(), SizeExt);
    llvm::Value *End = CGM->Builder->CreateGEP(CodeGen::Int8Ty, Buf, SizeExt, "err_msg_end");
    CGM->Builder->CreateStore(llvm::ConstantInt::get(CodeGen::Int8Ty, 0), End);

    // Error: {errorInt: i32, errorPointer: *i8, errorObject: *i8}
    llvm::Value *One = llvm::ConstantInt::get(CodeGen::Int32Ty, 1);
    llvm::Value *ErrorVar = Load();
    llvm::Value *ValuePtr = CGM->Builder->CreateInBoundsGEP(T, ErrorVar, {CodeGen::Zero, One});
    return CGM->Builder->CreateStore(Buf, ValuePtr);
}

llvm::StoreInst *CodeGenError::StoreObject(llvm::Value *Val) {
    // Error: {errorInt: i32, errorString: ptr, errorObject: ptr}
    // The integer field is NOT touched here: visit(SemaFailStmt) defaults it
    // to 1 when no explicit code is given, and must not clobber an explicit
    // one (`fail 404, new Ctx()` keeps 404).
    llvm::Value *Two = llvm::ConstantInt::get(CodeGen::Int32Ty, 2);
    llvm::Value *ValuePtr = CGM->Builder->CreateInBoundsGEP(T, getValue(), {CodeGen::Zero, Two});
    return CGM->Builder->CreateStore(Val, ValuePtr);
}

llvm::StoreInst *CodeGenError::Store(llvm::Value *Val) {
    this->LoadBlock = nullptr;
    this->LoadI = nullptr;
    return nullptr;
}

llvm::LoadInst *CodeGenError::Load() {
    this->LoadBlock = CGM->Builder->GetInsertBlock();
    this->LoadI = CGM->Builder->CreateLoad(CodeGen::ErrorPtrTy, ErrorHandler);
    return this->LoadI;
}

llvm::Value *CodeGenError::getValue() {
    if (!this->LoadI || this->LoadBlock != CGM->Builder->GetInsertBlock()) {
        return Load();
    }
    return this->LoadI;
}

void CodeGenError::setPointer(llvm::Value *Pointer) {
	this->ErrorHandler = ErrorHandler;
	this->LoadI = nullptr;
}

void CodeGenError::Init() {
	// Set default values for error struct: 0 for int, null for pointer
	llvm::Constant *Zero = llvm::ConstantInt::get(CodeGen::Int32Ty, 0);
	llvm::Constant *One = llvm::ConstantInt::get(CodeGen::Int32Ty, 1);
	llvm::Constant *Two = llvm::ConstantInt::get(CodeGen::Int32Ty, 2);
	llvm::Constant *NullPtr = llvm::ConstantPointerNull::get(CodeGen::Int8Ty->getPointerTo());

	// Load the error struct pointer
	llvm::Value *ErrorVar = Load();

	// Store default values
	llvm::Value *PtrInt = CGM->Builder->CreateInBoundsGEP(T, ErrorVar, {Zero, Zero});
	CGM->Builder->CreateStore(Zero, PtrInt);
	llvm::Value *PtrStr = CGM->Builder->CreateInBoundsGEP(T, ErrorVar, {Zero, One});
	CGM->Builder->CreateStore(NullPtr, PtrStr);
	llvm::Value *PtrPtr = CGM->Builder->CreateInBoundsGEP(T, ErrorVar, {Zero, Two});
	CGM->Builder->CreateStore(NullPtr, PtrPtr);
}
