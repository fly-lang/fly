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

const std::unordered_set<FuncDecl *> &ASTNameSpace::getFunctions() const {
    return Functions;
}

const std::unordered_set<FuncCall *, FuncCallHash, FuncCallComp> &ASTNameSpace::getCalls() const {
    return Calls;
}

bool ASTNameSpace::addCall(FuncCall *Call) {
    return Calls.insert(Call).second;
}

const llvm::StringMap<ClassDecl *> &ASTNameSpace::getClasses() const {
    return Classes;
}

bool ASTNameSpace::Finalize() {
    for (auto *Function : Functions) {
        Function->Finalize();
    }
    return false;
}
