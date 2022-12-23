//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGFunction.cpp - Code Generator Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenFunctionBase.h"
#include "CodeGen/CodeGenVar.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTFunction.h"
#include "AST/ASTParams.h"
#include "AST/ASTExpr.h"
#include "AST/ASTCall.h"
#include "Basic/Debug.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/ADT/StringRef.h"
#include "AST/ASTClassFunction.h"

using namespace fly;

CodeGenFunctionBase::CodeGenFunctionBase(CodeGenModule *CGM, ASTFunctionBase *AST) : CGM(CGM), AST(AST) {
    FnTy = GenFuncType(AST->getType(), AST->getParams());
}

llvm::Function *CodeGenFunctionBase::Create() {
    Fn = llvm::Function::Create(FnTy, llvm::GlobalValue::ExternalLinkage, "", CGM->getModule());
    return Fn;
}

llvm::FunctionType *CodeGenFunctionBase::GenFuncType(const ASTType *RetTyData, const ASTParams *Params) {
    if (Params->getList().empty()) {
        // Create empty Function Type
        return llvm::FunctionType::get(CGM->GenType(RetTyData), Params->getEllipsis() != nullptr);
    } else {
        // Create Function Type with parameters
        SmallVector<llvm::Type *, 8> ArrayParams;
        // Add the PreParams on first
        for (auto &PreTy : PreParams) {
            ArrayParams.push_back(PreTy);
        }
        for (auto Param : Params->getList()) {
            llvm::Type *ParamTy = CGM->GenType(Param->getType());
            ArrayParams.push_back(ParamTy);
        }
        return llvm::FunctionType::get(CGM->GenType(RetTyData), ArrayParams, Params->getEllipsis() != nullptr);
    }
}

ASTFunctionBase *CodeGenFunctionBase::getAST() {
    return AST;
}

llvm::StringRef CodeGenFunctionBase::getName() const {
    return Fn->getName();
}

llvm::Function *CodeGenFunctionBase::getFunction() {
    return Fn;
}

llvm::FunctionType *CodeGenFunctionBase::getFunctionType() {
    return FnTy;
}


void CodeGenFunctionBase::GenBody() {
    FLY_DEBUG("CodeGenFunction", "GenBody");
    Entry = BasicBlock::Create(CGM->LLVMCtx, "entry", Fn);
    CGM->Builder->SetInsertPoint(Entry);

    // CodeGen of Params are contained into LocalVars
    // Allocation of declared local vars
    for (auto &LocalVar: AST->getLocalVars()) {
        LocalVar->setCodeGen(new CodeGenVar(CGM, LocalVar));
        LocalVar->getCodeGen()->Init();
    }

    // Store Param Values
    int n = 0;
    for (auto &P: AST->getParams()->getList()) {
        CodeGenVar *CGV = P->getCodeGen();
        CGV->Store(Fn->getArg(n));
        if (P->getExpr()) {
            CGM->GenExpr(Fn, P->getType(), P->getExpr());
        }
        ++n;
    }
    CGM->GenBlock(Fn, AST->getBody()->getContent());
}
