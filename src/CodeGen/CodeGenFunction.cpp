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
#include "AST/ASTNameSpace.h"
#include "AST/ASTFunction.h"
#include "AST/ASTType.h"
#include "Basic/Debug.h"
#include "llvm/IR/Function.h"
#include "llvm/ADT/StringRef.h"

using namespace fly;

CodeGenFunction::CodeGenFunction(CodeGenModule *CGM, ASTFunction *AST, bool isExternal) :
    CodeGenFunctionBase(CGM, AST), AST(AST), isExternal(isExternal) {

    // Generate Params Types
    llvm::SmallVector<llvm::Type *, 8> ParamTypes;
    if (isMainFunction(AST)) {
        RetType = CGM->Int32Ty;
    } else {
        GenReturnType();
        ParamTypes.push_back(CGM->ErrorType->getPointerTo(0));
    }
    GenParamTypes(CGM, ParamTypes, AST->getParams());

    // Create LLVM Function
    FnType = llvm::FunctionType::get(RetType, ParamTypes, AST->getParams()->getEllipsis() != nullptr);
    Fn = llvm::Function::Create(FnType, llvm::GlobalValue::ExternalLinkage, "", CGM->getModule());

    // Set Name
    std::string Id = CodeGen::toIdentifier(AST->getName(), AST->getNameSpace()->getName());
    Fn->setName(Id);

    // Set Linkage
    if (isExternal && AST->getScopes()->getVisibility() == ASTVisibilityKind::V_PRIVATE) {
        Fn->setLinkage(GlobalValue::LinkageTypes::InternalLinkage);
    }
}

void CodeGenFunction::GenBody() {
    FLY_DEBUG("CodeGenFunction", "GenBody");
    setInsertPoint();

    // Allocate Error if is Main Function or take from params
    bool isMain = isMainFunction(AST);
    if (isMain) {
        ErrorVar = CGM->Builder->CreateAlloca(CGM->ErrorType);
    } else {
        ErrorVar = Fn->getArg(0);
    }

    AllocaVars();
    StoreParams(isMain);
    CGM->GenBlock(this, AST->getBody()->getContent());

    // if is Main check error and return right exit code
    if (isMain) {
        llvm::Value *Zero32 = llvm::ConstantInt::get(CGM->Int32Ty, 0);
        llvm::Value *Zero8 = llvm::ConstantInt::get(CGM->Int8Ty, 0);
        // take return value from error struct
        llvm::Value *ErrorKind = CGM->Builder->CreateInBoundsGEP(CGM->ErrorType, ErrorVar, {Zero32, Zero32});
        llvm::Value *Ret = CGM->Builder->CreateICmpNE(CGM->Builder->CreateLoad(ErrorKind), Zero8);
        // main() will return 0 if ok or 1 on error
        CGM->Builder->CreateRet(CGM->Builder->CreateZExt(Ret, Fn->getReturnType()));
    } else if (AST->getType()->isVoid()) {
        CGM->Builder->CreateRetVoid();
    }
}

bool CodeGenFunction::isMainFunction(ASTFunctionBase *FunctionBase) {
    return FunctionBase->getKind() == ASTFunctionKind::FUNCTION && FunctionBase->getName() == StringRef("main")
        && FunctionBase->getType()->isVoid();
}
