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

ASTNodeBase::ASTNodeBase(const std::string &Name, ASTContext *Context) :
        Name(Name), Context(Context) {
}

ASTContext &ASTNodeBase::getContext() const {
    return *Context;
}

const std::string &ASTNodeBase::getName() {
    return Name;
}

const llvm::StringMap<ASTGlobalVar *> &ASTNodeBase::getGlobalVars() const {
    return GlobalVars;
}

const std::unordered_set<ASTFunction*> &ASTNodeBase::getFunctions() const {
    return Functions;
}

const llvm::StringMap<ASTClass *> &ASTNodeBase::getClasses() const {
    return Classes;
}

std::string ASTNodeBase::str() const {
    return "{ Name=" + Name + " }";
}
