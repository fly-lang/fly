//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTNode.cpp - AST Node implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
// This file implements the ASTNode interface.
//
//===--------------------------------------------------------------------------------------------------------------===//

#include <AST/ASTContext.h>
#include "AST/ASTNode.h"

using namespace fly;

ASTNode::ASTNode(const StringRef fileName, const FileID &fid, ASTContext *Context) :
        ASTNodeBase(fileName, fid, Context) {
}

const ASTNameSpace* ASTNode::getNameSpace() {
    return NameSpace;
}

void ASTNode::setNameSpace() {
    StringRef NS = "default";
    setNameSpace(NS);
}

void ASTNode::setNameSpace(StringRef NS) {
    // Check if NS exist or add
    NameSpace = Context->NameSpaces.lookup(NS);
    if (NameSpace == nullptr) {
        NameSpace = new ASTNameSpace(NS);
        Context->NameSpaces.insert(std::make_pair(NS, NameSpace));
    }
}

const llvm::StringMap<ImportDecl*> &ASTNode::getImports() {
    return Imports;
}

bool ASTNode::addImport(const SourceLocation &Loc, StringRef Name, StringRef Alias) {
    // Check if this Node already own this Import
    ImportDecl* Import = Imports.lookup(Name);
    if (Import != nullptr) {
        Context->Diag(Loc, diag::err_duplicate_import)  << Name;
        return false;
    }

    // Retrieve Import from Context if already exists in order to maintain only one instance of ImportDecl
    Import = Context->Imports.lookup(Name);
    if (Import == nullptr) {
        Import = new ImportDecl(Loc, Name, Alias);
    }
    auto Pair = std::make_pair(Name, Import);

    // Add Import to Context
    Context->Imports.insert(Pair);

    // Add Import to Node
    Imports.insert(Pair);

    return true;
}

const llvm::StringMap<GlobalVarDecl *> &ASTNode::getVars() {
    return Vars;
}

bool ASTNode::addVar(GlobalVarDecl *Var) {
    auto Pair = std::make_pair(Var->getName(), Var);
    if(Var->Visibility == VisibilityKind::Public || Var->Visibility == VisibilityKind::Default) {
        GlobalVarDecl *LookupVar = NameSpace->Vars.lookup(Var->getName());
        if (LookupVar) {
            Context->Diag(LookupVar->getLocation(), diag::err_duplicate_gvar)  << LookupVar->getName();
            return false;
        }
        NameSpace->Vars.insert(Pair);
    }
    GlobalVarDecl *LookupVar = Vars.lookup(Var->getName());
    if (LookupVar) {
        Context->Diag(LookupVar->getLocation(), diag::err_duplicate_gvar)  << LookupVar->getName();
        return false;
    }
    Vars.insert(Pair);
    return true;
}

bool ASTNode::Finalize() {
    return Context->AddNode(this);
}

bool ASTNode::isFirstNode() const {
    return FirstNode;
}

void ASTNode::setFirstNode(bool First) {
    ASTNode::FirstNode = First;
}

GlobalVarDecl *ASTNode::addIntVar(const SourceLocation &Loc, VisibilityKind Visibility, ModifiableKind Modifiable,
                                  StringRef Name, int *Val) {
    GlobalVarDecl *Var = new GlobalVarDecl(Loc, ModifiableKind::Variable, new IntTypeDecl(Val), Name);
    Var->setVisibility(Visibility);
    addVar(Var);
    return Var;
}

GlobalVarDecl *ASTNode::addFloatVar(const SourceLocation &Loc, VisibilityKind Visibility, ModifiableKind Modifiable,
                                    StringRef Name, float *Val) {
    GlobalVarDecl *Var = new GlobalVarDecl(Loc, ModifiableKind::Variable, new FloatTypeDecl(Val), Name);
    Var->setVisibility(Visibility);
    addVar(Var);
    return Var;
}

GlobalVarDecl *ASTNode::addBoolVar(const SourceLocation &Loc, VisibilityKind Visibility, ModifiableKind Modifiable,
                                   StringRef Name, bool *Val) {
    GlobalVarDecl *Var = new GlobalVarDecl(Loc, Modifiable, new BoolTypeDecl(Val), Name);
    Var->setVisibility(Visibility);
    addVar(Var);
    return Var;
}

ASTNode::~ASTNode() {
    Vars.clear();
    Imports.clear();
}
