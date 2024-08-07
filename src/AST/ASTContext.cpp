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

std::string ASTContext::DEFAULT_NAMESPACE = "default";

/**
 * ASTContext destructor
 */
ASTContext::~ASTContext() {
    Modules.clear();
}

uint64_t ASTContext::getNextModuleId() {
    return ++ModuleIdCounter;
}

/**
 * Get all Modules from the Context
 * @return
 */
const llvm::SmallVector<ASTModule *, 8> &ASTContext::getModules() const {
    return Modules;
}
