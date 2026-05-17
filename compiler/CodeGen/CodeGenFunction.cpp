//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGFunction.cpp - Code Generator Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenFunction.h"

#include "AST/ASTFunction.h"
#include "AST/ASTModule.h"
#include "AST/ASTType.h"
#include "Basic/Debug.h"
#include "Sema/SemaBlockStmt.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenError.h"
#include "CodeGen/CodeGenModule.h"
#include "Sema/SemaFunction.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"

#include <Sema/SemaBuiltin.h>
#include <Sema/SemaError.h>
#include <Sema/SemaParam.h>
#include <Sema/SemaType.h>

using namespace fly;

CodeGenFunction::CodeGenFunction(CodeGenModule *CGM, SemaFunction *Sema, bool isExternal) :
    CodeGenFunctionBase(CGM, Sema), isExternal(isExternal), isMain(isMainFunction(Sema)) {

	// Set Id
	// Id = toIdentifier(Sema);

    // Generate Params Types
    if (isMain) {
        RetType = CodeGen::Int32Ty;
        // Always expose argc/argv so env_init() can store them for fly.os.env*
        ParamTypes.push_back(CodeGen::Int32Ty);                           // int argc
        ParamTypes.push_back(llvm::PointerType::getUnqual(CGM->LLVMCtx)); // char** argv
        // Do NOT call GenParamTypes — the C entry point uses argc/argv, not the Fly param type
    } else {
        GenReturnType();

        // Check if RetType was successfully generated
        if (!RetType) {
            CGM->Diag(diag::err_codegen_invalid_type);
            // Use void as fallback to prevent crash
            RetType = CodeGen::VoidTy;
        }

        // Add ErrorHandler as first param
        ParamTypes.push_back(CodeGen::ErrorPtrTy);
        GenParamTypes(CGM, ParamTypes, Sema);
    }

    // Validate all types before creating function
    if (!RetType) {
        RetType = CodeGen::VoidTy;
    }
    for (auto &Ty : ParamTypes) {
        if (!Ty) {
            CGM->Diag(diag::err_codegen_invalid_type);
            return; // Cannot create function with invalid parameter types
        }
    }

    // Create LLVM Function
    FnType = llvm::FunctionType::get(RetType, ParamTypes, false);

    // Set Name: main() is the C entry point and must not be mangled
	std::string Name = isMain ? "main" : Mangle(Sema);
    Fn = llvm::Function::Create(FnType, llvm::GlobalValue::ExternalLinkage, Name, CGM->getModule());

    // Set Linkage
    if (isExternal && Sema->getVisibility() == SemaVisibilityKind::PRIVATE) {
        Fn->setLinkage(llvm::GlobalValue::LinkageTypes::InternalLinkage);
    }
}

/**
 * Alloca Error Handler
 * Alloca Local Vars
 */
void CodeGenFunction::GenBody() {
    FLY_DEBUG_SCOPE("CodeGenFunction", "GenBody");
    setInsertPoint();

	// Store in Function Error Handler
	if (isMain) {

		// Alloca Function Parameters and Local Vars
		AllocaLocalVars();

		// Alloca Error Handler
		Sema->getErrorHandler()->accept(*CGM);

		// For main() the error context has no caller-provided pointer, so allocate
		// the error struct locally and store its address into the handler alloca.
		CodeGenError *CGE = Sema->getErrorHandler()->getCodeGen();
		llvm::AllocaInst *ErrStruct = CGM->Builder->CreateAlloca(CodeGen::ErrorTy, nullptr, "main_err");
		CGM->Builder->CreateStore(ErrStruct, CGE->getPointer());

		// Store Default No Error in Error Handler
		CGE->Init(); // Initialize the error handler struct with default values (0 for int, null for pointer)

		// Call env_init(argc, argv) so fly.os.env* functions can access command-line args
		{
			llvm::Value *Argc = Fn->getArg(0);
			llvm::Value *Argv = Fn->getArg(1);
			llvm::FunctionCallee EnvInitFn = CGM->Module->getOrInsertFunction(
				"env_init",
				llvm::FunctionType::get(CodeGen::VoidTy,
					{CodeGen::Int32Ty, llvm::PointerType::getUnqual(CGM->LLVMCtx)}, false));
			CGM->Builder->CreateCall(EnvInitFn, {Argc, Argv});
		}

	} else {

		// Alloca Function Error Handler
		Sema->getErrorHandler()->accept(*CGM);

		// Alloca Function Parameters and Local Vars
		AllocaLocalVars();

		// Store Error Handler
		Sema->getErrorHandler()->getCodeGen()->StoreErrorHandler(Fn->getArg(0));

		// Store in Function Parameters
		StoreParams(1);
	}

	// Set error handler with function error handler
	CGM->CurrentErrorHandler = Sema->getErrorHandler()->getCodeGen();

	// Generate Function Body from Sema tree
	if (Sema->getBody()) {
		Sema->getBody()->accept(*CGM);
	}

    // if is Main check error and return right exit code
    if (isMain) {
        llvm::Value *Zero32 = llvm::ConstantInt::get(CodeGen::Int32Ty, 0);
        // take return value from error struct
        CodeGenError *CGE = Sema->getErrorHandler()->getCodeGen();
        llvm::Value * ErrorHandler = CGE->getValue();
        llvm::Value *ErrorVal = CGM->Builder->CreateInBoundsGEP(CGE->getType(), ErrorHandler, {Zero32, Zero32});
        // llvm::Value *Ret = CGM->Builder->CreateICmpNE(BuiErrorVal->, Zero32);
        // main() will return 0 if ok or 1 on error
        CGM->Builder->CreateRet(CGM->Builder->CreateLoad(CodeGen::Int32Ty, ErrorVal));
    } else {
    	CheckReturnVoid();
    }
}

bool CodeGenFunction::isMainFunction(SemaFunction *Sema) {
    // All functions are implicitly void, so just check the name
    return Sema->getAST().getName() == StringRef("main");
}

