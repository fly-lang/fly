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
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "AST/ASTClassVar.h"

using namespace fly;

CodeGenClassVar::CodeGenClassVar(CodeGenModule *CGM, ASTClassVar *Var, llvm::Type *ClassType, uint32_t Index) :
        CodeGenVar(CGM, Var), ClassType(ClassType), Index(llvm::ConstantInt::get(CGM->Int32Ty, Index)),
        Zero(llvm::ConstantInt::get(CGM->Int32Ty, 0)) {
}

void CodeGenClassVar::setClassInstance(llvm::Value *Instance) {
    this->ClassInstance = Instance;
}

void CodeGenClassVar::Init() {
    assert(ClassInstance && "Cannot init from unallocated stack");
    doLoad = true;
}

llvm::StoreInst *CodeGenClassVar::Store(llvm::Value *Val) {
    assert(ClassInstance && "Cannot store into unallocated stack");
    return CodeGenVar::Store(Val);
}

llvm::LoadInst *CodeGenClassVar::Load() {
    assert(ClassInstance && "Cannot load from unallocated stack");
    Pointer = CGM->Builder->CreateInBoundsGEP(ClassType, ClassInstance, { Zero, Index });
    return CodeGenVar::Load();
}

llvm::Value *CodeGenClassVar::getPointer() {
    if (!Pointer)
        CodeGenClassVar::Init();
    assert(Pointer && "Null pointer not managed");
    return Pointer;
}