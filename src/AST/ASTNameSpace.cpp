//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTNameSpace.cpp - AST Namespace implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTNameSpace.h"

using namespace fly;

ASTNameSpace::ASTNameSpace(const SourceLocation &Loc, llvm::StringRef Name, ASTContext *Context, bool ExternalLib) :
    ASTIdentifier(Loc, Name), Context(Context), ExternalLib(ExternalLib) {
}

ASTNameSpace::~ASTNameSpace() {
    Modules.clear();
    GlobalVars.clear();
}

const llvm::StringMap<ASTModule*> &ASTNameSpace::getModules() const {
    return Modules;
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

std::string ASTNameSpace::str() const {
    return Logger("ASTNameSpace").
           Attr("Name", Name).
           End();
}

const llvm::StringMap<ASTIdentityType *> &ASTNameSpace::getIdentityTypes() const {
    return IdentityTypes;
}
