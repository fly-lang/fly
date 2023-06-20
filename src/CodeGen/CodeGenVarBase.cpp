//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenVarBase.cpp - Code Generator Base Var
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "CodeGen/CodeGenVarBase.h"
#include "CodeGen/CodeGenModule.h"
#include "AST/ASTVar.h"
#include "AST/ASTClass.h"
#include "AST/ASTType.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Instructions.h"

using namespace fly;

CodeGenVarBase::CodeGenVarBase(CodeGenModule *CGM, ASTVar *Var) : CGM(CGM), Var(Var), T(CGM->GenType(Var->getType())) {

}

ASTVar *CodeGenVarBase::getVar() {
    return Var;
}

void CodeGenVarBase::Init() {
    doLoad = true;
}

llvm::StoreInst *CodeGenVarBase::Store(llvm::Value *Val) {
    assert(Val && "Cannot store null value");

    llvm::StoreInst *S = CGM->Builder->CreateStore(Val, getPointer());
    doLoad = true;

    return S;
}

llvm::LoadInst *CodeGenVarBase::Load() {
    LoadI = CGM->Builder->CreateLoad(T, Pointer);
    doLoad = false;

    return LoadI;
}

llvm::Value *CodeGenVarBase::getValue() {
    return doLoad ? Load() : LoadI;
}

llvm::Value *CodeGenVarBase::getPointer() {
    return Pointer;
}
