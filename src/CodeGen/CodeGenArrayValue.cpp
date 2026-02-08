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
