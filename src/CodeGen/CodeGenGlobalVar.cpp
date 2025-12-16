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
#include "Sema/SemaModule.h"
#include "Sema/SemaNameSpace.h"
#include "AST/ASTVar.h"
#include "AST/ASTValue.h"
#include "AST/ASTModifier.h"

#include <AST/ASTExpr.h>
#include <Sema/SemaGlobalVar.h>

using namespace fly;

CodeGenGlobalVar::CodeGenGlobalVar(CodeGenModule *CGM, SemaGlobalVar* Sym, bool isExternal) : CGM(CGM), Sym(Sym) {
    std::string Id = CGM->toIdentifier(Sym->getAST().getName(), Sym->getModule()->getNameSpace());

	// External Linkage
	llvm::GlobalValue::LinkageTypes Linkage = llvm::GlobalValue::LinkageTypes::ExternalLinkage;

	// Generate Type
    llvm::Type *Ty = CGM->GenType(Sym->getType());

	// Generate Value
	llvm::Constant *Const = nullptr;
    if (!isExternal) {

    	// Internal or External Linkage
        if (Sym->getVisibility() == SemaVisibilityKind::PRIVATE) {
            Linkage = llvm::GlobalValue::LinkageTypes::InternalLinkage;
        }

    	// Set value
        if (Sym->getAST().getExpr() == nullptr) {

        	// Generate a Default Value
            Const = CGM->GenDefaultValue(Sym->getType(), Ty);
        } else {

        	// Generate the Value
            ASTValue *Value = ((ASTValueExpr *) Sym->getAST().getExpr())->getValue();

            if (Sym->getType()->isString()) {
                llvm::StringRef Str = ((ASTStringValue *) Value)->getValue();
                Const = llvm::ConstantDataArray::getString(CGM->LLVMCtx, Str);
                Ty = Const->getType();
            } else {
                Const = CGM->GenValue(Sym->getType(), Value);
            }
        }
    }

	// Set Global Var Constant
	bool IsConstant = Sym->isConstant();

	// Set Global Var
    Pointer = new llvm::GlobalVariable(*CGM->Module, Ty, IsConstant, Linkage, Const, Id);
}

llvm::Type *CodeGenGlobalVar::getType() {
    return T;
}

llvm::StoreInst *CodeGenGlobalVar::Store(llvm::Value *Val) {
    assert(!((llvm::GlobalVariable *) Pointer)->isConstant() && "Cannot store into constant var");
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
