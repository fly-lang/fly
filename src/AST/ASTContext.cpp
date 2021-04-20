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

using namespace fly;

bool ASTContext::AddNode(ASTNode *Node) {
    assert(Node->getFileID().isValid() && "ASTNode FileID is not valid!");
    assert(Node->NameSpace && "NameSpace is empty!");
    llvm::StringMap<ASTNode *> &NSNodes = Node->NameSpace->Nodes;

    // Set FirstNode
    bool isFirstAddition = false;
    if (FirstNode == nullptr) {
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

    // Now all Imports must be read
    for(auto &Import : Imports) {
        if (Import.getValue()->getNameSpace() == nullptr) {
            // TODO Log Error Unresolved Import
            return false;
        }
    }
    return Success;
}

const StringMap<ASTNameSpace *> &ASTContext::getNameSpaces() const {
    return NameSpaces;
}
