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
#include "AST/ASTModule.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTValue.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTScopes.h"
#include "AST/ASTVarStmt.h"

using namespace fly;

CodeGenGlobalVar::CodeGenGlobalVar(CodeGenModule *CGM, ASTGlobalVar* Var, bool isExternal) : CGM(CGM), Var(Var) {
    std::string Id = CodeGen::toIdentifier(Var->getName(), Var->getNameSpace()->getName());

    // Check Value
    llvm::Constant *Const = nullptr;
    bool IsConstant = Var->isConstant();
    GlobalValue::LinkageTypes Linkage = GlobalValue::LinkageTypes::ExternalLinkage;
    llvm::Type *Ty = CGM->GenType(Var->getType());
    if (!isExternal) {
        if (Var->getVisibility() == ASTVisibilityKind::V_PRIVATE) {
            Linkage = GlobalValue::LinkageTypes::InternalLinkage;
        }
        if (Var->getExpr() == nullptr) {
            Const = CGM->GenDefaultValue(Var->getType(), Ty);
        } else {
            ASTValue *Value = ((ASTValueExpr *) Var->getExpr())->getValue();
            if (Var->getType()->isString()) {
                llvm::StringRef Str = ((ASTStringValue *) Value)->getValue();
                Const = llvm::ConstantDataArray::getString(CGM->LLVMCtx, Str);
                Ty = Const->getType();
            } else {
                Const = CGM->GenValue(Var->getType(), Value);
            }
        }
    }

    Pointer = new llvm::GlobalVariable(*CGM->Module, Ty, IsConstant, Linkage, Const, Id);
}

llvm::Type *CodeGenGlobalVar::getType() {
    return T;
}

llvm::StoreInst *CodeGenGlobalVar::Store(llvm::Value *Val) {
    assert(!((GlobalVariable *) Pointer)->isConstant() && "Cannot store into constant var");
    this->LoadI = nullptr;
    return CGM->Builder->CreateStore(Val, Pointer);
}

llvm::LoadInst *CodeGenGlobalVar::Load() {
    this->LoadI = CGM->Builder->CreateLoad(Pointer);
    return this->LoadI;
}

llvm::Value *CodeGenGlobalVar::getValue() {
    return this->LoadI ? this->LoadI : Load();
}

llvm::Value *CodeGenGlobalVar::getPointer() {
    return Pointer;
}

CodeGenVarBase *CodeGenGlobalVar::getVar(llvm::StringRef Name) {
    return nullptr;
}
