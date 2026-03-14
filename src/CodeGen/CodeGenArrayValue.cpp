//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGExpr.cpp - Code Generator Expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenArrayValue.h"

#include "Basic/Debug.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "Sema/SemaEnumEntry.h"
#include "Sema/SemaEnumList.h"
#include "Sema/SemaEnumType.h"
#include "Sema/SemaType.h"
#include "Sema/SemaValue.h"

using namespace fly;

CodeGenArrayValue::CodeGenArrayValue(CodeGenModule *CGM) : CodeGenExpr(CGM) {
	FLY_DEBUG_START("CodeGenArrayValue", "CodeGenArrayValue");
}

std::vector<llvm::Value *> CodeGenArrayValue::getValues() const {
	return Values;
}

llvm::Type *CodeGenArrayValue::getElementType() const {
	return ElementType;
}

void CodeGenArrayValue::GenExpr(SemaArrayValue *Sema) {
	SemaArrayType *ArrayType = static_cast<SemaArrayType *>(Sema->getType());
	ArrayType->accept(*CGM);

	// Get the element type from the array type
	ArrayType->getElementType()->accept(*CGM);
	ElementType = ArrayType->getElementType()->getCodeGen()->getType();

	// Generate values and store them for later use
	Values.clear();
	for (SemaValue *Value : Sema->getValues()) {
		Value->accept(*CGM);
		llvm::Value *Val = Value->getCodeGen()->getValue();
		Values.push_back(Val);
	}

	// Calculate Space
	llvm::Value* AllocSize = llvm::ConstantInt::get(CodeGen::IntPtrTy, 0);
	if (Values.size() > 0) {
		llvm::Value* NumElements = llvm::ConstantInt::get(CodeGen::IntPtrTy, Values.size());
		llvm::TypeSize SizeInBytes = CGM->getTarget().getDataLayout().getTypeAllocSize(Values[0]->getType());
		llvm::Value* ElementSize = llvm::ConstantInt::get(CodeGen::IntPtrTy, SizeInBytes);
		AllocSize = Builder->CreateMul(NumElements, ElementSize);

		// @malloc data type - CreateMalloc returns ElementType* (already bitcasted)
		llvm::Instruction *I = llvm::CallInst::CreateMalloc(Builder->GetInsertBlock(), CodeGen::IntPtrTy,
													  ElementType, AllocSize, nullptr, nullptr);
		V = Builder->Insert(I);  // Already ElementType*, no need for additional bitcast
	} else {
		V = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(ElementType->getPointerTo()));
	}

	// Note: Element stores will be done in CodeGenVar::StoreArrayValue
}

void CodeGenArrayValue::GenExpr(SemaEnumList *Sema) {
	FLY_DEBUG_START("CodeGenArrayValue", "GenExpr(SemaEnumList)");

	// Enum entries are stored as i32 constants
	ElementType = CodeGen::Int32Ty;

	// Generate constant values for all enum entries (sorted by index)
	const auto &Entries = Sema->getEnumType()->getEntries();

	// Collect entries and sort by index to ensure deterministic order
	llvm::SmallVector<SemaEnumEntry *, 8> SortedEntries;
	for (auto &Entry : Entries) {
		SortedEntries.push_back(Entry.getValue());
	}
	std::sort(SortedEntries.begin(), SortedEntries.end(),
		[](SemaEnumEntry *A, SemaEnumEntry *B) { return A->getIndex() < B->getIndex(); });

	Values.clear();
	for (SemaEnumEntry *Entry : SortedEntries) {
		llvm::Value *Val = llvm::ConstantInt::get(CodeGen::Int32Ty, Entry->getIndex());
		Values.push_back(Val);
	}

	// Allocate memory for the array
	if (!Values.empty()) {
		llvm::Value* NumElements = llvm::ConstantInt::get(CodeGen::IntPtrTy, Values.size());
		llvm::TypeSize SizeInBytes = CGM->getTarget().getDataLayout().getTypeAllocSize(ElementType);
		llvm::Value* ElementSize = llvm::ConstantInt::get(CodeGen::IntPtrTy, SizeInBytes);
		llvm::Value* AllocSize = Builder->CreateMul(NumElements, ElementSize);

		llvm::Instruction *I = llvm::CallInst::CreateMalloc(Builder->GetInsertBlock(), CodeGen::IntPtrTy,
			ElementType, AllocSize, nullptr, nullptr);
		V = Builder->Insert(I);
	} else {
		V = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(ElementType->getPointerTo()));
	}

	FLY_DEBUG_END("CodeGenArrayValue", "GenExpr(SemaEnumList)");
	// Note: Element stores will be done in CodeGenVar::StoreArrayValue
}

