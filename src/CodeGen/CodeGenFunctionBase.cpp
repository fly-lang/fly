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
#include "Basic/Debug.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/ADT/StringRef.h"

using namespace fly;

CodeGenFunctionBase::CodeGenFunctionBase(CodeGenModule *CGM, ASTFunctionBase *AST) : CGM(CGM), AST(AST) {
    llvm::FunctionType *FnTy = GenFuncType(AST->getType(), AST->getParams());
    Fn = llvm::Function::Create(FnTy, llvm::GlobalValue::ExternalLinkage, "", CGM->getModule());
}

llvm::FunctionType *CodeGenFunctionBase::GenFuncType(const ASTType *RetTyData, const ASTParams *Params) {
    if (Params->getList().empty()) {
        // Create empty Function Type
        return llvm::FunctionType::get(CGM->GenType(RetTyData), Params->getEllipsis() != nullptr);
    } else {
        // Create Function Type with parameters
        SmallVector<llvm::Type *, 8> ArrayParams;
        for (auto Param : Params->getList()) {
            llvm::Type *ParamTy = CGM->GenType(Param->getType());
            ArrayParams.push_back(ParamTy);
        }
        return llvm::FunctionType::get(CGM->GenType(RetTyData), ArrayParams, Params->getEllipsis() != nullptr);
    }
}

llvm::StringRef CodeGenFunctionBase::getName() const {
    return Fn->getName();
}

llvm::Function *CodeGenFunctionBase::getFunction() {
    return Fn;
}

void CodeGenFunctionBase::GenBody() {
    FLY_DEBUG("CodeGenFunction", "GenBody");
    Entry = BasicBlock::Create(CGM->LLVMCtx, "entry", Fn);
    CGM->Builder->SetInsertPoint(Entry);

    // CodeGen of Params are contained into LocalVars

    // Allocation of declared local vars
    for (auto &LocalVar: AST->getLocalVars()) {
        LocalVar->setCodeGen(new CodeGenVar(CGM, LocalVar)); // FIXME Class
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
