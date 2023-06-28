//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGFunction.cpp - Code Generator Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenFunction.h"
#include "CodeGen/CodeGenVar.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTFunction.h"
#include "AST/ASTParams.h"
#include "llvm/IR/Function.h"
#include "llvm/ADT/StringRef.h"

using namespace fly;

CodeGenFunction::CodeGenFunction(CodeGenModule *CGM, ASTFunction *AST, bool isExternal) :
    CodeGenFunctionBase(CGM, AST), AST(AST), isExternal(isExternal) {
    llvm::Type *RetType = CGM->GenType(AST->getType());
    llvm::SmallVector<llvm::Type *, 8> ParamTypes;
    GenTypes(CGM, ParamTypes, AST->getParams());
    FnTy = llvm::FunctionType::get(RetType, ParamTypes, AST->getParams()->getEllipsis() != nullptr);
    Fn = llvm::Function::Create(FnTy, llvm::GlobalValue::ExternalLinkage, "", CGM->getModule());

    // Set Name
    std::string Id = CodeGen::toIdentifier(AST->getName(), AST->getNameSpace()->getName());
    Fn->setName(Id);

    // Set Linkage
    if (isExternal && AST->getScopes()->getVisibility() == ASTVisibilityKind::V_PRIVATE) {
        Fn->setLinkage(GlobalValue::LinkageTypes::InternalLinkage);
    }
}
