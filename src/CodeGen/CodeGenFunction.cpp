//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGFunction.cpp - Code Generator Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenFunction.h"
#include "CodeGen/CodeGenVar.h"
#include "CodeGen/CodeGen.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTBlock.h"
#include "llvm/IR/DerivedTypes.h"

using namespace fly;

CodeGenFunction::CodeGenFunction(CodeGenModule *CGM, const llvm::StringRef FName, const ASTType *FType,
                                 const ASTFuncHeader *FParams, const ASTBlock *FBody) : CGM(CGM) {
    llvm::FunctionType *FnTy = GenFuncType(FType, FParams);

    // Create Function
    Fn = llvm::Function::Create(FnTy, llvm::Function::ExternalLinkage, FName, CGM->Module);
    Name = Fn->getName();
    Entry = BasicBlock::Create(CGM->LLVMCtx, "entry", Fn);
    CGM->Builder->SetInsertPoint(Entry);

    // CodeGen of Params and Allocation
    for (auto &P : FParams->getParams()) {
        CodeGenVar *CGV = new CodeGenVar(CGM, P);
        P->setCodeGen(CGV);
    }

    // CodeGen of Vars and Allocation
    for (auto &EntryV : FBody->getDeclVars()) {
        CodeGenVar *CGV = new CodeGenVar(CGM, EntryV.getValue());
        EntryV.getValue()->setCodeGen(CGV);
    }

    // Store Values
    int n = 0;
    for (auto &P : FParams->getParams()) {
        CodeGenVar *CGV = P->getCodeGen();
        CGV->Store(Fn->getArg(n));
        if (P->getExpr()) { // TODO check Expr
            CGM->GenExpr(P->getType(), P->getExpr());
        }
        ++n;
    }

    // Add Function Body
    for (auto &S : FBody->getContent()) {
        CGM->GenStmt(S);
    }
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

BasicBlock *CodeGenFunction::getEntry() {
    return Entry;
}
