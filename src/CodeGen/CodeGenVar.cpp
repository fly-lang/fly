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

CodeGenVar::CodeGenVar(CodeGenModule *CGM, ASTVar *Var) : CodeGenVarBase(CGM, Var) {
    // Fix Architecture Compatibility of bool i1 to i8
    this->T = Var->getType()->getKind() == ASTTypeKind::TYPE_BOOL ? CGM->Int8Ty : CGM->GenType(Var->getType());
}

void CodeGenVar::Init() {
    Pointer = CGM->Builder->CreateAlloca(T);
    CodeGenVarBase::Init();
}

llvm::StoreInst *CodeGenVar::Store(llvm::Value *Val) {
    assert(Pointer && "Cannot store into unallocated stack");
    BlockID = CGM->Builder->GetInsertBlock()->getName();

    // Fix Architecture Compatibility of bool i1 to i8
    if (Var->getType()->getKind() == ASTTypeKind::TYPE_BOOL) {
        Val = CGM->Builder->CreateZExt(Val, CGM->Int8Ty);
    }

    return CodeGenVarBase::Store(Val);
}

llvm::LoadInst *CodeGenVar::Load() {
    assert(Pointer && "Cannot load from unallocated stack");
    BlockID = CGM->Builder->GetInsertBlock()->getName();
    return CodeGenVarBase::Load();
}

llvm::Value *CodeGenVar::getValue() {
    doLoad = doLoad || BlockID != CGM->Builder->GetInsertBlock()->getName();
    return CodeGenVarBase::getValue();
}
