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

using namespace fly;

CodeGenClassVar::CodeGenClassVar(CodeGenModule *CGM, ASTClassVar *Var, llvm::Type *ClassType, uint32_t Index) :
        CodeGenVar(CGM, Var), ClassType(ClassType), Index(llvm::ConstantInt::get(CGM->Int32Ty, Index)),
        Zero(llvm::ConstantInt::get(CGM->Int32Ty, 0)) {
}

/**
 * Set when var is referenced
 * @param Instance
 */
void CodeGenClassVar::Init(llvm::Value *Instance) {
    assert(Instance && "Cannot init from unallocated stack");
    this->ClassInstance = Instance;
    doLoad = true;
}

llvm::StoreInst *CodeGenClassVar::Store(llvm::Value *Val) {
    assert(ClassInstance && "Cannot store into unallocated stack");
    assert(ClassType && "Class Type not defined");
    BlockID = CGM->Builder->GetInsertBlock()->getName();
    return CodeGenVarBase::Store(Val);
}

llvm::LoadInst *CodeGenClassVar::Load() {
    assert(ClassInstance && "Cannot load from unallocated stack");
    assert(ClassType && "Class Type not defined");
    BlockID = CGM->Builder->GetInsertBlock()->getName();
    if (doLoad)
        Pointer = CGM->Builder->CreateInBoundsGEP(ClassType, ClassInstance, {Zero, Index});
    return CodeGenVarBase::Load();
}

llvm::Value *CodeGenClassVar::getValue() {
    doLoad = doLoad || BlockID != CGM->Builder->GetInsertBlock()->getName();
    return CodeGenVarBase::getValue();
}

llvm::Value *CodeGenClassVar::getPointer() {
    if (!Pointer)
        Pointer = CGM->Builder->CreateInBoundsGEP(ClassType, ClassInstance, {Zero, Index});
    return Pointer;
}