//===--------------------------------------------------------------------------------------------------------------===//
// compiler/CodeGen/CodeGenArrayValue.cpp - array value code generation
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
	FLY_DEBUG_SCOPE("CodeGenArrayValue", "CodeGenArrayValue");
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
	bool ElemIsArray = ArrayType->getElementType()->isArray();

	// Generate values and store them for later use (elements are expressions)
	Values.clear();
	for (SemaExpr *Value : Sema->getValues()) {
		Value->accept(*CGM);
		llvm::Value *Val = Value->getCodeGen()->getValue();
		// An INNER array literal materializes as a full %array fat-pointer
		// VALUE {data, size}: its raw buffer pointer alone loses the size (and
		// mis-strides the outer buffer) — iterating a nested literal crashed.
		if (ElemIsArray) {
			CodeGenArrayValue *InnerCG =
				static_cast<CodeGenArrayValue *>(Value->getCodeGen());
			llvm::Value *Cnt = llvm::ConstantInt::get(
				CodeGen::IntTy, InnerCG->getValues().size());
			llvm::Value *StructV = llvm::UndefValue::get(CodeGen::ArrayTy);
			StructV = Builder->CreateInsertValue(StructV, Val, 0);
			StructV = Builder->CreateInsertValue(StructV, Cnt, 1);
			Val = StructV;
		}
		Values.push_back(Val);
	}

	// Calculate Space — from the SEMANTIC element type: for a nested array the
	// stored element is the %array struct (16 bytes), while Values[0] used to
	// report the raw pointer's 8 and undersize the buffer.
	if (Values.size() > 0) {
		llvm::Value* NumElements = llvm::ConstantInt::get(CodeGen::IntPtrTy, Values.size());
		llvm::TypeSize SizeInBytes = CGM->getTarget().getDataLayout().getTypeAllocSize(ElementType);
		llvm::Value* ElementSize = llvm::ConstantInt::get(CodeGen::IntPtrTy, SizeInBytes.getFixedValue());
		llvm::Value* AllocSize = Builder->CreateMul(NumElements, ElementSize);

		// Call malloc to allocate memory for the array data
		llvm::FunctionCallee MallocFn = CGM->getModule()->getOrInsertFunction(
			"malloc",
			llvm::FunctionType::get(
				llvm::PointerType::getUnqual(CGM->getLLVMCtx()),
				{CodeGen::IntPtrTy},
				false));
		V = Builder->CreateCall(MallocFn, {AllocSize});

		// Fill the buffer HERE, self-contained: an inner literal must be fully
		// materialized before the outer literal captures it — the old deferral
		// to CodeGenVar::StoreArrayValue only ever ran for the TOP-level store,
		// so inner buffers stayed unfilled.
		for (size_t i = 0; i < Values.size(); i++) {
			llvm::Value *Index = llvm::ConstantInt::get(CodeGen::IntPtrTy, i);
			llvm::Value *ElemPtr = Builder->CreateGEP(ElementType, V, Index);
			Builder->CreateStore(Values[i], ElemPtr);
		}
	} else {
		V = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(ElementType->getPointerTo()));
	}
}

void CodeGenArrayValue::GenExpr(SemaEnumList *Sema) {
	FLY_DEBUG_SCOPE("CodeGenArrayValue", "GenExpr(SemaEnumList)");

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
		llvm::Value* ElementSize = llvm::ConstantInt::get(CodeGen::IntPtrTy, SizeInBytes.getFixedValue());
		llvm::Value* AllocSize = Builder->CreateMul(NumElements, ElementSize);

		// Call malloc to allocate memory for the array data
		llvm::FunctionCallee MallocFn = CGM->getModule()->getOrInsertFunction(
			"malloc",
			llvm::FunctionType::get(
				llvm::PointerType::getUnqual(CGM->getLLVMCtx()),
				{CodeGen::IntPtrTy},
				false));
		V = Builder->CreateCall(MallocFn, {AllocSize});
	} else {
		V = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(ElementType->getPointerTo()));
	}
	// Note: Element stores will be done in CodeGenVar::StoreArrayValue
}

