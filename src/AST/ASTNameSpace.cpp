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
#include "AST/ASTNode.h"
#include "AST/ASTFunc.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTClass.h"

using namespace fly;

ASTNameSpace::ASTNameSpace(const llvm::StringRef &NS) : Name(NS) {}

const llvm::StringRef ASTNameSpace::DEFAULT = "default";

ASTNameSpace::~ASTNameSpace() {
    Nodes.clear();
    GlobalVars.clear();
}

const llvm::StringRef &fly::ASTNameSpace::getName() const {
    return Name;
}

const llvm::StringMap<ASTNode*> &ASTNameSpace::getNodes() const {
    return Nodes;
}

const llvm::StringMap<ASTGlobalVar *> &ASTNameSpace::getGlobalVars() const {
    return GlobalVars;
}

bool ASTNameSpace::addGlobalVar(ASTGlobalVar *Var) {
    auto Pair = std::make_pair(Var->getName(), Var);
    return GlobalVars.insert(Pair).second;
}

const std::unordered_set<ASTFunc*> &ASTNameSpace::getFunctions() const {
    return Functions;
}

bool ASTNameSpace::addFunction(ASTFunc *Func) {
    if (Functions.insert(Func).second) {
        return addResolvedCall(ASTFuncCall::CreateCall(Func));
    }

    return false;
}

const llvm::StringMap<std::vector<ASTFuncCall *>> &ASTNameSpace::getResolvedCalls() const {
    return ResolvedCalls;
}

bool ASTNameSpace::addResolvedCall(ASTFuncCall *Call) {
    const auto &It = ResolvedCalls.find(Call->getName());
    if (It == ResolvedCalls.end()) {
        std::vector<ASTFuncCall *> Functions;
        Functions.push_back(Call);
        return ResolvedCalls.insert(std::make_pair(Call->getName(), Functions)).second;
    }
    It->getValue().push_back(Call);
    return true;
}

const llvm::StringMap<ASTClass *> &ASTNameSpace::getClasses() const {
    return Classes;
}

bool ASTNameSpace::addClass(ASTClass *Class) {
    auto Pair = std::make_pair(Class->getName(), Class);
    return Classes.insert(Pair).second;
}

void ASTNameSpace::addUnRefCall(ASTFuncCall *Call) {
    UnRefCalls.push_back(Call);
}

void ASTNameSpace::addUnRefGlobalVar(ASTVarRef *Var) {
    UnRefGlobalVars.push_back(Var);
}

/**
 * Take all unreferenced Global Variables from Functions and try to resolve them
 * into this NameSpace
 * @return
 */
bool ASTNameSpace::Resolve() {
    bool Success = true;
    for (auto &Node : Nodes) {
        Success &= Node.getValue()->Resolve(UnRefGlobalVars,
                                         GlobalVars,
                                         UnRefCalls,
                                         ResolvedCalls);
    }
    return Success;
}
