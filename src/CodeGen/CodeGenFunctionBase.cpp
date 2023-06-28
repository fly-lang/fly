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
#include "CodeGen/CodeGenInstance.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTFunction.h"
#include "AST/ASTParams.h"
#include "AST/ASTCall.h"
#include "AST/ASTType.h"
#include "Basic/Debug.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/ADT/StringRef.h"
#include "AST/ASTClassFunction.h"

using namespace fly;

CodeGenFunctionBase::CodeGenFunctionBase(CodeGenModule *CGM, ASTFunctionBase *AST) : CGM(CGM), AST(AST) {

}

void CodeGenFunctionBase::GenTypes(CodeGenModule * CGM, SmallVector<llvm::Type *, 8> &Types, const ASTParams *Params) {
    // Populate Types by reference
    if (!Params->getList().empty()) {
        for (auto Param : Params->getList()) {
            llvm::Type *ParamTy = CGM->GenType(Param->getType());
            Types.push_back(ParamTy);
        }
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

void CodeGenFunctionBase::setInsertPoint() {
    FLY_DEBUG("CodeGenFunctionBase", "setInsertPoint");
    Entry = BasicBlock::Create(CGM->LLVMCtx, "entry", Fn);
    CGM->Builder->SetInsertPoint(Entry);
}

void CodeGenFunctionBase::AllocaVars() {
    // Allocation of declared ASTParams
    for (auto &Param: AST->getParams()->getList()) {
        Param->setCodeGen(newCodeGen(Param));
        Param->getCodeGen()->Init();
    }

    // Allocation of all declared ASTLocalVar
    for (auto &EntryLocalVar: AST->getBody()->getLocalVars()) {
        ASTLocalVar *LocalVar = EntryLocalVar.getValue();
        LocalVar->setCodeGen(newCodeGen(LocalVar));
        LocalVar->getCodeGen()->Init();
    }

    // Store Param Values
    int n = 0;
    for (auto &Param: AST->getParams()->getList()) {
        CodeGenVarBase *CGV = Param->getCodeGen();
        CGV->Store(Fn->getArg(n));
        if (Param->getExpr()) {
            CGM->GenExpr(Fn, Param->getType(), Param->getExpr());
        }
        ++n;
    }
}

void CodeGenFunctionBase::GenBody() {
    FLY_DEBUG("CodeGenFunctionBase", "GenBody");
    setInsertPoint();
    AllocaVars();
    CGM->GenBlock(Fn, AST->getBody()->getContent());
    
    // Add return Void
    BasicBlock &BB = *Fn->getBasicBlockList().end();
    Instruction &I = *BB.end();
    if (FnTy->getReturnType()->isVoidTy()) {
        CGM->Builder->CreateRetVoid();
    }
}

CodeGenVarBase *CodeGenFunctionBase::newCodeGen(ASTVar *Var) {
    if (Var->getType()->isClass()) {
        return new CodeGenInstance(CGM, Var);
    }
    return new CodeGenVar(CGM, Var);
}
