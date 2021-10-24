//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTNodeBase.cpp - Base AST Node implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTNodeBase.h"
#include "AST/ASTFunc.h"
#include "Basic/Debug.h"

using namespace fly;

ASTNodeBase::ASTNodeBase(const llvm::StringRef &Name, ASTContext *Context) :
        Name(Name), Context(Context) {
}

ASTContext &ASTNodeBase::getContext() const {
    return *Context;
}

const llvm::StringRef &ASTNodeBase::getName() {
    return Name;
}

const llvm::StringMap<ASTGlobalVar *> &ASTNodeBase::getGlobalVars() const {
    return GlobalVars;
}

const std::unordered_set<ASTFunc*> &ASTNodeBase::getFunctions() const {
    return Functions;
}

const llvm::StringMap<std::vector<ASTFuncCall *>> &ASTNodeBase::getFunctionCalls() const {
    return FunctionCalls;
}

const llvm::StringMap<ASTClass *> &ASTNodeBase::getClasses() const {
    return Classes;
}

bool ASTNodeBase::AddFunctionCall(ASTFuncCall *Call) {
    const auto &It = FunctionCalls.find(Call->getName());
    FLY_DEBUG_MESSAGE("ASTNodeBase", "AddFunctionCall", "Call=" << Call->str());
    if (It == FunctionCalls.end()) {
        std::vector<ASTFuncCall *> TmpFunctionCalls;
        TmpFunctionCalls.push_back(Call);
        return FunctionCalls.insert(std::make_pair(Call->getName(), TmpFunctionCalls)).second;
    }
    It->getValue().push_back(Call);
    return true;
}

std::string ASTNodeBase::str() const {
    return "{ Name=" + Name.str() + " }";
}
