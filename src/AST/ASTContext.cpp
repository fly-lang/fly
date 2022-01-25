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
    DefaultNS = AddNameSpace(ASTNameSpace::DEFAULT);
}

/**
 * ASTContext destructor
 */
ASTContext::~ASTContext() {
    NameSpaces.clear();
    ExternalImports.clear();
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
 * Write Diagnostics
 * @param Loc
 * @param DiagID
 * @return
 */
DiagnosticBuilder ASTContext::Diag(unsigned DiagID) const {
    return Diags.Report(DiagID);
}

ASTNameSpace *ASTContext::AddNameSpace(std::string Name, bool ExternLib) {
    // Check if Name exist or add it
    ASTNameSpace *NameSpace = NameSpaces.lookup(Name);
    if (NameSpace == nullptr) {
        NameSpace = new ASTNameSpace(Name, this, ExternLib);
        NameSpaces.insert(std::make_pair(Name, NameSpace));
    }
    return NameSpace;
}

/**
 * Add an ASTNode to the context
 * @param Node
 * @return true if no error occurs, otherwise false
 */
bool ASTContext::AddNode(ASTNode *Node) {
    assert(Node->getNameSpace() && "NameSpace is empty!");
    assert(!Node->getName().empty() && "FileName is empty!");
    FLY_DEBUG_MESSAGE("ASTContext", "AddNode", "Node=" << Node->str());
    llvm::StringMap<ASTNode *> &NSNodes = Node->getNameSpace()->Nodes;

    // Add to Nodes
    auto Pair = std::make_pair(Node->getName(), Node);
    NSNodes.insert(Pair);

    if (!Node->isHeader()) {
        // Try to link Node Imports to already resolved Nodes
        // Iterate over Node Imports
        for (auto &MapImport: Node->getImports()) {
            FLY_DEBUG_MESSAGE("ASTContext", "AddNode", "Import=" << MapImport.getValue()->str());
            if (MapImport.getValue()->getNameSpace() == nullptr) {
                ASTNameSpace *NameSpace = NameSpaces.lookup(MapImport.getKey());
                if (NameSpace != nullptr) {
                    MapImport.getValue()->setNameSpace(NameSpace);
                }
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
    for(auto &Import : ExternalImports) {
        if (Import.getValue()->getNameSpace() == nullptr) {
            Diag(Import.getValue()->getLocation(), diag::err_unresolved_import);
            return false;
        }
    }
    return Success;
}
