//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGVar.cpp - Code Generator Block implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "CodeGen/CodeGenLocalVar.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTType.h"
#include "llvm/IR/Value.h"

using namespace fly;

CodeGenLocalVar::CodeGenLocalVar(CodeGenModule *CGM, ASTLocalVar *Var) : CGM(CGM), Var(Var) {

}

llvm::Value *CodeGenLocalVar::getPointer() {
    return AllocaI;
}

llvm::Value *CodeGenLocalVar::getValue() {
    return isStored ? (needReload() ? Load() : LoadI) : (llvm::UnaryInstruction *) AllocaI;
}

llvm::AllocaInst *CodeGenLocalVar::Alloca() {
    const ASTType *T = Var->getType();
    // Fix Architecture Compatibility of bool i1 to i8
    Type *Ty = T->getKind() == TypeKind::TYPE_BOOL ? CGM->Int8Ty : CGM->GenType(T);
    AllocaI = CGM->Builder->CreateAlloca(Ty);
    AllocaI->getAllocatedType();
    return AllocaI;
}

llvm::StoreInst *CodeGenLocalVar::Store(llvm::Value *Val) {
    assert(!Var->isConstant() && "Cannot store into constant var");
    assert(AllocaI && "Cannot store into unallocated stack");
    if (Var->getType()->getKind() == TypeKind::TYPE_BOOL) { // Fix Architecture Compatibility of bool i1 to i8
        Val = CGM->Builder->CreateZExt(Val, CGM->Int8Ty);
    }
    llvm::StoreInst *S = CGM->Builder->CreateStore(Val, AllocaI);
    isStored = true;
    Reload = true;
    BlockID = CGM->Builder->GetInsertBlock()->getName();
    return S;
}

llvm::LoadInst *CodeGenLocalVar::Load() {
    assert(AllocaI && "Cannot load from unallocated stack");
    LoadI = CGM->Builder->CreateLoad(AllocaI);
    Reload = false;
    BlockID = CGM->Builder->GetInsertBlock()->getName();
    return LoadI;
}

bool CodeGenLocalVar::needReload() {
    return Reload || BlockID != CGM->Builder->GetInsertBlock()->getName();
}