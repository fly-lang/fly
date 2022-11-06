//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTNodeBase.cpp - Base AST Node implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTNodeBase.h"
#include "AST/ASTFunction.h"
#include "AST/ASTFunctionCall.h"

using namespace fly;

ASTNodeBase::ASTNodeBase(const std::string Name, ASTContext *Context) :
        Name(Name), Context(Context) {
}

ASTContext &ASTNodeBase::getContext() const {
    return *Context;
}

const std::string ASTNodeBase::getName() {
    return Name;
}

const llvm::StringMap<ASTGlobalVar *> &ASTNodeBase::getGlobalVars() const {
    return GlobalVars;
}

const llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> &ASTNodeBase::getFunctions() const {
    return Functions;
}