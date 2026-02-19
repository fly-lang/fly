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
#include <Sema/SemaError.h>
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
	if (!Sema->getReturnType()) {
		CGM->Diag(diag::err_codegen_invalid_type);
		return;
	}
	Sema->getReturnType()->accept(*CGM);
	if (!Sema->getReturnType()->getCodeGen()) {
		CGM->Diag(diag::err_codegen_invalid_type);
		return;
	}
    RetType = Sema->getReturnType()->getCodeGen()->getType();
}

void CodeGenFunctionBase::GenParamTypes(CodeGenModule * CGM, llvm::SmallVector<llvm::Type *, 8> &Types, SemaFunctionBase * Sema) {
    // Populate Types by reference
    // All parameters are passed by reference (as pointers), including primitive types
    for (auto Param : Sema->getParams()) {
    	if (!Param->getType()) {
    		CGM->Diag(diag::err_codegen_invalid_type);
    		continue;
    	}
    	Param->getType()->accept(*CGM);
    	if (!Param->getType()->getCodeGen()) {
    		CGM->Diag(diag::err_codegen_invalid_type);
    		continue;
    	}
        // All parameters are passed by reference (pointer type)
        llvm::Type *ParamType = Param->getType()->getCodeGen()->getType();
        Types.push_back(ParamType->getPointerTo());
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

void CodeGenFunctionBase::AllocaLocalVars() {
    // Parameters are passed by reference (as pointers), so we don't allocate them
    // We just set up the CodeGenVar to use the passed pointer directly
    for (auto Param : Sema->getParams()) {
    	Param->accept(*CGM);
    	// For parameters passed by reference, we use the pointer directly
    	// No allocation needed - the pointer is the parameter itself
    }

    // Allocation of all declared SemaLocalVar
    for (auto &LocalVar: Sema->getLocalVars()) {
    	LocalVar->accept(*CGM);
    	LocalVar->getCodeGen()->Alloca();
    }
}

void CodeGenFunctionBase::StoreParams(size_t Idx) {
    // Parameters are passed by reference (as pointers)
    // We store the pointer directly in the CodeGenVar
    for (auto &Param: Sema->getParams()) {

        // Get the pointer argument (already a pointer to the value)
        CodeGenVar *CGV = Param->getCodeGen();

        // Store the pointer as the variable's address
        // The argument is already a pointer, so we use it directly
        CGV->setPointer(Fn->getArg(Idx));

        // Mark const parameters as read-only at LLVM level
        if (Param->isConstant()) {
            CGV->setReadOnly(true);
            // Add LLVM readonly attribute to the parameter
            // This tells LLVM that this pointer parameter is not written to
            Fn->addParamAttr(Idx, llvm::Attribute::ReadOnly);
        }

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

	switch (Type->getKind()) {
		case SemaKind::TYPE_ARRAY: {
			SemaArrayType *Array = static_cast<SemaArrayType *>(Type);
			Mangled += "_A" + Mangle(Array->getElementType());
		}	break;
		case SemaKind::TYPE_ENUM:
			Mangled += "_E" + Type->getName();
			break;
		case SemaKind::TYPE_CLASS:
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
