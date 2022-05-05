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
ASTContext::ASTContext() {
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
 * Get all Nodes from the Context
 * @return
 */
const llvm::StringMap<ASTNode *> &ASTContext::getNodes() const {
    return Nodes;
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

    // Add to Nodes
    auto Pair = std::make_pair(Node->getName(), Node);
    Node->getNameSpace()->Nodes.insert(Pair);
    Nodes.insert(Pair);

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
