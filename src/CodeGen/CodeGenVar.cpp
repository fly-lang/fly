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
#include "CodeGen/CodeGenClass.h"
#include "CodeGen/CodeGenModule.h"
#include "AST/ASTVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTClass.h"
#include "AST/ASTType.h"
#include "llvm/IR/Value.h"

using namespace fly;

CodeGenVar::CodeGenVar(CodeGenModule *CGM, ASTVar *Var) : CGM(CGM), Var(Var) {
    // Fix Architecture Compatibility of bool i1 to i8
    this->T = Var->getType()->getKind() == ASTTypeKind::TYPE_BOOL ? CGM->Int8Ty : CGM->GenType(Var->getType());
}

void CodeGenVar::Init() {
    if (Var->getType()->isIdentity()) {
        llvm::PointerType *PtrT = T->getPointerTo(CGM->Module->getDataLayout().getAllocaAddrSpace());
        Pointer = CGM->Builder->CreateAlloca(PtrT);
    } else {
        Pointer = CGM->Builder->CreateAlloca(T);
    }
}

llvm::StoreInst *CodeGenVar::Store(llvm::Value *Val) {
    assert(Pointer && "Cannot store into unallocated stack");
    this->BlockID = CGM->Builder->GetInsertBlock()->getName();
    this->LoadI = nullptr;

    // Fix Architecture Compatibility of bool i1 to i8
    if (Var->getType()->getKind() == ASTTypeKind::TYPE_BOOL) {
        Val = CGM->Builder->CreateZExt(Val, CGM->Int8Ty);
    }

    return CGM->Builder->CreateStore(Val, this->Pointer);
}

llvm::LoadInst *CodeGenVar::Load() {
    assert(Pointer && "Cannot load from unallocated stack");
    this->BlockID = CGM->Builder->GetInsertBlock()->getName();
    this->LoadI = CGM->Builder->CreateLoad(Pointer);
    return this->LoadI;
}

llvm::Value *CodeGenVar::getValue() {
    if (!this->LoadI || this->BlockID != CGM->Builder->GetInsertBlock()->getName()) {
        return Load();
    }
    return this->LoadI;
}

llvm::Value *CodeGenVar::getPointer() {
    return this->Pointer;
}

ASTVar *CodeGenVar::getVar() {
    return Var;
}
