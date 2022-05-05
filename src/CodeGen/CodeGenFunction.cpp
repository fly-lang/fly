//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGFunction.cpp - Code Generator Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenFunction.h"
#include "CodeGen/CodeGenLocalVar.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTFunction.h"
#include "AST/ASTParams.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTBlock.h"
#include "Basic/Debug.h"
#include "llvm/IR/DerivedTypes.h"

using namespace fly;

CodeGenFunction::CodeGenFunction(CodeGenModule *CGM, ASTFunction *AST, bool isExternal) : CGM(CGM), AST(AST) {
    llvm::FunctionType *FnTy = GenFuncType(AST->getType(), AST->getParams());

    // Create Function
    llvm::GlobalValue::LinkageTypes Linkage = llvm::GlobalValue::ExternalLinkage;

    // Generate Body
    std::string Id = CodeGen::toIdentifier(AST->getName(), AST->getNameSpace()->getName());
    if (isExternal) {
        Fn = llvm::Function::Create(FnTy, Linkage, Id, CGM->getModule());
    } else {
        if (AST->getVisibility() == V_PRIVATE) {
            Linkage = GlobalValue::LinkageTypes::InternalLinkage;
        }
        Fn = llvm::Function::Create(FnTy, Linkage, Id, CGM->getModule());
    }
    Name = Fn->getName();
}

llvm::FunctionType *CodeGenFunction::GenFuncType(const ASTType *RetTyData, const ASTParams *Params) {
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

const llvm::StringRef &CodeGenFunction::getName() const {
    return Name;
}

llvm::Function *CodeGenFunction::getFunction() {
    return Fn;
}

void CodeGenFunction::GenBody() {
    FLY_DEBUG("CodeGenFunction", "GenBody");
    Entry = BasicBlock::Create(CGM->LLVMCtx, "entry", Fn);
    CGM->Builder->SetInsertPoint(Entry);

    // CodeGen of Params and Allocation
    for (auto &P: AST->getParams()->getList()) {
        CodeGenLocalVar *CGV = new CodeGenLocalVar(CGM, P);
        P->setCodeGen(CGV);
        P->getCodeGen()->Alloca();
    }

    // Allocation of declared local vars
    for (auto &LocalVar: AST->getLocalVars()) {
        LocalVar->getCodeGen()->Alloca();
    }

    // Store Param Values
    int n = 0;
    for (auto &P: AST->getParams()->getList()) {
        CodeGenLocalVar *CGV = P->getCodeGen();
        CGV->Store(Fn->getArg(n));
        if (P->getExpr()) {
            CGM->GenExpr(Fn, P->getType(), P->getExpr());
        }
        ++n;
    }
    CGM->GenBlock(Fn, AST->getBody()->getContent());
}
