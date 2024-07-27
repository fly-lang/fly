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

ASTModule::ASTModule(uint64_t &Id, const std::string Name, ASTContext *Context, bool isHeader) :
        Id(Id), Name(Name), Context(Context), Header(isHeader) {
}

ASTModule::~ASTModule() {
    Imports.clear();
}

const uint64_t ASTModule::getId() const {
    return Id;
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

const llvm::SmallVector<ASTImport *, 8> &ASTModule::getImports() {
    return Imports;
}

const llvm::SmallVector<ASTImport *, 8> &ASTModule::getAliasImports() {
    return AliasImports;
}

const llvm::StringMap<ASTGlobalVar *> &ASTModule::getExternalGlobalVars() const {
    return ExternalGlobalVars;
}

const llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> &ASTModule::getExternalFunctions() const {
    return ExternalFunctions;
}

const llvm::SmallVector<ASTIdentity *, 8> &ASTModule::getIdentities() const {
    return Identities;
}

const llvm::SmallVector<ASTGlobalVar *, 8> &ASTModule::getGlobalVars() const {
    return GlobalVars;
}

const llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> &ASTModule::getFunctions() const {
    return Functions;
}

CodeGenModule *ASTModule::getCodeGen() const {
    return CodeGen;
}

void ASTModule::setCodeGen(CodeGenModule *CGM) {
    CodeGen = CGM;
}

std::string ASTModule::str() const {
    return Logger("ASTModule").
           Attr("Name", Name).
           End();
}
