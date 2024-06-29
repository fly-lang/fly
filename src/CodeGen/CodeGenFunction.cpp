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
#include "AST/ASTNameSpace.h"
#include "AST/ASTModule.h"
#include "AST/ASTFunction.h"
#include "AST/ASTScopes.h"
#include "AST/ASTType.h"
#include "Basic/Debug.h"
#include "llvm/IR/Function.h"
#include "llvm/ADT/StringRef.h"

using namespace fly;

CodeGenFunction::CodeGenFunction(CodeGenModule *CGM, ASTFunction *AST, bool isExternal) :
    CodeGenFunctionBase(CGM, AST), AST(AST), isExternal(isExternal) {

    // Generate Params Types
    if (isMainFunction(AST)) {
        RetType = CGM->Int32Ty;
    } else {
        GenReturnType();

        // Add ErrorHandler as first param
        ParamTypes.push_back(CGM->ErrorPtrTy);
    }
    GenParamTypes(CGM, ParamTypes, AST->getParams());

    // Create LLVM Function
    FnType = llvm::FunctionType::get(RetType, ParamTypes, AST->getEllipsis() != nullptr);

    // Set Name
    std::string Id = CodeGen::toIdentifier(AST->getName(), AST->getNameSpace()->getName());
    Fn = llvm::Function::Create(FnType, llvm::GlobalValue::ExternalLinkage, Id, CGM->getModule());

    // Set Linkage
    if (isExternal && AST->getScopes()->getVisibility() == ASTVisibilityKind::V_PRIVATE) {
        Fn->setLinkage(GlobalValue::LinkageTypes::InternalLinkage);
    }
}

void CodeGenFunction::GenBody() {
    FLY_DEBUG("CodeGenFunction", "GenBody");
    setInsertPoint();

    bool isMain = isMainFunction(AST);
    AllocaErrorHandler();
    AllocaLocalVars();
    StoreErrorHandler(isMain);
    StoreParams(isMain);
    CGM->GenBlock(this, AST->getBody());

    // if is Main check error and return right exit code
    if (isMain) {
        llvm::Value *Zero32 = llvm::ConstantInt::get(CGM->Int32Ty, 0);
        llvm::Value *Zero8 = llvm::ConstantInt::get(CGM->Int8Ty, 0);
        // take return value from error struct
        CodeGenError *CGE = (CodeGenError *) AST->getErrorHandler()->getCodeGen();
        llvm::Value *ErrorKind = CGM->Builder->CreateInBoundsGEP(CGE->getType(), ErrorHandler, {Zero32, Zero32});
        llvm::Value *Ret = CGM->Builder->CreateICmpNE(CGM->Builder->CreateLoad(ErrorKind), Zero8);
        // main() will return 0 if ok or 1 on error
        CGM->Builder->CreateRet(CGM->Builder->CreateZExt(Ret, Fn->getReturnType()));
    } else if (AST->getReturnType()->isVoid()) {
        CGM->Builder->CreateRetVoid();
    }
}

bool CodeGenFunction::isMainFunction(ASTFunctionBase *FunctionBase) {
    return FunctionBase->getKind() == ASTFunctionKind::FUNCTION && ((ASTFunction *) FunctionBase)->getName() == StringRef("main")
        && FunctionBase->getReturnType()->isVoid();
}
