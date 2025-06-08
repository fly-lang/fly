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
#include <Sema/SemaMemberVar.h>
#include <Sema/SemaVar.h>
#include <llvm/IR/Instructions.h>

using namespace fly;

//CodeGenVar::CodeGenVar(CodeGenModule *CGM, ASTVar *Var) : CGM(CGM) {
//    this->T = Var->getType()->getKind() == ASTTypeKind::TYPE_BOOL ? CGM->Int8Ty : CGM->GenType(Var->getType());
//}

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
	if (Sema->getVarKind() == SemaVarKind::VAR_MEMBER) {
		SemaMemberVar * MemberVar = static_cast<SemaMemberVar *>(Sema);
		llvm::ConstantInt *Index = llvm::ConstantInt::get(CGM->Int32Ty, MemberVar->getIndex());
		llvm::PointerType *PtrType =llvm::cast<llvm::PointerType>(this->Pointer->getType());
		this->Pointer = CGM->Builder->CreateInBoundsGEP(PtrType->getElementType(), this->Pointer, {CGM->Zero, Index});
	}

    return this->Pointer;
}

