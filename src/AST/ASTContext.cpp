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

#include "AST/ASTContext.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTNode.h"
#include "AST/ASTImport.h"
#include "Basic/Diagnostic.h"

using namespace fly;

ASTContext::ASTContext(DiagnosticsEngine &Diags) : Diags(Diags) {
    DefaultNS = new ASTNameSpace(ASTNameSpace::DEFAULT);
    NameSpaces.insert(std::make_pair(ASTNameSpace::DEFAULT, DefaultNS));
}

ASTContext::~ASTContext() {
    NameSpaces.clear();
    Imports.clear();
}

bool ASTContext::AddNode(ASTNode *Node) {
    assert(Node->NameSpace && "NameSpace is empty!");
    assert(!Node->FileName.empty() && "FileName is empty!");
    llvm::StringMap<ASTNode *> &NSNodes = Node->NameSpace->Nodes;

    // Add to Nodes
    auto Pair = std::make_pair(Node->FileName, Node);
    NSNodes.insert(Pair);

    // Try to link Node Imports to already resolved Nodes
    // Iterate over Node Imports
    for (auto &MapImport : Node->Imports) {
        if (MapImport.getValue()->getNameSpace() == nullptr) {
            ASTNameSpace *NameSpace = NameSpaces.lookup(MapImport.getKey());
            if (NameSpace != nullptr) {
                MapImport.getValue()->setNameSpace(NameSpace);
            }
        }
    }

    return true;
}

bool ASTContext::DelNode(ASTNode *Node) {
    Node->NameSpace->Nodes.erase(Node->getFileName());
    return true;
}
/**
 * Take all unreferenced Global Variables from Functions and try to resolve them
 * into this NameSpace
 * @return
 */
bool ASTContext::Resolve() {
    bool Success = true;

    // add UnRefGlobalVars and UnRefCalls to respectively namespace
    // Resolve NameSpaces
    for (auto &NSEntry : NameSpaces) {
        ASTNameSpace *&NS = NSEntry.getValue();
        for (auto &UnRefGlobalVar : UnRefGlobalVars) {
            if (UnRefGlobalVar->getNameSpace() == NS->getName()) {
                NS->addUnRefGlobalVar(UnRefGlobalVar);
            }
        }
        for (auto UnRefCall : UnRefCalls) {
            if (UnRefCall->getNameSpace() == NS->getName()) {
                NS->addUnRefCall(UnRefCall);
            }
        }
        NS->Resolve();
    }

    // Now all Imports must be read
    for(auto &Import : Imports) {
        if (Import.getValue()->getNameSpace() == nullptr) {
            Diag(Import.getValue()->getLocation(), diag::err_unresolved_import);
            return false;
        }
    }
    return Success;
}

const llvm::StringMap<ASTNameSpace *> &ASTContext::getNameSpaces() const {
    return NameSpaces;
}

void ASTContext::addUnRefCall(ASTFuncCall *Call) {
    UnRefCalls.push_back(Call);
}

void ASTContext::addUnRefGlobalVar(ASTVarRef *Var) {
    UnRefGlobalVars.push_back(Var);
}

DiagnosticBuilder ASTContext::Diag(SourceLocation Loc, unsigned DiagID) const {
    return Diags.Report(Loc, DiagID);
}

ASTNameSpace *ASTContext::getDefaultNameSpace() const {
    return DefaultNS;
}
