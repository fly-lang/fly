//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGGlobalVar.cpp - Code Generator Block implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "CodeGen/CodeGenGlobalVar.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGen.h"
#include "AST/ASTValue.h"

using namespace fly;

CodeGenGlobalVar::CodeGenGlobalVar(CodeGenModule *CGM, const llvm::StringRef &Name, const ASTType *Ty,
                                   const ASTValue *Val, const bool isConstant) : CGM(CGM) {
    // Check Value
    llvm::Constant *Const = nullptr;
    llvm::Type *Typ;
    if (Val == nullptr || Val->empty()) {
        Typ = CGM->GenType(Ty);
    } else {
        Const = CGM->GenValue(Ty, Val);
        Typ = Const->getType();
    }
    GVar = new llvm::GlobalVariable(*CGM->Module, Typ, isConstant,GlobalValue::ExternalLinkage, Const, Name);
}

llvm::Value *CodeGenGlobalVar::getPointer() {
    return GVar;
}

llvm::Value *CodeGenGlobalVar::getValue() {
    return isStored ? (needLoad ? Load() : LoadI) : (llvm::Value *)GVar;
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
