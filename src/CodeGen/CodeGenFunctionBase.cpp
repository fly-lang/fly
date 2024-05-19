//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGFunction.cpp - Code Generator Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenFunctionBase.h"
#include "CodeGen/CodeGenInstance.h"
#include "CodeGen/CodeGenEnumEntry.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenError.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTFunction.h"
#include "AST/ASTParam.h"
#include "AST/ASTCall.h"
#include "AST/ASTType.h"
#include "AST/ASTIdentityType.h"
#include "AST/ASTClassMethod.h"
#include "Basic/Debug.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/ADT/StringRef.h"

using namespace fly;

CodeGenFunctionBase::CodeGenFunctionBase(CodeGenModule *CGM, ASTFunctionBase *AST) : CGM(CGM), AST(AST) {
    CodeGenError *CGE = CGM->GenErrorHandler(AST->getErrorHandler());
    AST->getErrorHandler()->setCodeGen(CGE);
}

CodeGenModule *CodeGenFunctionBase::getCodeGenModule() {
    return CGM;
}

void CodeGenFunctionBase::GenReturnType() {
    RetType = CGM->GenType(AST->getType());
}

void CodeGenFunctionBase::GenParamTypes(CodeGenModule * CGM, llvm::SmallVector<llvm::Type *, 8> &Types, llvm::SmallVector<ASTParam *, 8> Params) {
    // Populate Types by reference
    if (Params.empty()) {
        return;
    }
    if (!Params.empty()) {
        for (auto Param : Params) {
            llvm::Type *ParamTy = CGM->GenType(Param->getType());
            Types.push_back(ParamTy);
        }
    }
//    if (Params->getEllipsis() != nullptr) {
        // TODO
//    }
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
    return FnType;
}

void CodeGenFunctionBase::setInsertPoint() {
    FLY_DEBUG("CodeGenFunctionBase", "setInsertPoint");
    Entry = BasicBlock::Create(CGM->LLVMCtx, "entry", Fn);
    CGM->Builder->SetInsertPoint(Entry);
}

void CodeGenFunctionBase::AllocaVars() {
    // Alloca ErrorHandler
    AST->getErrorHandler()->getCodeGen()->Init();

    // Allocation of declared ASTParam
    for (auto &Param: AST->getParams()) {
        Param->setCodeGen(CGM->GenVar(Param));
        Param->getCodeGen()->Init();
    }

    // Allocation of all declared ASTLocalVar
    for (auto &EntryLocalVar: AST->getBody()->getLocalVars()) {
        ASTLocalVar *LocalVar = EntryLocalVar.getValue();
        LocalVar->setCodeGen(CGM->GenVar(LocalVar));
        LocalVar->getCodeGen()->Init();
    }
}

void CodeGenFunctionBase::StoreParams(bool isMain) {
    // Store Param Values (n = 0 is the Error param)
    int n = isMain ? 0 : 1;

    if (!isMain)
        CGM->Builder->CreateStore(Fn->getArg(0), AST->getErrorHandler()->getCodeGen()->getPointer());

    for (auto &Param: AST->getParams()) {

        // Store Arg value into Param
        CodeGenVarBase *CGV = Param->getCodeGen();
        CGV->Store(Fn->getArg(n)); // FIXME only for params passed by value

        // TODO if Arg is not present store the Param default value
//        if (Param->getExpr()) {
//            CGM->GenExpr(this, Param->getType(), Param->getExpr());
//        }
        ++n;
    }
}
