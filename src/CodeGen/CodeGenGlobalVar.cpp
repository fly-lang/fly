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

CodeGenGlobalVar::CodeGenGlobalVar(CodeGenModule *CGM, ASTGlobalVar* AST, bool isExternal) : CodeGenVarBase(CGM, AST) {
    // Check Value
    bool Success = true;
    llvm::Constant *Const = nullptr;
    GlobalValue::LinkageTypes Linkage = GlobalValue::LinkageTypes::ExternalLinkage;
    llvm::Type *Ty = CGM->GenType(AST->getType());
    if (!isExternal) {
        if (AST->getScopes()->getVisibility() == ASTVisibilityKind::V_PRIVATE) {
            Linkage = GlobalValue::LinkageTypes::InternalLinkage;
        }
        if (AST->getExpr() == nullptr) {
            Const = CGM->GenDefaultValue(AST->getType(), Ty);
        } else if (AST->getExpr()->getExprKind() == ASTExprKind::EXPR_VALUE) {
            const ASTValue &Value = ((ASTValueExpr *) AST->getExpr())->getValue();
            Const = CGM->GenValue(AST->getType(), &Value);
        } else {
            CGM->Diag(AST->getExpr()->getLocation(), diag::err_invalid_gvar_value);
            Success = false;
        }
    }

    if (Success) {
        std::string Id = CodeGen::toIdentifier(AST->getName(), AST->getNameSpace()->getName());
        Pointer = new llvm::GlobalVariable(*CGM->Module, Ty, AST->getScopes()->isConstant(), Linkage, Const, Id);
    }
}

/**
 * Need to be called on function body complete
 */
void CodeGenGlobalVar::Init() {
    doLoad = true;
}

llvm::StoreInst *CodeGenGlobalVar::Store(llvm::Value *Val) {
    assert(!((GlobalVariable *) Pointer)->isConstant() && "Cannot store into constant var");
    return CodeGenVarBase::Store(Val);
}

llvm::LoadInst *CodeGenGlobalVar::Load() {
    return CodeGenVarBase::Load();
}

llvm::Value *CodeGenGlobalVar::getValue() {
    return doLoad ? Load() : LoadI;
}

llvm::Value *CodeGenGlobalVar::getPointer() {
    return Pointer;
}