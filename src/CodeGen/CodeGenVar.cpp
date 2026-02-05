//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGVar.cpp - Code Generator Block implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenVar.h"

#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"

#include <Sema/SemaCall.h>
#include <Sema/SemaClassAttribute.h>
#include <Sema/SemaClassInstance.h>
#include <Sema/SemaMember.h>
#include <Sema/SemaVar.h>
#include <llvm/IR/Instructions.h>

using namespace fly;

CodeGenVar::CodeGenVar(CodeGenModule *CGM, SemaVar *Sema, llvm::Type *T) : CodeGenExpr(CGM),
	Sema(Sema), T(T) {

}

CodeGenVar::CodeGenVar(CodeGenModule *CGM, SemaVar *Sema, llvm::Type *T, size_t Index) : CodeGenExpr(CGM),
	Sema(Sema), T(T), Index(Index) {

}

llvm::Type *CodeGenVar::getType() {
    return T;
}

llvm::AllocaInst *CodeGenVar::Alloca() {
	// Array, Class, Struct
	if (T->isStructTy()) {
		// Alloca pointer to struct types (except CodeGen::ArrayTy)
		llvm::PointerType *PtrTy = T->getPointerTo(CGM->Module->getDataLayout().getAllocaAddrSpace());
		this->Pointer = CGM->Builder->CreateAlloca(PtrTy);
	} else {
		// Alloca for non-struct types
		// Check if the type is bool (i1) and convert it to i8
		this->Pointer = CGM->Builder->CreateAlloca(T->isIntegerTy(1) ? CodeGen::Int8Ty : T);
	}
	return llvm::cast<llvm::AllocaInst>(this->Pointer);
}

llvm::StoreInst *CodeGenVar::Store(llvm::Value *Val) {
    this->BlockID = CGM->Builder->GetInsertBlock()->getName();
    this->LoadI = nullptr;

    // Fix Architecture Compatibility of bool i1 to i8
    if (T->isIntegerTy(1)) {
        Val = CGM->Builder->CreateZExt(Val, CodeGen::Int8Ty);
    }

	return CGM->Builder->CreateStore(Val, getPointer());
}

llvm::StoreInst *CodeGenVar::StoreArrayValue(llvm::Value *ArrayPtr, const std::vector<llvm::Value *> &Values, llvm::Type *ElementType) {
	if (!Values.empty()) {
		// Store each value into the allocated array data
		for (size_t i = 0; i < Values.size(); i++) {
			llvm::Value *Index = llvm::ConstantInt::get(CodeGen::IntPtrTy, i);
			llvm::Value *ElemPtr = CGM->Builder->CreateGEP(ElementType, ArrayPtr, Index);
			CGM->Builder->CreateStore(Values[i], ElemPtr);
		}

		// Cast the data pointer to i8* once (required by CodeGen::ArrayTy)
		llvm::Value *DataPtr = CGM->Builder->CreateBitCast(ArrayPtr, CodeGen::Int8PtrTy);

		// Allocate and populate dimensions array (1D array with single dimension)
		llvm::Value *DimSize = llvm::ConstantInt::get(CodeGen::SizeTy, Values.size());
		llvm::AllocaInst *DimsArray = CGM->Builder->CreateAlloca(CodeGen::SizeTy, llvm::ConstantInt::get(CodeGen::Int32Ty, 1));
		CGM->Builder->CreateStore(DimSize, DimsArray);

		// Create the array structure: { i8* data, size_t* dims, size_t rank }
		llvm::Value *ArrayStruct = llvm::UndefValue::get(CodeGen::ArrayTy);

		// Set field 0: data pointer (i8*)
		ArrayStruct = CGM->Builder->CreateInsertValue(ArrayStruct, DataPtr, 0);

		// Set field 1: dims pointer (size_t*)
		ArrayStruct = CGM->Builder->CreateInsertValue(ArrayStruct, DimsArray, 1);

		// Set field 2: rank (size_t) - 1 for 1D array
		llvm::Value *Rank = llvm::ConstantInt::get(CodeGen::SizeTy, 1);
		ArrayStruct = CGM->Builder->CreateInsertValue(ArrayStruct, Rank, 2);

		return Store(ArrayPtr);
	}

	// Store Null
	return CGM->Builder->CreateStore(llvm::Constant::getNullValue(T->getPointerTo()), getPointer());
}

llvm::Value *CodeGenVar::getDefaultValue(llvm::Type *T) {
	llvm::Value *DefaultValue = nullptr;
	switch (T->getTypeID()) {
		case llvm::Type::IntegerTyID:
			DefaultValue = llvm::ConstantInt::get(T, 0);
			break;
		case llvm::Type::FloatTyID:
		case llvm::Type::DoubleTyID:
			DefaultValue = llvm::ConstantFP::get(T, 0.0);
			break;
		case llvm::Type::PointerTyID:
			// Strings and pointers default to nullptr
			DefaultValue = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(T));
			break;
		case llvm::Type::StructTyID:
			// Other structs default to null value (all fields initialized to zero)
			DefaultValue = llvm::Constant::getNullValue(T->getPointerTo());
			break;
		default:
			CGM->Diag(diag::err_invalid_behavior);
			break;
	}
	return DefaultValue;
}

