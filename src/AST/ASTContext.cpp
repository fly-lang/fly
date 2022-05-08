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

using namespace fly;

/**
 * ASTContext constructor
 * @param Diags
 */
ASTContext::ASTContext() {

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
 * Get all Nodes from the Context
 * @return
 */
const llvm::StringMap<ASTNode *> &ASTContext::getNodes() const {
    return Nodes;
}
