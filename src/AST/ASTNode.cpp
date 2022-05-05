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
#include "AST/ASTFunctionCall.h"
#include "AST/ASTClass.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTUnref.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"
#include "llvm/ADT/StringMap.h"

using namespace fly;

ASTNode::ASTNode(const std::string FileName, ASTContext *Context) :
        ASTNodeBase(FileName, Context), Header(true) {
    FLY_DEBUG_MESSAGE("ASTNode", "ASTNode", "FileName=" << FileName);
}

ASTNode::ASTNode(const std::string FileName, ASTContext *Context, CodeGenModule * CGM) :
        ASTNodeBase(FileName, Context), CGM(CGM), Header(false) {
    FLY_DEBUG_MESSAGE("ASTNode", "ASTNode", "FileName=" << FileName);
}

ASTNode::~ASTNode() {
    FLY_DEBUG("ASTNode", "~ASTNode");
    Imports.clear();
}

CodeGenModule *ASTNode::getCodeGen() const {
    return CGM;
}

const bool ASTNode::isHeader() const {
    return Header;
}

ASTNameSpace* ASTNode::getNameSpace() {
    return NameSpace;
}

void ASTNode::setDefaultNameSpace() {
    FLY_DEBUG("ASTNode", "setDefaultNameSpace");
    setNameSpace(ASTNameSpace::DEFAULT);
}

ASTImport *ASTNode:: FindImport(const std::string &Name) {
    FLY_DEBUG_MESSAGE("ASTNode", "FindImport", "Name=" << Name);

    // Search into Node imports
    return Imports.lookup(Name);
}

void ASTNode::setNameSpace(std::string Name) {
    FLY_DEBUG_MESSAGE("ASTNode", "setNameSpace", "Name=" << Name);
    NameSpace = Context->AddNameSpace(Name);
}

const llvm::StringMap<ASTImport*> &ASTNode::getImports() {
    return Imports;
}

bool ASTNode::AddExternalGlobalVar(ASTGlobalVar *Var) {
    FLY_DEBUG_MESSAGE("ASTNode", "AddExternalGlobalVar", "Var=" << Var->str());
    return ExternalGlobalVars.insert(std::make_pair(Var->getName(), Var)).second;
}

const llvm::StringMap<ASTGlobalVar *> &ASTNode::getExternalGlobalVars() const {
    return ExternalGlobalVars;
}

bool ASTNode::AddExternalFunction(ASTFunction *Call) {
    FLY_DEBUG_MESSAGE("ASTNode", "AddExternalFunction", "Call=" << Call->str());
    return ExternalFunctions.insert(Call).second;
}

const std::unordered_set<ASTFunction *> &ASTNode::getExternalFunctions() const {
    return ExternalFunctions;
}
