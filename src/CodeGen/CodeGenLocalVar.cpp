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
#include "AST/ASTFunc.h"
#include "llvm/IR/Value.h"

using namespace fly;

CodeGenLocalVar::CodeGenLocalVar(CodeGenModule *CGM, ASTLocalVar *S) : CGM(CGM), Constant(S->isConstant()) {
    const ASTType *Tyb = S->getType();
    Type *Ty = CGM->GenType(Tyb);
    AllocaI = CGM->Builder->CreateAlloca(Ty);
    AllocaI->getAllocatedType();
}

CodeGenLocalVar::CodeGenLocalVar(CodeGenModule *CGM, ASTFuncParam *P) : CGM(CGM), Constant(P->isConstant()) {
    const ASTType *Tyb = P->getType();
    Type *Ty = CGM->GenType(Tyb);
    AllocaI = CGM->Builder->CreateAlloca(Ty);
    AllocaI->getAllocatedType();
}

llvm::UnaryInstruction *CodeGenLocalVar::get() {
    return isStored ? (needLoad ? Load() : LoadI) : (llvm::UnaryInstruction *) AllocaI;
}

llvm::StoreInst *CodeGenLocalVar::Store(llvm::Value *Val) {
    assert(!Constant && "Cannot store into constant var");
    assert(AllocaI && "Cannot store into unallocated stack");
    llvm::StoreInst *S = CGM->Builder->CreateStore(Val, AllocaI);
    isStored = true;
    needLoad = true;
    return S;
}

llvm::LoadInst *CodeGenLocalVar::Load() {
    assert(AllocaI && "Connot load from unallocated stack");
    llvm::LoadInst *L = CGM->Builder->CreateLoad(AllocaI);
    needLoad = false;
    return L;
}