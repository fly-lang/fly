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
#include "AST/ASTNameSpace.h"
#include "AST/ASTFunction.h"
#include "AST/ASTVar.h"
#include "AST/ASTCall.h"
#include "AST/ASTClass.h"
#include "Sema/SemaClassMethod.h"
#include "Sema/SemaClassAttribute.h"
#include "Basic/Debug.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/ADT/StringRef.h"

#include <AST/ASTTypeRef.h>
#include <CodeGen/CodeGenVar.h>
#include <Sema/SemaClassType.h>
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
    // Alloca ErrorHandler
    CodeGenError *CGE = CGM->GenErrorHandler(Sema->getErrorHandler());
    Sema->getErrorHandler()->setCodeGen(CGE);
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

    		// Create CodeGenVar for all inner attributes of a class
    		if (LocalVar->getType()->isClass()) {
    			SemaClassType *Class = static_cast<SemaClassType *>(LocalVar->getType());
    			// Class start with param 0 with the vtable type
    			uint32_t Idx = Class->getClassKind() == SemaClassKind::STRUCT ? 0 : 1;
    			for (auto &Entry : Class->getAttributes()) {
    				SemaClassAttribute *Attribute = Entry.getValue();
    				CodeGenClassVar *CGAttr = new CodeGenClassVar(CGM, CGM->GenType(Attribute->getType()), CGV, Idx);
    				CGV->addVar(Attribute->getAST()->getName(), CGAttr);
    				Attribute->setCodeGen(CGAttr);
    				Idx++;
    			}
    		}
			LocalVar->setCodeGen(CGV);
    	}
    }
}

void CodeGenFunctionBase::StoreErrorHandler(bool isMain) {
    if (isMain) {
        llvm::Constant *Zero = llvm::ConstantInt::get(CGM->Int32Ty, 0);
        llvm::Constant *One = llvm::ConstantInt::get(CGM->Int32Ty, 1);
        llvm::Constant *Two = llvm::ConstantInt::get(CGM->Int32Ty, 2);
        llvm::Constant *NullPtr = llvm::ConstantPointerNull::get(CGM->Int8Ty->getPointerTo());
        CodeGenVarBase *CGE = Sema->getErrorHandler()->getCodeGen();
        llvm::Value *ErrorVar = CGE->Load();
        llvm::Value *PtrType = CGM->Builder->CreateInBoundsGEP(CGE->getType(), ErrorVar, {Zero, Zero});
        CGM->Builder->CreateStore(llvm::ConstantInt::get(CGM->Int8Ty, 0), PtrType);
        llvm::Value *PtrInt = CGM->Builder->CreateInBoundsGEP(CGE->getType(), ErrorVar, {Zero, One});
        CGM->Builder->CreateStore(Zero, PtrInt);
        llvm::Value *PtrPtr = CGM->Builder->CreateInBoundsGEP(CGE->getType(), ErrorVar, {Zero, Two});
        CGM->Builder->CreateStore(NullPtr, PtrPtr);
    } else {
        ((CodeGenError *) Sema->getErrorHandler()->getCodeGen())->StorePointer(Fn->getArg(0));
    }
}

void CodeGenFunctionBase::StoreParams(bool isMain) {
    // Store Param Values (n = 0 is the Error param)
    int n = isMain ? 0 : 1;

    for (auto &Param: Sema->getAST()->getParams()) {

        // Store Arg value into Param
        CodeGenVarBase *CGV = Param->getSema()->getCodeGen();
        CGV->Store(Fn->getArg(n));

        // TODO if Arg is not present store the Param default value
//        if (Param->getExpr()) {
//            CGM->GenExpr(this, Param->getType(), Param->getExpr());
//        }
        ++n;
    }
}
