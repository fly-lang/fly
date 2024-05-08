//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTModule.cpp - AST Module implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTContext.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTModule.h"

using namespace fly;

ASTModule::ASTModule(const std::string FileName, ASTContext *Context, bool isHeader) :
        Name(FileName), Context(Context), Header(isHeader) {
}

ASTModule::~ASTModule() {
    Imports.clear();
}

bool ASTModule::isHeader() const {
    return Header;
}

ASTContext &ASTModule::getContext() const {
    return *Context;
}

std::string ASTModule::getName() {
    return Name;
}

ASTNameSpace* ASTModule::getNameSpace() {
    return NameSpace;
}

const llvm::StringMap<ASTImport*> &ASTModule::getImports() {
    return Imports;
}

const llvm::StringMap<ASTImport*> &ASTModule::getAliasImports() {
    return AliasImports;
}

const llvm::StringMap<ASTGlobalVar *> &ASTModule::getExternalGlobalVars() const {
    return ExternalGlobalVars;
}

const llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> &ASTModule::getExternalFunctions() const {
    return ExternalFunctions;
}

ASTIdentity *ASTModule::getIdentity() const {
    return Identity;
}

const llvm::StringMap<ASTGlobalVar *> &ASTModule::getGlobalVars() const {
    return GlobalVars;
}

const llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> &ASTModule::getFunctions() const {
    return Functions;
}

CodeGenModule *ASTModule::getCodeGen() const {
    return CodeGen;
}

std::string ASTModule::str() const {
    return Logger("ASTModule").
           Attr("Name", Name).
           End();
}
