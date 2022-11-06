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

CodeGenVarBase::CodeGenVarBase(CodeGenModule *CGM, ASTVar *Var) : CGM(CGM), Var(Var),
        // Fix Architecture Compatibility of bool i1 to i8
        T(Var->getType()->getKind() == ASTTypeKind::TYPE_BOOL ? CGM->Int8Ty : CGM->GenType(Var->getType())) {

}

llvm::StoreInst *CodeGenVarBase::Store(llvm::Value *Val) {

    // Fix Architecture Compatibility of bool i1 to i8
    if (Var->getType()->getKind() == ASTTypeKind::TYPE_BOOL) {
        Val = CGM->Builder->CreateZExt(Val, CGM->Int8Ty);
    }

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
