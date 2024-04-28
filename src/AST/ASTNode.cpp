//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTNode.cpp - AST Node implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTContext.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTNode.h"

using namespace fly;

ASTNode::ASTNode(const std::string FileName, ASTContext *Context, bool isHeader) :
        Name(FileName), Context(Context), Header(isHeader) {
}

ASTNode::~ASTNode() {
    Imports.clear();
}

bool ASTNode::isHeader() const {
    return Header;
}

ASTContext &ASTNode::getContext() const {
    return *Context;
}

std::string ASTNode::getName() {
    return Name;
}

ASTNameSpace* ASTNode::getNameSpace() {
    return NameSpace;
}

const llvm::StringMap<ASTImport*> &ASTNode::getImports() {
    return Imports;
}

const llvm::StringMap<ASTImport*> &ASTNode::getAliasImports() {
    return AliasImports;
}

const llvm::StringMap<ASTGlobalVar *> &ASTNode::getExternalGlobalVars() const {
    return ExternalGlobalVars;
}

const llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> &ASTNode::getExternalFunctions() const {
    return ExternalFunctions;
}

ASTIdentity *ASTNode::getIdentity() const {
    return Identity;
}

const llvm::StringMap<ASTGlobalVar *> &ASTNode::getGlobalVars() const {
    return GlobalVars;
}

const llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> &ASTNode::getFunctions() const {
    return Functions;
}

std::string ASTNode::str() const {
    return Logger("ASTNode").
           Attr("Name", Name).
           End();
}
