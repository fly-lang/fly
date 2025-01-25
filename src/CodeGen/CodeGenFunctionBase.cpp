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
#include "AST/ASTParam.h"
#include "AST/ASTCall.h"
#include "AST/ASTType.h"
#include "AST/ASTIdentityType.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassMethod.h"
#include "AST/ASTClassAttribute.h"
#include "Basic/Debug.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/ADT/StringRef.h"

using namespace fly;

CodeGenFunctionBase::CodeGenFunctionBase(CodeGenModule *CGM, ASTFunctionBase *AST) : CGM(CGM), AST(AST) {

}

CodeGenModule *CodeGenFunctionBase::getCodeGenModule() {
    return CGM;
}

void CodeGenFunctionBase::GenReturnType() {
    RetType = CGM->GenType(AST->getReturnType());
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

void CodeGenFunctionBase::AllocaErrorHandler() {
    // Alloca ErrorHandler
    CodeGenError *CGE = CGM->GenErrorHandler(AST->getErrorHandler());
    AST->getErrorHandler()->setCodeGen(CGE);
}

void CodeGenFunctionBase::AllocaLocalVars() {

    // Allocation of declared ASTParam
    for (auto &Param: AST->getParams()) {
        CodeGenVar *CGV = CGM->GenLocalVar(Param);
        CGV->Alloca();
        Param->setCodeGen(CGV);
    }

    // Allocation of all declared ASTLocalVar
    for (auto &LocalVar: AST->getLocalVars()) {
    	if (LocalVar->getType()->isError()) {
    		CodeGenError *CGE = CGM->GenErrorHandler(LocalVar);
    		LocalVar->setCodeGen(CGE);
		} else {
    		CodeGenVar *CGV = CGM->GenLocalVar(LocalVar);
    		CGV->Alloca();

    		// Create CodeGenVar for all inner attributes of a class
    		if (LocalVar->getType()->isIdentity()) {
    			ASTIdentityType *IdentityType = (ASTIdentityType *) LocalVar->getType();
    			if (IdentityType->isClass()) {
    				ASTClass * Class =  (ASTClass *) IdentityType->getDef();
    				// Class start with param 0 with the vtable type
    				uint32_t Idx = Class->getClassKind() == ASTClassKind::STRUCT ? 0 : 1;
    				for (auto &Attribute : Class->getAttributes()) {
    					CodeGenVar *CGAttr = new CodeGenVar(CGM, CGM->GenType(Attribute->getType()), CGV, Idx);
    					CGV->addVar(Attribute->getName(), CGAttr);
    					Attribute->setCodeGen(CGAttr);
    					Idx++;
    				}
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
        CodeGenVarBase *CGE = AST->getErrorHandler()->getCodeGen();
        llvm::Value *ErrorVar = CGE->Load();
        llvm::Value *PtrType = CGM->Builder->CreateInBoundsGEP(CGE->getType(), ErrorVar, {Zero, Zero});
        CGM->Builder->CreateStore(llvm::ConstantInt::get(CGM->Int8Ty, 0), PtrType);
        llvm::Value *PtrInt = CGM->Builder->CreateInBoundsGEP(CGE->getType(), ErrorVar, {Zero, One});
        CGM->Builder->CreateStore(Zero, PtrInt);
        llvm::Value *PtrPtr = CGM->Builder->CreateInBoundsGEP(CGE->getType(), ErrorVar, {Zero, Two});
        CGM->Builder->CreateStore(NullPtr, PtrPtr);
    } else {
        ((CodeGenError *) AST->getErrorHandler()->getCodeGen())->StorePointer(Fn->getArg(0));
    }
}

void CodeGenFunctionBase::StoreParams(bool isMain) {
    // Store Param Values (n = 0 is the Error param)
    int n = isMain ? 0 : 1;

    for (auto &Param: AST->getParams()) {

        // Store Arg value into Param
        CodeGenVarBase *CGV = Param->getCodeGen();
        CGV->Store(Fn->getArg(n));

        // TODO if Arg is not present store the Param default value
//        if (Param->getExpr()) {
//            CGM->GenExpr(this, Param->getType(), Param->getExpr());
//        }
        ++n;
    }
}
