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
#include "AST/ASTLocalVar.h"
#include "AST/ASTFunc.h"
#include "llvm/IR/Value.h"

using namespace fly;

CodeGenVar::CodeGenVar(CodeGenModule *CGM, ASTLocalVar *S) : CGM(CGM), Constant(S->isConstant()) {
    const ASTType *Tyb = S->getType();
    Type *Ty = CGM->GenType(Tyb);
    AllocaI = CGM->Builder->CreateAlloca(Ty);
    AllocaI->getAllocatedType();
}

CodeGenVar::CodeGenVar(CodeGenModule *CGM, ASTFuncParam *P) : CGM(CGM), Constant(P->isConstant()) {
    const ASTType *Tyb = P->getType();
    Type *Ty = CGM->GenType(Tyb);
    AllocaI = CGM->Builder->CreateAlloca(Ty);
    AllocaI->getAllocatedType();
}

llvm::UnaryInstruction *CodeGenVar::get() {
    return isStored ? (needLoad ? Load() : LoadI) : static_cast<llvm::UnaryInstruction *>(AllocaI);
}

llvm::StoreInst *CodeGenVar::Store(llvm::Value *Val) {
    assert(!Constant && "Cannot store into constant var");
    assert(AllocaI && "Cannot store into unallocated stack");
    llvm::StoreInst *S = CGM->Builder->CreateStore(Val, AllocaI);
    isStored = true;
    needLoad = true;
    return S;
}

llvm::LoadInst *CodeGenVar::Load() {
    assert(AllocaI && "Connot load from unallocated stack");
    llvm::LoadInst *L = CGM->Builder->CreateLoad(AllocaI);
    needLoad = false;
    return L;
}