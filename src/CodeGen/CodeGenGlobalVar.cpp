//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGGlobalVar.cpp - Code Generator Block implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "CodeGen/CodeGenGlobalVar.h"
#include "CodeGen/CodeGen.h"

using namespace fly;

CodeGenGlobalVar::CodeGenGlobalVar(CodeGenModule *CGM, const llvm::StringRef Name, const TypeBase *Ty,
                                   llvm::StringRef StrVal, const bool isConstant) : CGM(CGM) {
    // Check Value
    llvm::Constant *Val = nullptr;
    llvm::Type *Typ;
    if (StrVal.empty()) {
        Typ = CGM->GenType(Ty);
    } else {
        Val = CGM->GenValue(Ty, StrVal);
        Typ = Val->getType();
    }
    GVar = new llvm::GlobalVariable(*CGM->Module, Typ, isConstant,GlobalValue::ExternalLinkage, Val, Name);
}

GlobalVariable *CodeGenGlobalVar::getGlobalVar() const {
    return GVar;
}

llvm::User *CodeGenGlobalVar::get() {
    return isStored ? (needLoad ? Load() : LoadI) : static_cast<llvm::User *>(GVar);
}

llvm::StoreInst *CodeGenGlobalVar::Store(llvm::Value *Val) {
    assert(!GVar->isConstant() && "Cannot store into constant var");
    llvm::StoreInst *S = CGM->Builder->CreateStore(Val, GVar);
    isStored = true;
    needLoad = true;
    return S;
}

llvm::LoadInst *CodeGenGlobalVar::Load() {
    llvm::LoadInst *L = CGM->Builder->CreateLoad(GVar);
    needLoad = false;
    return L;
}
