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
#include "CodeGen/CodeGenClass.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "AST/ASTClassVar.h"
#include "AST/ASTClass.h"
#include "AST/ASTType.h"

using namespace fly;

CodeGenClassVar::CodeGenClassVar(CodeGenModule *CGM, ASTClassVar *Var, llvm::Type *ClassType, uint32_t Index) :
        CGM(CGM), Var(Var), ClassType(ClassType), Index(llvm::ConstantInt::get(CGM->Int32Ty, Index)),
        Zero(llvm::ConstantInt::get(CGM->Int32Ty, 0)) {
}

/**
 * Set when var is referenced
 * @param Instance
 */
void CodeGenClassVar::Init() {
    this->Pointer = nullptr;
}

llvm::StoreInst *CodeGenClassVar::Store(llvm::Value *Val) {
    assert(ClassType && "Class Type not defined");

    // Fix Architecture Compatibility of bool i1 to i8
    if (Var->getType()->getKind() == ASTTypeKind::TYPE_BOOL) {
        Val = CGM->Builder->CreateZExt(Val, CGM->Int8Ty);
    }

    StoreInst *S = CGM->Builder->CreateStore(Val, getValue());
    this->BlockID = CGM->Builder->GetInsertBlock()->getName();
    this->LoadI = nullptr;
    return S;
}

llvm::LoadInst *CodeGenClassVar::Load() {
    assert(ClassType && "Class Type not defined");
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
        this->Pointer = CGM->Builder->CreateInBoundsGEP(ClassType, Instance, {Zero, Index});
    return this->Pointer;
}

ASTVar *CodeGenClassVar::getVar() {
    return Var;
}

llvm::Value *CodeGenClassVar::getIndex() {
    return Index;
}

void CodeGenClassVar::setInstance(llvm::Value *Inst) {
    this->Instance = Inst;
}
