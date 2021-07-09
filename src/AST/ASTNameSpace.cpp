//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTNameSpace.cpp - AST Namespace implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
// This file implements the ASTNameSpace interface.
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTNameSpace.h"
#include "AST/FuncDecl.h"
#include "AST/GlobalVarDecl.h"
#include "AST/ClassDecl.h"

using namespace fly;

ASTNameSpace::ASTNameSpace(const llvm::StringRef &NS) : NameSpace(NS) {}

const llvm::StringRef ASTNameSpace::DEFAULT = "default";

ASTNameSpace::~ASTNameSpace() {
    Nodes.clear();
    GlobalVars.clear();
}

const llvm::StringRef &fly::ASTNameSpace::getNameSpace() const {
    return NameSpace;
}

const llvm::StringMap<ASTNode*> &ASTNameSpace::getNodes() const {
    return Nodes;
}

const llvm::StringMap<GlobalVarDecl *> &ASTNameSpace::getGlobalVars() const {
    return GlobalVars;
}

bool ASTNameSpace::addGlobalVar(GlobalVarDecl *Var) {
    auto Pair = std::make_pair(Var->getName(), Var);
    return GlobalVars.insert(Pair).second;
}

const std::unordered_set<FuncDecl *, FuncDeclHash, FuncDeclComp> &ASTNameSpace::getFunctions() const {
    return Functions;
}

bool ASTNameSpace::addFunction(FuncDecl *Func) {
    if (Functions.insert(Func).second) {
        return addResolvedCall(FuncCall::CreateCall(Func));
    }
    return false;
}

const llvm::StringMap<std::vector<FuncCall *>> &ASTNameSpace::getResolvedCalls() const {
    return ResolvedCalls;
}

bool ASTNameSpace::addResolvedCall(FuncCall *Call) {
    const auto &It = ResolvedCalls.find(Call->getName());
    if (It == ResolvedCalls.end()) {
        std::vector<FuncCall *> Functions;
        Functions.push_back(Call);
        return ResolvedCalls.insert(std::make_pair(Call->getName(), Functions)).second;
    }
    It->getValue().push_back(Call);
    return true;
}

const llvm::StringMap<ClassDecl *> &ASTNameSpace::getClasses() const {
    return Classes;
}

bool ASTNameSpace::addClass(ClassDecl *Class) {
    auto Pair = std::make_pair(Class->getName(), Class);
    return Classes.insert(Pair).second;
}

bool ASTNameSpace::Finalize() {
    for (auto *Function : Functions) {
        Function->Finalize();
    }
    return false;
}