llvm::StoreInst *CodeGenVar::StoreDefaultValue() {
	SemaType *Ty = Sema->getType();

	// Check if this is a dynamic array (CodeGen::ArrayTy structure)
	if (Ty->isArray()) {
		SemaArrayType *ArrayTy = static_cast<SemaArrayType *>(Ty);
		llvm::Type *ElementType = ArrayTy->getElementType()->getCodeGen()->getType();

		// Dynamic array: zero-initialize with memset
		if (ElementType->isIntegerTy() || ElementType->isFloatingPointTy()) {
			llvm::Value *Size = nullptr;

			// Get the size from expression or constant
			if (ArrayTy->getSizeExpr()) {
				Size = ArrayTy->getSizeExpr()->getCodeGen()->getValue();
			} else if (ArrayTy->getSize() > 0) {
				Size = llvm::ConstantInt::get(CodeGen::Int64Ty, ArrayTy->getSize());
			}

			if (Size) {
				// Use i8 0 for memset value
				llvm::ConstantInt *ZeroInt8 = llvm::ConstantInt::get(CodeGen::Int8Ty, 0);

				// @malloc data type - create the array pointer (returns i8*)
				llvm::Instruction *I = llvm::CallInst::CreateMalloc(
					CGM->Builder->GetInsertBlock(),
					CodeGen::IntPtrTy,
					ElementType,
					Size,
					nullptr,
					nullptr
				);
				llvm::Value *MallocPtr = CGM->Builder->Insert(I);

				// Use malloc result directly for memset (already i8*)
				CGM->Builder->CreateMemSet(MallocPtr, ZeroInt8, Size, llvm::MaybeAlign());

				// Store the pointer
				setPointer(MallocPtr);
				return nullptr; // memset already writes to memory, no Store needed
			}
		}
	}

	// Non-array types
	llvm::Value *DefaultValue = getDefaultValue(Ty->getCodeGen()->getType());
	return Store(DefaultValue);
}

llvm::LoadInst *CodeGenVar::Load() {
    this->BlockID = CGM->Builder->GetInsertBlock()->getName();
    this->LoadI = CGM->Builder->CreateLoad(getPointer());
    return this->LoadI;
}

llvm::Value *CodeGenVar::getValue() {
    if (!this->LoadI || this->BlockID != CGM->Builder->GetInsertBlock()->getName()) {
        return Load();
    }
    return this->LoadI;
}

llvm::Value *CodeGenVar::getPointer() {
	if (Sema->getKind() == SemaKind::MEMBER) { // FIXME
		assert(this->Pointer && "Pointer must be set for MemberVar");

		// If Pointer is not set, create it
		llvm::PointerType *PtrType = llvm::cast<llvm::PointerType>(this->Pointer->getType());
		llvm::ArrayRef<llvm::Value *> IdxList = {CodeGen::Zero, llvm::ConstantInt::get(CodeGen::Int32Ty, Index)};
		this->Pointer = CGM->Builder->CreateInBoundsGEP(PtrType->getElementType(), this->Pointer, IdxList);
	} else if (Sema->getKind() == SemaKind::ATTRIBUTE) {
		assert(this->Pointer && "Pointer must be set for ClassAttribute");
		SemaClassAttribute * Attribute = static_cast<SemaClassAttribute *>(Sema);

		if (!Attribute->isStatic()) {
			CodeGenVar *CGV = static_cast<SemaClassInstance *>(Attribute->getParent())->getCodeGen();
			llvm::ArrayRef<llvm::Value *> IdxList = {CodeGen::Zero, llvm::ConstantInt::get(CodeGen::Int32Ty, Attribute->getCodeGen()->getIndex())};
			this->Pointer = CGM->Builder->CreateInBoundsGEP(CGV->getType(), CGV->getValue(), IdxList);
		}
	}
	// else if (Sema->getVarKind() == SemaVarKind::CLASS_INSTANCE) {
	// 	// Check ClassType for setting Index
	// 	SemaClassInstance * This = static_cast<SemaClassInstance *>(Sema);
	//
	// 	if (This->getParent()) {
	// 		CodeGenVarBase *CGV = This->getParent()->getCodeGen();
	// 		llvm::ArrayRef<llvm::Value *> IdxList = {CodeGen::Zero, llvm::ConstantInt::get(CodeGen::Int32Ty, This->getCodeGen()->getIndex())};
	// 		this->Pointer = CGM->Builder->CreateInBoundsGEP(CGV->getType(), CGV->getValue(), IdxList);
	// 	}
	// }

    return this->Pointer;
}

void CodeGenVar::setPointer(llvm::Value *Pointer) {
    this->Pointer = Pointer;
	this->LoadI = nullptr;
}

size_t CodeGenVar::getIndex() {
	return Index;
}
