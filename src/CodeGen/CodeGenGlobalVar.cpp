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

CodeGenGlobalVar::CodeGenGlobalVar(CodeGenModule *CGM, const TypeBase *Ty, llvm::StringRef StrVal, const bool isConstant) {
    // Check Value
    llvm::Constant *Val = nullptr;
    llvm::Type *Typ;
    if (StrVal.empty()) {
        Typ = CGM->GenType(Ty);
    } else {
        Val = CGM->GenValue(Ty, StrVal);
        Typ = Val->getType();
    }
    GVar = new llvm::GlobalVariable(*CGM->Module, Typ, isConstant,GlobalValue::ExternalLinkage, Val);
}

GlobalVariable *CodeGenGlobalVar::getGlobalVar() const {
    return GVar;
}
