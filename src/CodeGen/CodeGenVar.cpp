//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGVar.cpp - Code Generator Block implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "CodeGen/CodeGenVar.h"
#include "CodeGen/CodeGenClass.h"
#include "CodeGen/CodeGenModule.h"

#include <Sema/SemaCall.h>
#include <Sema/SemaClassAttribute.h>
#include <Sema/SemaClassInstance.h>
#include <Sema/SemaClassType.h>
#include <Sema/SemaMemberVar.h>
#include <Sema/SemaVar.h>
#include <llvm/IR/Instructions.h>

using namespace fly;

CodeGenVar::CodeGenVar(CodeGenModule *CGM, SemaVar *Sema, llvm::Type *T, llvm::Value *Pointer) : CGM(CGM), Sema(Sema), T(T), Pointer(Pointer) {

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
		SemaMemberVar * MemberVar = static_cast<SemaMemberVar *>(Sema);

		// Check ClassType for setting Index
		SemaClassType *ClassType = static_cast<SemaClassType *>(MemberVar->getParent()->getType());
		llvm::ConstantInt *Index = getIndex(ClassType, MemberVar->getIndex());

		// If Pointer is not set, create it
		llvm::PointerType *PtrType = llvm::cast<llvm::PointerType>(this->Pointer->getType());
		this->Pointer = CGM->Builder->CreateInBoundsGEP(PtrType->getElementType(), this->Pointer, {CGM->Zero, Index});
	} else if (Sema->getVarKind() == SemaVarKind::CLASS_ATTRIBUTE) {
		SemaClassAttribute * Attribute = static_cast<SemaClassAttribute *>(Sema);

		if (!Attribute->isStatic()) {
			// Check ClassType for setting Index
			SemaClassType *ClassType = static_cast<SemaClassType *>(Attribute->getClass());
			llvm::ConstantInt *Index = getIndex(ClassType, Attribute->getIndex());

			CodeGenVarBase *CGV = static_cast<SemaClassInstance *>(Attribute->getParent())->getCodeGen();
			this->Pointer = CGM->Builder->CreateInBoundsGEP(CGV->getType(), CGV->getValue(), {CGM->Zero, Index});
		}
	}

    return this->Pointer;
}

llvm::ConstantInt * CodeGenVar::getIndex(fly::SemaClassType *ClassType, uint64_t Index) {
	return llvm::ConstantInt::get(CGM->Int32Ty, ClassType->getClassKind() == SemaClassKind::STRUCT ? Index : Index + 1);
}
