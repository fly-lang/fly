//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTNode.cpp - AST Node implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
// This file implements the ASTNode interface.
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTContext.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTNode.h"
#include "AST/ASTImport.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTClass.h"
#include "AST/ASTLocalVar.h"
#include "llvm/ADT/StringMap.h"

using namespace fly;

ASTNode::ASTNode(const std::string FileName, ASTContext *Context, bool isHeader) :
        ASTNodeBase(FileName, Context), Header(isHeader) {
}

ASTNode::~ASTNode() {
    Imports.clear();
}

const bool ASTNode::isHeader() const {
    return Header;
}

ASTNameSpace* ASTNode::getNameSpace() {
    return NameSpace;
}

const llvm::StringMap<ASTImport*> &ASTNode::getImports() {
    return Imports;
}

const llvm::StringMap<ASTGlobalVar *> &ASTNode::getExternalGlobalVars() const {
    return ExternalGlobalVars;
}

const llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> &ASTNode::getExternalFunctions() const {
    return ExternalFunctions;
}

ASTClass *ASTNode::getClass() const {
    return Class;
}

std::string ASTNode::str() const {
    return Logger("ASTNode").
           Attr("Name", Name).
           End();
}
