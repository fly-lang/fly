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
#include "AST/ASTLocalVar.h"
#include "AST/ASTBlock.h"
#include "llvm/IR/DerivedTypes.h"

using namespace fly;

CodeGenFunction::CodeGenFunction(CodeGenModule *CGM, ASTFunc *Func, bool isExternal) : CGM(CGM) {
    llvm::FunctionType *FnTy = GenFuncType(Func->getType(), Func->getHeader());

    // Create Function
    llvm::GlobalValue::LinkageTypes Linkage;

    // Generate Body
    if (isExternal) {
        Linkage = llvm::GlobalValue::ExternalLinkage;
        Fn = llvm::Function::Create(FnTy, Linkage, Func->getName(), CGM->getModule());
    } else {
        if (Func->getVisibility() == V_PRIVATE) {
            Linkage = GlobalValue::LinkageTypes::InternalLinkage;
        }
        Fn = llvm::Function::Create(FnTy, Linkage, Func->getName(), CGM->getModule());
        Entry = BasicBlock::Create(CGM->LLVMCtx, "entry", Fn);
        CGM->Builder->SetInsertPoint(Entry);

        // CodeGen of Params and Allocation
        for (auto &P: Func->getHeader()->getParams()) {
            CodeGenLocalVar *CGV = new CodeGenLocalVar(CGM, P);
            P->setCodeGen(CGV);
            P->getCodeGen()->Alloca();
        }

        // Allocation of declared vars
        for (auto &DeclVar: Func->getDeclVars()) {
            DeclVar->getCodeGen()->Alloca();
        }

        // Store Param Values
        int n = 0;
        for (auto &P: Func->getHeader()->getParams()) {
            CodeGenLocalVar *CGV = P->getCodeGen();
            CGV->Store(Fn->getArg(n));
            if (P->getExpr()) {
                CGM->GenExpr(Fn, P->getType(), P->getExpr());
            }
            ++n;
        }

        // Add Function Body
        CGM->GenBlock(Fn, Func->getBody()->getContent());
    }
    Name = Fn->getName();
}

llvm::FunctionType *CodeGenFunction::GenFuncType(const ASTType *RetTyData, const ASTFuncHeader *Params) {
    if (Params->getParams().empty()) {
        // Create empty Function Type
        return llvm::FunctionType::get(CGM->GenType(RetTyData), Params->getVarArg() != nullptr);
    } else {
        // Create Function Type with parameters
        SmallVector<llvm::Type *, 8> ArrayParams;
        for (auto Param : Params->getParams()) {
            llvm::Type *ParamTy = CGM->GenType(Param->getType());
            ArrayParams.push_back(ParamTy);
        }
        return llvm::FunctionType::get(CGM->GenType(RetTyData), ArrayParams, Params->getVarArg() != nullptr);
    }
}

const llvm::StringRef &CodeGenFunction::getName() const {
    return Name;
}

llvm::Function *CodeGenFunction::getFunction() {
    return Fn;
}
