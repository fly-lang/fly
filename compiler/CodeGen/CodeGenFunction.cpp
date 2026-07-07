//===--------------------------------------------------------------------------------------------------------------===//
// compiler/CodeGen/CodeGenFunction.cpp - function code generation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenFunction.h"
#include "CodeGen/CodeGenHelper.h"

#include "AST/ASTFunction.h"
#include "AST/ASTModule.h"
#include "AST/ASTType.h"
#include "Basic/Debug.h"
#include "Sema/SemaBlockStmt.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenError.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenVar.h"
#include "Sema/SemaFunction.h"
#include "Sema/SemaBlockStmt.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"

#include <Sema/SemaBuiltin.h>
#include <Sema/SemaError.h>
#include <Sema/SemaParam.h>
#include <Sema/SemaType.h>

using namespace fly;

bool CodeGenFunction::isCABIRuntime(SemaFunction *Sema) {
    // A fly.runtime function with a real (non-empty) body is emitted as an
    // unmangled C symbol. Empty-body fly.runtime entries are pure externs
    // (libc/libm) and are never defined here — see CodeGenModule::visit.
    return Sema->getNamespaceName() == "fly_runtime" &&
           Sema->getBody() && !Sema->getBody()->isEmpty();
}

CodeGenFunction::CodeGenFunction(CodeGenModule *CGM, SemaFunction *Sema, bool isExternal) :
    CodeGenFunctionBase(CGM, Sema), isExternal(isExternal), isMain(isMainFunction(Sema)) {

	// Set Id
	// Id = toIdentifier(Sema);

    isCABI = !isMain && isCABIRuntime(Sema);

    // Generate Params Types
    if (isCABI) {
        // C-ABI: const params by value (in order); first non-const param becomes the
        // return value; no error param. Mirrors the fly.runtime call-site convention
        // in CodeGenStdLibRuntime::GenCall.
        for (auto *Param : Sema->getParams()) {
            if (!Param->getType()) continue;
            Param->getType()->accept(*CGM);
            if (!Param->getType()->getCodeGen()) continue;
            llvm::Type *ValTy = Param->getType()->getCodeGen()->getType();
            if (Param->isConstant()) {
                ParamTypes.push_back(ValTy);           // by value
            } else if (CABIOutParam == nullptr) {
                CABIOutParam = Param;                   // first non-const -> return
            }
        }
        RetType = CABIOutParam ? CABIOutParam->getType()->getCodeGen()->getType()
                               : CodeGen::VoidTy;
    } else if (isMain) {
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
            RetType = CodeGen::VoidTy;
        }

        // Functions that use the out-param convention have LLVM return type void;
        // the user-declared return type is carried by the hidden 'out' pointer param.
        if (RetType != CodeGen::VoidTy) {
            auto &Params = Sema->getParams();
            if (!Params.empty() && Params.back()->getName() == "out")
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

    // Set Name: main() is the C entry point and must not be mangled; C-ABI runtime
    // functions export their exact (unmangled) name so fly.runtime call sites resolve.
	std::string Name = (isMain || isCABI) ? std::string(Sema->getName()) : CodeGenHelper::Mangle(Sema);
	if (isMain) Name = "main";
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

    // A generic function specialization (e.g. fly.llvm.slotFreeT<ASTName>) is emitted
    // in every module/archive that instantiates it; downgrade its DEFINITION to weak
    // (mergeable) linkage to avoid duplicate-symbol errors at link. main / C-ABI /
    // regular functions are never specializations, so they keep external linkage; and
    // declaration-only references never reach GenBody, so they stay valid external.
    if (Fn && static_cast<SemaFunction *>(Sema)->getGenericTemplate() != nullptr)
        Fn->setLinkage(llvm::GlobalValue::LinkOnceODRLinkage);

    // Only C-ABI runtime functions return-by-value; reset so a bare `return` in a
    // normal function still emits `ret void`.
    CGM->CABIReturnPtr = nullptr;
    CGM->CABIReturnTy  = nullptr;

    if (isCABI) {
        GenCABIBody();
        return;
    }

    GenDebugSubprogram();

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

// Emit a fly.runtime C-ABI body: incoming by-value args are copied into param
// allocas; the first non-const param is a fresh local returned by value at the end.
void CodeGenFunction::GenCABIBody() {
    // Local error context so any error-referencing call in the body still works.
    Sema->getErrorHandler()->accept(*CGM);
    CodeGenError *CGE = Sema->getErrorHandler()->getCodeGen();
    llvm::AllocaInst *ErrStruct = CGM->Builder->CreateAlloca(CodeGen::ErrorTy, nullptr, "rt_err");
    CGM->Builder->CreateStore(ErrStruct, CGE->getPointer());
    CGE->Init();
    CGM->CurrentErrorHandler = CGE;

    // Give every param a stack slot; store incoming by-value args for const params.
    size_t ArgIdx = 0;
    for (auto *Param : Sema->getParams()) {
        Param->accept(*CGM);
        llvm::Type *ValTy = Param->getType()->getCodeGen()->getType();
        llvm::AllocaInst *Slot = CGM->Builder->CreateAlloca(ValTy);
        Param->getCodeGen()->setPointer(Slot);
        if (Param->isConstant()) {
            CGM->Builder->CreateStore(Fn->getArg(ArgIdx), Slot);
            ++ArgIdx;
        }
    }

    // Make bare `return` statements yield the out param by value.
    if (CABIOutParam) {
        CGM->CABIReturnPtr = CABIOutParam->getCodeGen()->getPointer();
        CGM->CABIReturnTy  = CABIOutParam->getType()->getCodeGen()->getType();
    }

    // Local variables.
    for (auto &LocalVar : Sema->getLocalVars()) {
        LocalVar->accept(*CGM);
        LocalVar->getCodeGen()->Alloca();
    }

    // Body.
    if (Sema->getBody())
        Sema->getBody()->accept(*CGM);

    // Return the (first non-const) out param by value, or void.
    if (!CGM->Builder->GetInsertBlock()->getTerminator()) {
        if (CABIOutParam) {
            llvm::Type *RetTy = CABIOutParam->getType()->getCodeGen()->getType();
            llvm::Value *Val = CGM->Builder->CreateLoad(RetTy, CABIOutParam->getCodeGen()->getPointer());
            CGM->Builder->CreateRet(Val);
        } else {
            CGM->Builder->CreateRetVoid();
        }
    }

    CGM->CABIReturnPtr = nullptr;
    CGM->CABIReturnTy  = nullptr;
}

bool CodeGenFunction::isMainFunction(SemaFunction *Sema) {
    // All functions are implicitly void, so just check the name
    return Sema->getAST().getName() == StringRef("main");
}

