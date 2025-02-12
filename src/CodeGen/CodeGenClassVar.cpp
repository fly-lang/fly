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
#include "Sym/SymClassAttribute.h"

using namespace fly;

CodeGenClassVar::CodeGenClassVar(CodeGenModule *CGM, llvm::Type *T, CodeGenVarBase *Parent, size_t Index) :
        CGM(CGM), Parent(Parent), Type(T), Index(llvm::ConstantInt::get(CGM->Int32Ty, Index)),
        Zero(llvm::ConstantInt::get(CGM->Int32Ty, 0)) {
}

void CodeGenClassVar::setInstance(llvm::Value *Inst) {
    this->Instance = Inst;
}

llvm::StoreInst *CodeGenClassVar::Store(llvm::Value *Val) {
    assert(Type && "Class Type not defined");

    // Fix Architecture Compatibility of bool i1 to i8
	// FIXME
    // if (Var->getType()->isBool()) {
    //     Val = CGM->Builder->CreateZExt(Val, CGM->Int8Ty);
    // }

    StoreInst *S = CGM->Builder->CreateStore(Val, getValue());
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
        this->Pointer = CGM->Builder->CreateInBoundsGEP(Type, Instance, {Zero, Index});
    return this->Pointer;
}

llvm::Value *CodeGenClassVar::getIndex() {
    return Index;
}

llvm::Type * CodeGenClassVar::getType() {
	return Type;
}

CodeGenVarBase * CodeGenClassVar::getVar(llvm::StringRef Name) {
	return Parent;
}
