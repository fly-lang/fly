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

}

llvm::Function *CodeGenFunctionBase::Create() {
    FnTy = GenFuncType(AST->getType(), AST->getParams());
    Fn = llvm::Function::Create(FnTy, llvm::GlobalValue::ExternalLinkage, "", CGM->getModule());
    return Fn;
}

llvm::FunctionType *CodeGenFunctionBase::GenFuncType(const ASTType *RetType, const ASTParams *Params) {
    return GenFuncType(CGM->GenType(RetType), Params);
}

llvm::FunctionType *CodeGenFunctionBase::GenFuncType(llvm::Type *RetType, const ASTParams *Params) {
    // Create Function Type with parameters
    SmallVector<llvm::Type *, 8> ArrayParams;

    // Add the PreParams on first
    for (auto &PreTy : PreParams) {
        ArrayParams.push_back(PreTy);
    }

    if (!Params->getList().empty()) {
        for (auto Param : Params->getList()) {
            llvm::Type *ParamTy = CGM->GenType(Param->getType());
            ArrayParams.push_back(ParamTy);
        }
    }

    return llvm::FunctionType::get(RetType, ArrayParams, Params->getEllipsis() != nullptr);
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

void CodeGenFunctionBase::setInsertPoint() {
    FLY_DEBUG("CodeGenFunctionBase", "setInsertPoint");
    Entry = BasicBlock::Create(CGM->LLVMCtx, "entry", Fn);
    CGM->Builder->SetInsertPoint(Entry);
}

void CodeGenFunctionBase::AllocaVars() {
    // CodeGen of Params are contained into LocalVars
    // Allocation of declared local vars
    for (auto &EntryLocalVar: AST->getBody()->getLocalVars()) {
        ASTLocalVar *LocalVar = EntryLocalVar.getValue();
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
}

void CodeGenFunctionBase::GenBody() {
    FLY_DEBUG("CodeGenFunctionBase", "GenBody");
    setInsertPoint();
    AllocaVars();
    CGM->GenBlock(Fn, AST->getBody()->getContent());
}
