//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGVar.cpp - Code Generator Block implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "CodeGen/CGVar.h"
#include "CodeGen/CodeGen.h"
#include "llvm/IR/Value.h"

using namespace fly;

CGVar::CGVar(CodeGenModule *CGM, VarDeclStmt *S) : CGM(CGM), Var(*S) {
    const TypeBase *Tyb = S->getType();
    Type *Ty = CGM->GenTypeValue(Tyb);
    AllocaI = CGM->Builder->CreateAlloca(Ty);
}

llvm::UnaryInstruction *CGVar::get() {
    return isStored ? (needLoad ? Load() : LoadI) : static_cast<llvm::UnaryInstruction *>(AllocaI);
}

llvm::StoreInst *CGVar::Store(llvm::Value *Val) {
    assert(!Var.isConstant() && "Cannot store into constant var");
    assert(AllocaI && "Connot store into unallocated stack");
    llvm::StoreInst *S = CGM->Builder->CreateStore(Val, AllocaI);
    isStored = true;
    needLoad = true;
    return S;
}

llvm::LoadInst *CGVar::Load() {
    assert(AllocaI && "Connot load from unallocated stack");
    llvm::LoadInst *L = CGM->Builder->CreateLoad(AllocaI);
    needLoad = false;
    return L;
}