//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTContext.cpp - AST Context implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTContext.h"
#include "AST/ASTNameSpace.h"

using namespace fly;

/**
 * ASTContext constructor
 * @param Diags
 */
ASTContext::ASTContext() = default;

/**
 * ASTContext destructor
 */
ASTContext::~ASTContext() {
    delete DefaultNameSpace;
    NameSpaces.clear();
    Modules.clear();
    ExternalImports.clear();
}

uint64_t ASTContext::getNextModuleId() {
    return ++ModuleIdCounter;
}

/**
 * Get the Default Namespace
 * @return DefaultNS
 */
ASTNameSpace *ASTContext::getDefaultNameSpace() const {
    return DefaultNameSpace;
}

/**
 * Get all added namespaces in the context
 * @return a StringMap of ASTNameSpace
 */
const llvm::StringMap<ASTNameSpace *> &ASTContext::getNameSpaces() const {
    return NameSpaces;
}

/**
 * Get all Modules from the Context
 * @return
 */
const llvm::SmallVector<ASTModule *, 8> &ASTContext::getModules() const {
    return Modules;
}
