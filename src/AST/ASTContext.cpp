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
#include "AST/ASTUnref.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"

using namespace fly;

/**
 * ASTContext constructor
 * @param Diags
 */
ASTContext::ASTContext(DiagnosticsEngine &Diags) : Diags(Diags) {
    DefaultNS = new ASTNameSpace(ASTNameSpace::DEFAULT, this);
    NameSpaces.insert(std::make_pair(ASTNameSpace::DEFAULT, DefaultNS));
}

/**
 * ASTContext destructor
 */
ASTContext::~ASTContext() {
    NameSpaces.clear();
    Imports.clear();
}

/**
 * Get the Default Namespace
 * @return DefaultNS
 */
ASTNameSpace *ASTContext::getDefaultNameSpace() const {
    return DefaultNS;
}

/**
 * Get all added namespaces in the context
 * @return a StringMap of ASTNameSpace
 */
const llvm::StringMap<ASTNameSpace *> &ASTContext::getNameSpaces() const {
    return NameSpaces;
}

/**
 * Write Diagnostics
 * @param Loc
 * @param DiagID
 * @return
 */
DiagnosticBuilder ASTContext::Diag(SourceLocation Loc, unsigned DiagID) const {
    return Diags.Report(Loc, DiagID);
}

/**
 * Add an ASTNode to the context
 * @param Node
 * @return true if no error occurs, otherwise false
 */
bool ASTContext::AddNode(ASTNode *Node) {
    assert(Node->NameSpace && "NameSpace is empty!");
    assert(!Node->Name.empty() && "FileName is empty!");
    FLY_DEBUG_MESSAGE("ASTContext", "AddNode", "Node=" << Node->str());
    llvm::StringMap<ASTNode *> &NSNodes = Node->NameSpace->Nodes;

    // Add to Nodes
    auto Pair = std::make_pair(Node->Name, Node);
    NSNodes.insert(Pair);

    // Try to link Node Imports to already resolved Nodes
    // Iterate over Node Imports
    for (auto &MapImport : Node->Imports) {
        FLY_DEBUG_MESSAGE("ASTContext", "AddNode", "Import=" << MapImport.getValue()->str());
        if (MapImport.getValue()->getNameSpace() == nullptr) {
            ASTNameSpace *NameSpace = NameSpaces.lookup(MapImport.getKey());
            if (NameSpace != nullptr) {
                MapImport.getValue()->setNameSpace(NameSpace);
            }
        }
    }

    return true;
}

/**
 * Remove an ASTNode
 * @param Node
 * @return true if no error occurs, otherwise false
 */
bool ASTContext::DelNode(ASTNode *Node) {
    Node->NameSpace->Nodes.erase(Node->getName());
    return true;
}

/**
 * Resolve AST
 * @return true if no error occurs, otherwise false
 */
bool ASTContext::Resolve() {
    bool Success = true;

    // add UnRefGlobalVars and UnRefCalls to respectively namespace
    // Resolve NameSpaces
    for (auto &NSEntry : NameSpaces) {
        Success &= ASTResolver::Resolve(NSEntry.getValue());
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
