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
#include "AST/ASTContext.h"
#include "AST/ASTNode.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTClass.h"

using namespace fly;

const std::string ASTNameSpace::DEFAULT = "default";

ASTNameSpace::ASTNameSpace(const SourceLocation &Loc, llvm::StringRef Name, ASTContext *Context, bool ExternalLib) :
    ASTIdentifier(Loc, Name), Context(Context), ExternalLib(ExternalLib) {
}

ASTNameSpace::~ASTNameSpace() {
    Nodes.clear();
    GlobalVars.clear();
}

const llvm::StringMap<ASTNode*> &ASTNameSpace::getNodes() const {
    return Nodes;
}

bool ASTNameSpace::isExternalLib() const {
    return ExternalLib;
}

const llvm::StringMap<ASTIdentity *> &ASTNameSpace::getIdentities() const {
    return Identities;
}

const llvm::StringMap<ASTGlobalVar *> &ASTNameSpace::getGlobalVars() const {
    return GlobalVars;
}

const llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> &ASTNameSpace::getFunctions() const {
    return Functions;
}

std::string ASTNameSpace::print() const {
    return FullName;
}

std::string ASTNameSpace::str() const {
    return Logger("ASTNameSpace").
           Attr("Name", Name).
           End();
}
