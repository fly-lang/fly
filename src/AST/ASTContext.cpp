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
    assert(Node->getFileID().isValid() && "ASTNode FileID is not valid!");
    assert(Node->NameSpace && "NameSpace is empty!");
    assert(!Node->FileName.empty() && "FileName is empty!");
    llvm::StringMap<ASTNode *> &NSNodes = Node->NameSpace->Nodes;

    // Set FirstNode
    bool isFirstAddition = false;
    if (!FirstNode) {
        FirstNode = Node;
        isFirstAddition = true;
        Node->setFirstNode(true);
    }

    // Add to Nodes
    auto Pair = std::make_pair(Node->FileName, Node);
    NSNodes.insert(Pair);

    // Search only if this node is the first one because Nodes are empty
    if (!isFirstAddition) {
        // Try to link Node Imports to already resolved Nodes
        // Iterate over Node Imports
        for (auto &Import : Node->Imports) {
            if (Import.getValue()->getNameSpace() == nullptr) {
                ASTNameSpace *NameSpace = Node->Context->NameSpaces.lookup(Import.getKey());
                if (NameSpace != nullptr) {
                    Import.getValue()->setNameSpace(NameSpace);
                }
            }
        }
    }

    return true;
}

bool ASTContext::DelNode(ASTNode *Node) {
    assert(Node->getFileID().isValid() && "ASTNode FileID is not valid!");
    Node->NameSpace->Nodes.erase(Node->getFileName());
    return true;
}

bool ASTContext::Finalize() {
    // Close the chain by resolving nodes of first one
    bool Success = FirstNode->Finalize();

    // Finalize NameSpaces
    for (auto &NSEntry : NameSpaces) {
        ASTNameSpace *&NS = NSEntry.getValue();
        NS->Finalize();
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

DiagnosticBuilder ASTContext::Diag(SourceLocation Loc, unsigned DiagID) const {
    return Diags.Report(Loc, DiagID);
}

ASTNameSpace *ASTContext::getDefaultNameSpace() const {
    return DefaultNS;
}

bool ASTContext::hasErrors() const {
    return Diags.hasErrorOccurred();
}
