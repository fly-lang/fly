//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGFunction.cpp - Code Generator Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenFunctionBase.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenError.h"
#include "AST/ASTFunction.h"
#include "Sema/SemaClassMethod.h"
#include "Sema/SemaClassAttribute.h"
#include "Basic/Debug.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/ADT/StringRef.h"

#include <CodeGen/CodeGenVar.h>
#include <Sema/SemaErrorHandler.h>
#include <Sema/SemaParam.h>
#include <llvm/IR/Instructions.h>

using namespace fly;

CodeGenFunctionBase::CodeGenFunctionBase(CodeGenModule *CGM, SemaFunctionBase *Sema) : CGM(CGM), Sema(Sema) {

}

CodeGenModule *CodeGenFunctionBase::getCodeGenModule() {
    return CGM;
}

void CodeGenFunctionBase::GenReturnType() {
    RetType = CGM->GenType(Sema->getReturnType());
}

void CodeGenFunctionBase::GenParamTypes(CodeGenModule * CGM, llvm::SmallVector<llvm::Type *, 8> &Types, SemaFunctionBase * Sema) {
    // Populate Types by reference
    if (Sema->getParams().empty()) {
        return;
    }
    for (auto Param : Sema->getParams()) {
        llvm::Type *ParamTy = CGM->GenType(Param->getType());
        Types.push_back(ParamTy);
    }
//    if (Params->getEllipsis() != nullptr) {
        // TODO
//    }
}

SemaFunctionBase *CodeGenFunctionBase::getSema() {
    return Sema;
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
    FLY_DEBUG_START("CodeGenFunctionBase", "setInsertPoint");
    Entry = llvm::BasicBlock::Create(CGM->LLVMCtx, "entry", Fn);
    CGM->Builder->SetInsertPoint(Entry);
}

void CodeGenFunctionBase::AllocaErrorHandler() {
	// Set Error Handler
	if (Sema->getErrorHandler()) {
		CodeGenError *CGE = CGM->GenErrorHandler(Sema->getErrorHandler());
		Sema->getErrorHandler()->setCodeGen(CGE);
	}
}

void CodeGenFunctionBase::AllocaLocalVars() {

    // Allocation of declared ASTVar
    for (auto Param: Sema->getParams()) {
        CodeGenVar *CGV = CGM->GenLocalVar(Param);
        CGV->Alloca();
        Param->setCodeGen(CGV);
    }

    // Allocation of all declared ASTVar
    for (auto &LocalVar: Sema->getLocalVars()) {
    	if (LocalVar->getType()->isError()) {
    		CodeGenError *CGE = CGM->GenErrorHandler(LocalVar);
    		LocalVar->setCodeGen(CGE);
        } else {
	        CodeGenVar *CGV = CGM->GenLocalVar(LocalVar);
        	CGV->Alloca();
        	LocalVar->setCodeGen(CGV);
        }
    }
}

void CodeGenFunctionBase::StoreParams() {
    // Store Param Values (n = 0 is the Error param)
    int Idx = 1;

    for (auto &Param: Sema->getParams()) {

        // Store Arg value into Param
        CodeGenVarBase *CGV = Param->getCodeGen();
        CGV->Store(Fn->getArg(Idx));

        // TODO if Arg is not present store the Param default value
//        if (Param->getExpr()) {
//            CGM->GenExpr(this, Param->getType(), Param->getExpr());
//        }
        ++Idx;
    }
}
