//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGFunction.cpp - Code Generator Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenFunction.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenError.h"
#include "AST/ASTModule.h"
#include "AST/ASTFunction.h"
#include "Sema/SemaFunction.h"
#include "AST/ASTType.h"
#include "Basic/Debug.h"
#include "llvm/IR/Function.h"
#include "llvm/ADT/StringRef.h"
#include <Sema/SemaErrorHandler.h>
#include <Sema/SemaType.h>

using namespace fly;

CodeGenFunction::CodeGenFunction(CodeGenModule *CGM, SemaFunction *Sema, bool isExternal) :
    CodeGenFunctionBase(CGM, Sema), isExternal(isExternal), isMain(isMainFunction(Sema)) {

	// Set Id
	Id = toIdentifier(Sema);

    // Generate Params Types
    if (isMain) {
        RetType = CGM->Int32Ty;
    } else {
        GenReturnType();

        // Add ErrorHandler as first param
        ParamTypes.push_back(CGM->ErrorPtrTy);
    }
    GenParamTypes(CGM, ParamTypes, Sema);

    // Create LLVM Function
    FnType = llvm::FunctionType::get(RetType, ParamTypes, false);

    // Set Name
    Fn = llvm::Function::Create(FnType, llvm::GlobalValue::ExternalLinkage, Id, CGM->getModule());

    // Set Linkage
    if (isExternal && Sema->getVisibility() == SemaVisibilityKind::PRIVATE) {
        Fn->setLinkage(llvm::GlobalValue::LinkageTypes::InternalLinkage);
    }
}

std::string CodeGenFunction::toIdentifier(SemaFunction *Function) {
	FLY_DEBUG_START("CodeGenFunction", "toIdentifier");
	// For functions, use the mangled name
	std::string MangledName = Function->getMangledName();
	SemaNameSpace *NameSpace = CGM->getNameSpace();
	return CGM->toIdentifier(MangledName, NameSpace);
}

/**
 * Alloca Error Handler
 * Alloca Local Vars
 */
void CodeGenFunction::GenBody() {
    FLY_DEBUG_START("CodeGenFunction", "GenBody");
    setInsertPoint();

	// Store in Function Error Handler
	if (isMain) {

		// Alloca Function Parameters and Local Vars
		AllocaLocalVars();

		llvm::Constant *Zero = llvm::ConstantInt::get(CGM->Int32Ty, 0);
		llvm::Constant *One = llvm::ConstantInt::get(CGM->Int32Ty, 1);
		llvm::Constant *Two = llvm::ConstantInt::get(CGM->Int32Ty, 2);
		llvm::Constant *NullPtr = llvm::ConstantPointerNull::get(CGM->Int8Ty->getPointerTo());
		CodeGenVarBase *CGE = CGM->GenErrorHandler(Sema->getErrorHandler());
		llvm::Value *ErrorVar = CGE->Load();
		llvm::Value *PtrType = CGM->Builder->CreateInBoundsGEP(CGE->getType(), ErrorVar, {Zero, Zero});
		CGM->Builder->CreateStore(llvm::ConstantInt::get(CGM->Int8Ty, 0), PtrType);
		llvm::Value *PtrInt = CGM->Builder->CreateInBoundsGEP(CGE->getType(), ErrorVar, {Zero, One});
		CGM->Builder->CreateStore(Zero, PtrInt);
		llvm::Value *PtrPtr = CGM->Builder->CreateInBoundsGEP(CGE->getType(), ErrorVar, {Zero, Two});
		CGM->Builder->CreateStore(NullPtr, PtrPtr);
	} else {

		// Alloca Function Error Handler
		AllocaErrorHandler();

		// Alloca Function Parameters and Local Vars
		AllocaLocalVars();

		// Store Error Handler
		Sema->getErrorHandler()->getCodeGen()->StoreErrorHandler(Fn->getArg(0));

		// Store in Function Parameters
		StoreParams(1);
	}

	// Generate Function Body
    CGM->GenBlockStmt(this, Sema->getAST().getBody());

    // if is Main check error and return right exit code
    if (isMain) {
        llvm::Value *Zero32 = llvm::ConstantInt::get(CGM->Int32Ty, 0);
        llvm::Value *Zero8 = llvm::ConstantInt::get(CGM->Int8Ty, 0);
        // take return value from error struct
        CodeGenError *CGE = Sema->getErrorHandler()->getCodeGen();
        llvm::Value * ErrorHandler = CGE->getValue();
        llvm::Value *ErrorKind = CGM->Builder->CreateInBoundsGEP(CGE->getType(), ErrorHandler, {Zero32, Zero32});
        llvm::Value *Ret = CGM->Builder->CreateICmpNE(CGM->Builder->CreateLoad(ErrorKind), Zero8);
        // main() will return 0 if ok or 1 on error
        CGM->Builder->CreateRet(CGM->Builder->CreateZExt(Ret, Fn->getReturnType()));
    } else {
    	CheckReturnVoid();
    }
}

bool CodeGenFunction::isMainFunction(SemaFunction *Sema) {
    return Sema->getAST().getName() == StringRef("main") && Sema->getAST().getReturnType()->getSema()->isVoid();
}
