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
        // If main declares a string[] args parameter, expose it as (i32 argc, ptr argv)
        if (!Sema->getParams().empty()) {
            ParamTypes.push_back(CodeGen::Int32Ty);                           // int argc
            ParamTypes.push_back(llvm::PointerType::getUnqual(CGM->LLVMCtx)); // char** argv
        }
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
    FLY_DEBUG_START("CodeGenFunction", "GenBody");
    setInsertPoint();

	// Store in Function Error Handler
	if (isMain) {

		// Alloca Function Parameters and Local Vars
		AllocaLocalVars();

		// Alloca Error Handler
		Sema->getErrorHandler()->accept(*CGM);

		// Store Default No Error in Error Handler
		CodeGenError *CGE = Sema->getErrorHandler()->getCodeGen();
		CGE->Init(); // Initialize the error handler struct with default values (0 for int, null for pointer)

		// Build the string[] args param from argc/argv when declared
		if (!Sema->getParams().empty()) {
			GenMainArgs();
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

// Build a Fly string[] from the C entry-point's argc / argv and wire it to the
// first (and only) Fly parameter of main().
//
// The resulting LLVM layout:
//   %args = alloca array                      ; { ptr data, i32 size }
//   store i32 (argc-1), %args.size
//   %data = malloc((argc-1) * sizeof(string)) ; heap array of { ptr, i32 }
//   store ptr %data, %args.data
//   for i = 0 .. argc-2:
//     data[i].ptr  = argv[i+1]
//     data[i].size = strlen(argv[i+1])
//
// argv[0] (the program name) is intentionally skipped so that args[0] is the
// first user-supplied argument — matching the convention of Java, C# and Swift.
void CodeGenFunction::GenMainArgs() {
    llvm::Value *Argc = Fn->getArg(0); // i32 argc
    llvm::Value *Argv = Fn->getArg(1); // ptr  argv (char**)

    SemaParam *ArgsParam = Sema->getParams()[0];
    CodeGenVar *CGV = ArgsParam->getCodeGen();

    // Resolve LLVM types via the Sema builtin infrastructure
    SemaType *StringSema = SemaBuiltin::getStringType();
    StringSema->accept(*CGM);
    llvm::Type *StringTy = StringSema->getCodeGen()->getType();

    SemaArrayType *ArraySema = SemaBuiltin::CreateArrayType(StringSema, (uint64_t)0);
    ArraySema->accept(*CGM);
    llvm::Type *ArrayTy = ArraySema->getCodeGen()->getType();
    delete ArraySema;

    // Alloca the Fly array struct that will hold the string[] value
    llvm::AllocaInst *ArgsAlloca = CGM->Builder->CreateAlloca(ArrayTy, nullptr, "args");
    CGV->setPointer(ArgsAlloca);

    // args.size = argc - 1  (skip argv[0] = program name)
    llvm::Value *One32 = llvm::ConstantInt::get(CodeGen::Int32Ty, 1);
    llvm::Value *ArgsCount = CGM->Builder->CreateSub(Argc, One32, "args.count");

    llvm::Value *SizeFieldPtr = CGM->Builder->CreateStructGEP(ArrayTy, ArgsAlloca, 1, "args.size.ptr");
    CGM->Builder->CreateStore(ArgsCount, SizeFieldPtr);

    // malloc(args.count * sizeof(string))
    llvm::TypeSize ElemBytes = CGM->Module->getDataLayout().getTypeAllocSize(StringTy);
    llvm::Value *ElemSize = llvm::ConstantInt::get(CodeGen::IntPtrTy, ElemBytes.getFixedValue());
    llvm::Value *ArgsCountExt = CGM->Builder->CreateSExt(ArgsCount, CodeGen::IntPtrTy, "args.count.ext");
    llvm::Value *TotalBytes = CGM->Builder->CreateMul(ArgsCountExt, ElemSize, "args.total_bytes");

    llvm::FunctionCallee MallocFn = CGM->Module->getOrInsertFunction(
        "malloc",
        llvm::FunctionType::get(llvm::PointerType::getUnqual(CGM->LLVMCtx), {CodeGen::IntPtrTy}, false));
    llvm::Value *DataPtr = CGM->Builder->CreateCall(MallocFn, {TotalBytes}, "args.data");

    llvm::Value *DataFieldPtr = CGM->Builder->CreateStructGEP(ArrayTy, ArgsAlloca, 0, "args.data.ptr");
    CGM->Builder->CreateStore(DataPtr, DataFieldPtr);

    // Loop: for i = 0; i < args.count; ++i  → data[i] = { argv[i+1], strlen(argv[i+1]) }
    llvm::BasicBlock *EntryBB  = CGM->Builder->GetInsertBlock();
    llvm::BasicBlock *CondBB   = llvm::BasicBlock::Create(CGM->LLVMCtx, "args.loop.cond", Fn);
    llvm::BasicBlock *BodyBB   = llvm::BasicBlock::Create(CGM->LLVMCtx, "args.loop.body", Fn);
    llvm::BasicBlock *ExitBB   = llvm::BasicBlock::Create(CGM->LLVMCtx, "args.loop.exit", Fn);

    CGM->Builder->CreateBr(CondBB);
    CGM->Builder->SetInsertPoint(CondBB);

    llvm::PHINode *Idx = CGM->Builder->CreatePHI(CodeGen::Int32Ty, 2, "args.i");
    Idx->addIncoming(llvm::ConstantInt::get(CodeGen::Int32Ty, 0), EntryBB);

    llvm::Value *Done = CGM->Builder->CreateICmpSGE(Idx, ArgsCount, "args.done");
    CGM->Builder->CreateCondBr(Done, ExitBB, BodyBB);
    CGM->Builder->SetInsertPoint(BodyBB);

    // Load argv[i+1]
    llvm::Value *ArgvOff = CGM->Builder->CreateAdd(Idx, One32, "argv.off");
    llvm::Value *ArgvElemPtr = CGM->Builder->CreateGEP(
        llvm::PointerType::getUnqual(CGM->LLVMCtx), Argv, ArgvOff, "argv.elem.ptr");
    llvm::Value *ArgvElem = CGM->Builder->CreateLoad(
        llvm::PointerType::getUnqual(CGM->LLVMCtx), ArgvElemPtr, "argv.elem");

    // strlen(argv[i+1])
    llvm::FunctionCallee StrlenFn = CGM->Module->getOrInsertFunction(
        "strlen",
        llvm::FunctionType::get(CodeGen::IntPtrTy, {llvm::PointerType::getUnqual(CGM->LLVMCtx)}, false));
    llvm::Value *StrLen = CGM->Builder->CreateCall(StrlenFn, {ArgvElem}, "strlen");
    llvm::Value *StrLenInt = CGM->Builder->CreateIntCast(StrLen, CodeGen::IntTy, false, "strlen.int");

    // &data[i]
    llvm::Value *IdxExt = CGM->Builder->CreateSExt(Idx, CodeGen::IntPtrTy, "i.ext");
    llvm::Value *SlotPtr = CGM->Builder->CreateGEP(StringTy, DataPtr, IdxExt, "str.slot");

    // data[i].ptr = argv[i+1]
    llvm::Value *SlotPtrField = CGM->Builder->CreateStructGEP(StringTy, SlotPtr, 0, "slot.ptr");
    CGM->Builder->CreateStore(ArgvElem, SlotPtrField);

    // data[i].size = strlen
    llvm::Value *SlotSizeField = CGM->Builder->CreateStructGEP(StringTy, SlotPtr, 1, "slot.size");
    CGM->Builder->CreateStore(StrLenInt, SlotSizeField);

    // ++i
    llvm::Value *IdxNext = CGM->Builder->CreateAdd(Idx, One32, "i.next");
    Idx->addIncoming(IdxNext, BodyBB);
    CGM->Builder->CreateBr(CondBB);

    CGM->Builder->SetInsertPoint(ExitBB);
}
