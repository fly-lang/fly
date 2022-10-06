//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenClassVar.cpp - Code Generator Class Var
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenClass.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenVar.h"
#include "CodeGen/CodeGenModule.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassVar.h"
#include "AST/ASTClassFunction.h"
#include "AST/ASTNameSpace.h"

#include "llvm/IR/DerivedTypes.h"

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
    llvm::ArrayRef<Value *> IdxList = {Zero, Index};
    Pointer = CGM->Builder->CreateInBoundsGEP(ClassType, ClassInstance, IdxList);
}

llvm::Value *CodeGenClassVar::Load() {
    assert(ClassInstance && "Cannot load from unallocated stack");
    LoadI = CGM->Builder->CreateLoad(T, Pointer);
    needLoad = false;
    BlockID = CGM->Builder->GetInsertBlock()->getName();
    return LoadI;
}

llvm::Value *CodeGenClassVar::getPointer() {
    if (!Pointer)
        Init();
    assert(Pointer && "Null pointer not managed");
    return Pointer;
}