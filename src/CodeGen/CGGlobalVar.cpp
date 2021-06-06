//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGGlobalVar.cpp - Code Generator Block implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "CodeGen/CGGlobalVar.h"
#include "CodeGen/CodeGen.h"

using namespace fly;

CGGlobalVar::CGGlobalVar(CodeGenModule *CGM, const TypeBase *Ty, llvm::StringRef StrVal, const bool isConstant) {
    // Check Value
    llvm::Constant *InitVal = nullptr;
    llvm::Type *Typ = nullptr;
    if (StrVal.empty()) {
        Typ = CGM->GenTypeValue(Ty);
    } else {
        Typ = CGM->GenTypeValue(Ty, StrVal, InitVal);
    }
    GVar = new llvm::GlobalVariable(*CGM->Module, Typ, isConstant,
                                    GlobalValue::ExternalLinkage, InitVal);
}

GlobalVariable *CGGlobalVar::getGlobalVar() const {
    return GVar;
}
