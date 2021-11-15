//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTNameSpace.cpp - AST Namespace implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
// This file implements the ASTNameSpace interface.
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTNameSpace.h"
#include "AST/ASTContext.h"
#include "AST/ASTNode.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTClass.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"

using namespace fly;

const llvm::StringRef ASTNameSpace::DEFAULT = "default";

ASTNameSpace::ASTNameSpace(const llvm::StringRef &Name, ASTContext *Context) : ASTNodeBase(Name, Context) {
    FLY_DEBUG_MESSAGE("ASTNameSpace", "ASTNameSpace", "Name=" << Name);
}

ASTNameSpace::~ASTNameSpace() {
    FLY_DEBUG("ASTNameSpace", "~ASTNameSpace");
    Nodes.clear();
    GlobalVars.clear();
}

const llvm::StringMap<ASTNode*> &ASTNameSpace::getNodes() const {
    return Nodes;
}
