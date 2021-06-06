//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGFunction.cpp - Code Generator Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CGFunction.h"
#include "CodeGen/CodeGen.h"
#include "llvm/IR/DerivedTypes.h"

using namespace fly;

CGFunction::CGFunction(CodeGenModule *CGM, const llvm::StringRef FName, const TypeBase *FType,
                       const ParamsFuncDecl *FParams, const BlockStmt *FBlock) :
        CGM(CGM) {
    llvm::FunctionType *FnTy = GenFuncType(FType, FParams);

    // Create Function
    Fn = llvm::Function::Create(FnTy, llvm::Function::ExternalLinkage, FName);
    Name = Fn->getName();
    if (FBlock) {
        BasicBlock *BB = BasicBlock::Create(CGM->VMContext, "entry", Fn);
        CGM->Builder->SetInsertPoint(BB);
    }
}

llvm::FunctionType *CGFunction::GenFuncType(const TypeBase *RetTyData, const ParamsFuncDecl *Params) {
    if (Params->getVars().empty()) {
        // Create empty Function Type
        return llvm::FunctionType::get(CGM->GenTypeValue(RetTyData), Params->getVarArg() != nullptr);
    } else {
        // Create Function Type with parameters
        SmallVector<llvm::Type *, 8> ArrayParams;
        for (auto Param : Params->getVars()) {
            llvm::Type *ParamTy = CGM->GenTypeValue(Param->getType());
            ArrayParams.push_back(ParamTy);
        }
        return llvm::FunctionType::get(CGM->GenTypeValue(RetTyData), ArrayParams, Params->getVarArg() != nullptr);
    }
}

const llvm::StringRef &CGFunction::getName() const {
    return Name;
}

llvm::Function *CGFunction::getFunction() {
    return Fn;
}

void *CGFunction::Call() {
    CGM->Builder->CreateCall(Fn);
}
