//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGFunction.cpp - Code Generator Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenFunctionBase.h"

#include "AST/ASTFunction.h"
#include "Basic/Debug.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenError.h"
#include "CodeGen/CodeGenModule.h"
#include "Sema/SemaClassAttribute.h"
#include "Sema/SemaClassMethod.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"

#include <CodeGen/CodeGenVar.h>
#include <Sema/SemaErrorHandler.h>
#include <Sema/SemaLocalVar.h>
#include <Sema/SemaParam.h>
#include <llvm/IR/Instructions.h>
#include <unordered_map>

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

llvm::AllocaInst *CodeGenFunctionBase::AllocaVar(llvm::Type *Ty) {
	if (Ty->isStructTy()) {
		llvm::PointerType *PtrTy = Ty->getPointerTo(CGM->Module->getDataLayout().getAllocaAddrSpace());
		return CGM->Builder->CreateAlloca(PtrTy);
	} else {
		// Alloca for non-struct types
		// Check if the type is bool (i1) and convert it to i8
		return  CGM->Builder->CreateAlloca(Ty->isIntegerTy(1) ? CGM->Int8Ty : Ty);
	}
}

void CodeGenFunctionBase::AllocaLocalVars() {
    // Allocation of declared ASTVar
    for (auto Param : Sema->getParams()) {
    	llvm::Type *Ty = CGM->GenType(Param->getType());
    	llvm::AllocaInst * Alloca = AllocaVar(Ty);
    	CodeGenVar *CGV = new CodeGenVar(CGM, Param, Ty);
    	CGV->setPointer(Alloca);
        Param->setCodeGen(CGV);
    }

    // Allocation of all declared ASTVar
    for (auto &LocalVar: Sema->getLocalVars()) {
    	if (LocalVar->getType()->isError()) {
    		CodeGenError *CGE = CGM->GenErrorHandler(LocalVar);
    		LocalVar->setCodeGen(CGE);
        } else {
        	llvm::Type *Ty = CGM->GenType(LocalVar->getType());
        	llvm::AllocaInst *Alloca = AllocaVar(Ty);
        	CodeGenVar *CGV = new CodeGenVar(CGM, LocalVar, Ty);
        	CGV->setPointer(Alloca);
        	LocalVar->setCodeGen(CGV);
        }
    }
}

void CodeGenFunctionBase::StoreParams(size_t Idx) {
    // Store Param Values (n = 0 is the Error param)
    for (auto &Param: Sema->getParams()) {

        // Store Arg value into Param
        CodeGenVarBase *CGV = Param->getCodeGen();
        CGV->Store(Fn->getArg(Idx));

        ++Idx;
    }
}

void CodeGenFunctionBase::CheckReturnVoid() {
    FLY_DEBUG_START("CodeGenFunctionBase", "GenBody");

	// Only process functions that return void
	if (!Fn->getReturnType()->isVoidTy())
		return;

	llvm::BasicBlock &lastBlock = Fn->back(); // Get last basic block

	if (lastBlock.empty()) {
		// If block is empty, just insert ret void
		llvm::IRBuilder<> builder(&lastBlock);
		builder.CreateRetVoid();
		return;
	}

	llvm::Instruction *lastInst = lastBlock.getTerminator();

	if (!lastInst) {
		// No terminator — add ret void
		llvm::IRBuilder<> builder(&lastBlock);
		builder.CreateRetVoid();
		return;
	}

    FLY_DEBUG_END("CodeGenFunctionBase", "GenBody");
}

llvm::BasicBlock * CodeGenFunctionBase::getSafeBB() {
	return SafeBB;
}

void CodeGenFunctionBase::setSafeBB(llvm::BasicBlock *BB) {
	this->SafeBB = BB;
}

// Mapping Fly types to mangled representations
std::unordered_map<std::string, std::string> MangleTypeMap = {
	{"bool", "_b"},
	{"byte", "_y"},
	{"ushort", "_us"},
	{"short", "_s"},
	{"uint", "_ui"},
	{"int", "_i"},
	{"ulong", "_ul"},
	{"long", "_l"},
	{"float", "_f"},
	{"double", "_d"},
	{"void", "_v"},
	{"string", "_Ss"},
	// {"char", "_c"},
	{"error", "_e"},
};

// Function to process array type: "int[5]" -> "A5_i"
std::string CodeGenFunctionBase::Mangle(SemaType *Type) {
	std::string Mangled = "";

	switch (Type->getTypeKind()) {
		case SemaTypeKind::TYPE_ARRAY: {
			SemaArrayType *Array = static_cast<SemaArrayType *>(Type);
			Mangled += "_A" + Mangle(Array->getType());
		}	break;
		case SemaTypeKind::TYPE_ENUM:
			Mangled += "_E" + Type->getName();
			break;
		case SemaTypeKind::TYPE_CLASS:
			Mangled += "_C" + Type->getName();
			break;
		default:
			Mangled += MangleTypeMap.at(Type->getName());
	}

	return Mangled;
}

std::string CodeGenFunctionBase::Mangle(SemaFunctionBase *F) {
	std::string Name = std::string(F->getName()); // Function name

	// Mangling Function with _F prefix
	// Encode function name with its length
	std::string Mangled = "_F" + std::to_string(Name.size()) + Name;

	// Encode parameters
	for (const auto Param : F->getParams()) {
		Mangled += Mangle(Param->getType());
	}

	return Mangled;
}
