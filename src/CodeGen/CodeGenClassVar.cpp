//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenClassVar.cpp - Code Generator Class Var
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenClassVar.h"
#include "CodeGen/CodeGenVar.h"
#include "CodeGen/CodeGenModule.h"
#include "Sema/SemaClassAttribute.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>

using namespace fly;

CodeGenClassVar::CodeGenClassVar(CodeGenModule *CGM, SemaClassAttribute *Sema, uint64_t Index) :
        CGM(CGM), Type(CGM->GenType(Sema->getType())), Index(llvm::ConstantInt::get(CGM->Int32Ty, Index)),
        Zero(CGM->Zero) {
}

llvm::StoreInst *CodeGenClassVar::Store(llvm::Value *Val) {
    assert(Type && "Class Type not defined");

    // Fix Architecture Compatibility of bool i1 to i8
	// FIXME
    // if (Var->getType()->isBool()) {
    //     Val = CGM->Builder->CreateZExt(Val, CGM->Int8Ty);
    // }

    llvm::StoreInst *S = CGM->Builder->CreateStore(Val, getValue());
    this->BlockID = CGM->Builder->GetInsertBlock()->getName();
    this->LoadI = nullptr;
    return S;
}

llvm::LoadInst *CodeGenClassVar::Load() {
    assert(Type && "Class Type not defined");
    this->LoadI =  CGM->Builder->CreateLoad(getPointer());
    this->BlockID = CGM->Builder->GetInsertBlock()->getName();
    return this->LoadI;
}

llvm::Value *CodeGenClassVar::getValue() {
    if (!this->LoadI || this->BlockID != CGM->Builder->GetInsertBlock()->getName())
        return Load();
    return this->LoadI;
}

llvm::Value *CodeGenClassVar::getPointer() {
    if (!this->Pointer)
        this->Pointer = CGM->Builder->CreateInBoundsGEP(Type, nullptr, {Zero, Index});
    return this->Pointer;
}

llvm::Value *CodeGenClassVar::getIndex() {
    return Index;
}

llvm::Type * CodeGenClassVar::getType() {
	return Type;
}
