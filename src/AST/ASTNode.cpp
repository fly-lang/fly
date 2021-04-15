//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTContext.cpp - AST Context implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
// This file implements the ASTContext interface.
//
//===--------------------------------------------------------------------------------------------------------------===//

#include <AST/ASTContext.h>
#include "AST/ASTNode.h"

using namespace fly;

ASTNode::ASTNode(const StringRef &fileName, const FileID &fid, ASTContext *Context) :
        ASTNodeBase(fileName, fid, Context) {
}

const ASTNameSpace* ASTNode::getNameSpace() {
    return NameSpace;
}

void ASTNode::setNameSpace(const StringRef &NS) {
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

bool ASTNode::addImport(StringRef Name) {
    // Check if Node already own this import
    ImportDecl* Import = Imports.lookup(Name);
    if (Import != nullptr) {
        // TODO Diag Error
        return false;
    }

    // Retrieve Import from Context
    Import = Context->Imports.lookup(Name);
    if (Import == nullptr) {
        Import = new ImportDecl(Name);
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
    if(Var->isExternal()) {
        GlobalVarDecl *LookupVar = NameSpace->Vars.lookup(Var->getName());
        if (LookupVar != nullptr) {
            // TODO Diag Error
            return false;
        }
        NameSpace->Vars.insert(Pair);
    } else {
        GlobalVarDecl *LookupVar = Vars.lookup(Var->getName());
        if (LookupVar != nullptr) {
            // TODO Diag Error
            return false;
        }
        Vars.insert(Pair);
    }
    return true;
}

bool ASTNode::Finalize() {
    return Context->AddNode(*this);
}
