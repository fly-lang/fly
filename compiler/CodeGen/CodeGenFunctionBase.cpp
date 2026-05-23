//===--------------------------------------------------------------------------------------------------------------===//
// compiler/CodeGen/CodeGenFunctionBase.cpp - function base code generation
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

#include "AST/ASTVar.h"
#include "Basic/SourceLocation.h"
#include "Basic/SourceManager.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"

#include <CodeGen/CodeGenVar.h>
#include <Sema/SemaError.h>
#include <Sema/SemaLocalVar.h>
#include <Sema/SemaParam.h>
#include <llvm/IR/Instructions.h>

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
    FLY_DEBUG_SCOPE("CodeGenFunctionBase", "setInsertPoint");
    Entry = llvm::BasicBlock::Create(CGM->LLVMCtx, "entry", Fn);
    CGM->Builder->SetInsertPoint(Entry);
}

void CodeGenFunctionBase::GenDebugSubprogram() {
    if (!CGM->DBuilder || !CGM->DebugFile)
        return;

    // Resolve actual source line number from AST location if SourceManager is available
    unsigned Line = 1;
    if (CGM->SM) {
        const SourceLocation &Loc = Sema->getAST().getLocation();
        if (Loc.isValid())
            Line = CGM->SM->getSpellingLineNumber(Loc);
    }

    // Build subroutine type from return type + parameter types
    llvm::SmallVector<llvm::Metadata *, 8> EltTys;
    EltTys.push_back(CGM->GetOrCreateDIType(Sema->getReturnType()));
    for (auto *P : Sema->getParams())
        EltTys.push_back(CGM->GetOrCreateDIType(P->getType()));
    llvm::DISubroutineType *FnTy =
        CGM->DBuilder->createSubroutineType(CGM->DBuilder->getOrCreateTypeArray(EltTys));

    llvm::DISubprogram *SP = CGM->DBuilder->createFunction(
        CGM->DebugCU,
        Fn->getName(),
        Fn->getName(),
        CGM->DebugFile,
        Line,
        FnTy,
        /*ScopeLine=*/Line,
        llvm::DINode::FlagZero,
        llvm::DISubprogram::SPFlagDefinition
    );
    Fn->setSubprogram(SP);
    CGM->Builder->SetCurrentDebugLocation(
        llvm::DILocation::get(CGM->LLVMCtx, Line, 0, SP));
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

        if (CGM->DBuilder && CGM->DebugFile) {
            llvm::BasicBlock *BB = CGM->Builder->GetInsertBlock();
            if (BB) {
                llvm::Function *FnDbg = BB->getParent();
                if (FnDbg && FnDbg->getSubprogram()) {
                    unsigned Line = 1;
                    if (CGM->SM) {
                        const SourceLocation &Loc = LocalVar->getAST()->getLocation();
                        if (Loc.isValid())
                            Line = CGM->SM->getSpellingLineNumber(Loc);
                    }
                    llvm::DIType *VarTy = CGM->GetOrCreateDIType(LocalVar->getType());
                    auto *DV = CGM->DBuilder->createAutoVariable(
                        FnDbg->getSubprogram(),
                        LocalVar->getAST()->getName().str(),
                        CGM->DebugFile, Line, VarTy ? VarTy : CGM->DBuilder->createUnspecifiedType("?"));
                    CGM->DBuilder->insertDeclare(
                        LocalVar->getCodeGen()->getPointer(), DV,
                        CGM->DBuilder->createExpression(),
                        llvm::DILocation::get(CGM->LLVMCtx, Line, 0, FnDbg->getSubprogram()),
                        BB);
                }
            }
        }
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

        if (CGM->DBuilder && CGM->DebugFile) {
            llvm::BasicBlock *BB = CGM->Builder->GetInsertBlock();
            if (BB) {
                llvm::DISubprogram *SP = Fn->getSubprogram();
                if (SP) {
                    unsigned Line = SP->getLine();
                    if (CGM->SM) {
                        const SourceLocation &Loc = Param->getAST()->getLocation();
                        if (Loc.isValid())
                            Line = CGM->SM->getSpellingLineNumber(Loc);
                    }
                    llvm::DIType *ParamTy = CGM->GetOrCreateDIType(Param->getType());
                    auto *DV = CGM->DBuilder->createParameterVariable(
                        SP,
                        Param->getAST()->getName().str(),
                        /*ArgNo=*/static_cast<unsigned>(Idx) + 1,
                        CGM->DebugFile, Line,
                        ParamTy ? ParamTy : CGM->DBuilder->createUnspecifiedType("?"));
                    CGM->DBuilder->insertDeclare(
                        CGV->getPointer(), DV,
                        CGM->DBuilder->createExpression(),
                        llvm::DILocation::get(CGM->LLVMCtx, Line, 0, SP),
                        BB);
                }
            }
        }

        ++Idx;
    }
}

void CodeGenFunctionBase::CheckReturnVoid() {
    FLY_DEBUG_SCOPE("CodeGenFunctionBase", "GenBody");

	// Only process functions that return void
	if (!Fn->getReturnType()->isVoidTy())
		return;

	// The Builder's current insert block may differ from Fn->back() when control
	// flow (e.g. while inside if) leaves the insert point pointing at a merge
	// block that is not the last block appended to the function.
	llvm::BasicBlock *InsertBB = CGM->Builder->GetInsertBlock();
	if (InsertBB && !InsertBB->getTerminator()) {
		CGM->Builder->CreateRetVoid();
	}

	// Also ensure the last appended block has a terminator.
	llvm::BasicBlock &LastBlock = Fn->back();
	if (!LastBlock.getTerminator()) {
		llvm::IRBuilder<> builder(&LastBlock);
		builder.CreateRetVoid();
	}
}

