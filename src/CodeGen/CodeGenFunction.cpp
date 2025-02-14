//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGFunction.cpp - Code Generator Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenFunction.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenError.h"
#include "AST/ASTModule.h"
#include "AST/ASTFunction.h"
#include "AST/ASTScopes.h"
#include "Sym/SymFunction.h"
#include "AST/ASTTypeRef.h"
#include "Basic/Debug.h"
#include "llvm/IR/Function.h"
#include "llvm/ADT/StringRef.h"

#include <AST/ASTVar.h>
#include <Sym/SymModule.h>
#include <Sym/SymNameSpace.h>
#include <Sym/SymType.h>

using namespace fly;

CodeGenFunction::CodeGenFunction(CodeGenModule *CGM, SymFunction *Sym, bool isExternal) :
    CodeGenFunctionBase(CGM, Sym), isExternal(isExternal), isMain(isMainFunction(Sym)) {

    // Generate Params Types
    if (isMain) {
        RetType = CGM->Int32Ty;
    } else {
        GenReturnType();

        // Add ErrorHandler as first param
        ParamTypes.push_back(CGM->ErrorPtrTy);
    }
    GenParamTypes(CGM, ParamTypes, Sym->getAST()->getParams());

    // Create LLVM Function
    FnType = llvm::FunctionType::get(RetType, ParamTypes, false);

    // Set Name
    std::string Id = CodeGen::toIdentifier(Sym->getAST()->getName(), Sym->getModule()->getNameSpace()->getName());
    Fn = llvm::Function::Create(FnType, llvm::GlobalValue::ExternalLinkage, Id, CGM->getModule());

    // Set Linkage
    if (isExternal && Sym->getVisibility() == SymVisibilityKind::PRIVATE) {
        Fn->setLinkage(GlobalValue::LinkageTypes::InternalLinkage);
    }
}

/**
 * Alloca Error Handler
 * Alloca Local Vars
 */
void CodeGenFunction::GenBody() {
    FLY_DEBUG_START("CodeGenFunction", "GenBody");
    setInsertPoint();

	// Alloca Function Error Handler
    AllocaErrorHandler();

	// Alloca Function Parameters and Local Vars
    AllocaLocalVars();

	// Store in Function Error Handler
    StoreErrorHandler(isMain);

	// Store in Function Parameters
    StoreParams(isMain);

	// Generate Function Body
    CGM->GenBlock(this, Sym->getAST()->getBody());

    // if is Main check error and return right exit code
    if (isMain) {
        llvm::Value *Zero32 = llvm::ConstantInt::get(CGM->Int32Ty, 0);
        llvm::Value *Zero8 = llvm::ConstantInt::get(CGM->Int8Ty, 0);
        // take return value from error struct
        CodeGenError *CGE = (CodeGenError *) Sym->getAST()->getErrorHandler()->getSym()->getCodeGen();
        ErrorHandler = CGE->getValue();
        llvm::Value *ErrorKind = CGM->Builder->CreateInBoundsGEP(CGE->getType(), ErrorHandler, {Zero32, Zero32});
        llvm::Value *Ret = CGM->Builder->CreateICmpNE(CGM->Builder->CreateLoad(ErrorKind), Zero8);
        // main() will return 0 if ok or 1 on error
        CGM->Builder->CreateRet(CGM->Builder->CreateZExt(Ret, Fn->getReturnType()));
    } else if (Sym->getAST()->getReturnTypeRef()->getType()->isVoid()) {
        CGM->Builder->CreateRetVoid();
    }
}

bool CodeGenFunction::isMainFunction(SymFunction *Sym) {
    return Sym->getAST()->getName() == StringRef("main") && Sym->getAST()->getReturnTypeRef()->getType()->isVoid();
}
