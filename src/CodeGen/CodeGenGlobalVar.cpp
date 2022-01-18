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
#include "AST/ASTNameSpace.h"
#include "AST/ASTValue.h"
#include "AST/ASTGlobalVar.h"

using namespace fly;

CodeGenGlobalVar::CodeGenGlobalVar(CodeGenModule *CGM, ASTGlobalVar* AST, bool isExternal) : CGM(CGM) {
    // Check Value
    bool Success = true;
    llvm::Constant *Const = nullptr;
    GlobalValue::LinkageTypes Linkage = GlobalValue::LinkageTypes::ExternalLinkage;
    if (!isExternal) {
        if (AST->getVisibility() == V_PRIVATE) {
            Linkage = GlobalValue::LinkageTypes::InternalLinkage;
        }
        if (AST->getExpr() == nullptr) {
            Const = CGM->GenDefaultValue(AST->getType());
        } else if (AST->getExpr()->getKind() == EXPR_VALUE) {
            const ASTValue &Value = ((ASTValueExpr *) AST->getExpr())->getValue();
            Const = CGM->GenValue(AST->getType(), &Value);
        } else {
            CGM->Diag(AST->getExpr()->getLocation(), diag::err_invalid_gvar_value);
            Success = false;
        }
    }

    if (Success) {
        llvm::Type *Typ = CGM->GenType(AST->getType());
        std::string Id = CodeGen::toIdentifier(AST->getName(), AST->getNameSpace()->getName());
        GVar = new llvm::GlobalVariable(*CGM->Module, Typ, AST->isConstant(), Linkage, Const, Id);
    }
    needLoad = true;
}

llvm::Value *CodeGenGlobalVar::getPointer() {
    return GVar;
}

llvm::Value *CodeGenGlobalVar::getValue() {
    return needLoad ? Load() : LoadI;
}

llvm::StoreInst *CodeGenGlobalVar::Store(llvm::Value *Val) {
    assert(!GVar->isConstant() && "Cannot store into constant var");
    llvm::StoreInst *S = CGM->Builder->CreateStore(Val, GVar);
    needLoad = true;
    return S;
}

llvm::LoadInst *CodeGenGlobalVar::Load() {
    LoadI = CGM->Builder->CreateLoad(GVar);
    needLoad = false;
    return LoadI;
}

/**
 * Need to be called on function body complete
 */
void CodeGenGlobalVar::reset() {
    needLoad = true;
}
