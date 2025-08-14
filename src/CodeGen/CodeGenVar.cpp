//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGVar.cpp - Code Generator Block implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "CodeGen/CodeGenVar.h"
#include "CodeGen/CodeGenModule.h"

#include <Sema/SemaCall.h>
#include <Sema/SemaClassAttribute.h>
#include <Sema/SemaClassInstance.h>
#include <Sema/SemaMemberVar.h>
#include <Sema/SemaVar.h>
#include <llvm/IR/Instructions.h>

using namespace fly;

CodeGenVar::CodeGenVar(CodeGenModule *CGM, SemaVar *Sema, llvm::Type *T) :
	CGM(CGM), Sema(Sema), T(T) {

}

CodeGenVar::CodeGenVar(CodeGenModule *CGM, SemaVar *Sema, llvm::Type *T, size_t Index) :
	CGM(CGM), Sema(Sema), T(T), Index(Index) {

}

llvm::Type *CodeGenVar::getType() {
    return T;
}

llvm::StoreInst *CodeGenVar::Store(llvm::Value *Val) {
    this->BlockID = CGM->Builder->GetInsertBlock()->getName();
    this->LoadI = nullptr;

    // Fix Architecture Compatibility of bool i1 to i8
    if (T->isIntegerTy(1)) {
        Val = CGM->Builder->CreateZExt(Val, CGM->Int8Ty);
    }

	return CGM->Builder->CreateStore(Val, getPointer());
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
	if (Sema->getVarKind() == SemaVarKind::MEMBER_VAR) {
		assert(this->Pointer && "Pointer must be set for MemberVar");
		SemaMemberVar * MemberVar = static_cast<SemaMemberVar *>(Sema);

		// If Pointer is not set, create it
		CodeGenVarBase *CGV = MemberVar->getCodeGen();
		llvm::PointerType *PtrType = llvm::cast<llvm::PointerType>(this->Pointer->getType());
		llvm::ArrayRef<llvm::Value *> IdxList = {CGM->Zero, llvm::ConstantInt::get(CGM->Int32Ty, Index)};
		this->Pointer = CGM->Builder->CreateInBoundsGEP(PtrType->getElementType(), this->Pointer, IdxList);
	} else if (Sema->getVarKind() == SemaVarKind::CLASS_ATTRIBUTE) {
		assert(this->Pointer && "Pointer must be set for ClassAttribute");
		SemaClassAttribute * Attribute = static_cast<SemaClassAttribute *>(Sema);

		if (!Attribute->isStatic()) {
			CodeGenVarBase *CGV = static_cast<SemaClassInstance *>(Attribute->getParent())->getCodeGen();
			llvm::ArrayRef<llvm::Value *> IdxList = {CGM->Zero, llvm::ConstantInt::get(CGM->Int32Ty, Attribute->getCodeGen()->getIndex())};
			this->Pointer = CGM->Builder->CreateInBoundsGEP(CGV->getType(), CGV->getValue(), IdxList);
		}
	} else if (Sema->getVarKind() == SemaVarKind::CLASS_INSTANCE) {
		assert(this->Pointer && "Pointer must be set for ClassInstance");
		// TODO: Polymorphism
		// Check ClassType for setting Index
		SemaClassInstance * This = static_cast<SemaClassInstance *>(Sema);

		if (This->getParent()) {
			CodeGenVarBase *CGV = This->getParent()->getCodeGen();
			llvm::ArrayRef<llvm::Value *> IdxList = {CGM->Zero, llvm::ConstantInt::get(CGM->Int32Ty, This->getCodeGen()->getIndex())};
			this->Pointer = CGM->Builder->CreateInBoundsGEP(CGV->getType(), CGV->getValue(), IdxList);
		}
	}

    return this->Pointer;
}

void CodeGenVar::setPointer(llvm::Value *Pointer) {
    this->Pointer = Pointer;
	this->LoadI = nullptr;
}

size_t CodeGenVar::getIndex() {
	return Index;
}
