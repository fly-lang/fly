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

CodeGenVar::CodeGenVar(CodeGenModule *CGM, ASTVar *Var) : CGM(CGM), Var(Var),
        // Fix Architecture Compatibility of bool i1 to i8
        T(Var->getType()->getKind() == TypeKind::TYPE_BOOL ? CGM->Int8Ty : CGM->GenType(Var->getType())) {

}

void CodeGenVar::Init() {
    Pointer = CGM->Builder->CreateAlloca(T);

    if (Var->getType()->isClass()) {
        ((ASTClassType *) Var->getType())->getDef()->getCodeGen()->InvokeDefaultConstructor(Pointer);
    }
}

llvm::StoreInst *CodeGenVar::Store(llvm::Value *Val) {
    assert(getPointer() && "Cannot store into unallocated stack");

    // Fix Architecture Compatibility of bool i1 to i8
    if (Var->getType()->getKind() == TypeKind::TYPE_BOOL) {
        Val = CGM->Builder->CreateZExt(Val, CGM->Int8Ty);
    }

    llvm::StoreInst *S = CGM->Builder->CreateStore(Val, getPointer());
    isStored = true;
    needLoad = true;
    BlockID = CGM->Builder->GetInsertBlock()->getName();
    return S;
}

llvm::Value *CodeGenVar::Load() {
    assert(getPointer() && "Cannot load from unallocated stack");
    LoadI = CGM->Builder->CreateLoad(T, Pointer);
    needLoad = false;
    BlockID = CGM->Builder->GetInsertBlock()->getName();
    return LoadI;
}

bool CodeGenVar::needReload() {
    return needLoad || BlockID != CGM->Builder->GetInsertBlock()->getName();
}

llvm::Value *CodeGenVar::getValue() {
    assert(getPointer() && "Var not allocated yet");
    return isStored ? (needReload() ? Load() : LoadI) : Load(); //getPointer();
}

llvm::Value *CodeGenVar::getPointer() {
    return Pointer;
}
