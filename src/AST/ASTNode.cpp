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

#include <AST/ASTContext.h>
#include "AST/ASTNode.h"

using namespace fly;

ASTNode::ASTNode(const StringRef fileName, const FileID &fid, ASTContext *Context) :
        ASTNodeBase(fileName, fid, Context) {
}

const ASTNameSpace* ASTNode::getNameSpace() {
    return NameSpace;
}

void ASTNode::setNameSpace() {
    StringRef NS = "default";
    setNameSpace(NS);
}

void ASTNode::setNameSpace(StringRef NS) {
    // Check if NS exist or add
    NameSpace = Context->NameSpaces.lookup(NS);
    if (NameSpace == nullptr) {
        NameSpace = new ASTNameSpace(NS);
        Context->NameSpaces.insert(std::make_pair(NS, NameSpace));
    }
}

const llvm::StringMap<ImportDecl*> &ASTNode::getImports() {
    return Imports;
}

bool ASTNode::addImport(ImportDecl * NewImport) {
    // Check if this Node already own this Import
    ImportDecl* Import = Imports.lookup(NewImport->getName());
    if (Import != nullptr) {
        Context->Diag(NewImport->getLocation(), diag::err_duplicate_import)  << NewImport->getName();
        return false;
    }

    // Retrieve Import from Context if already exists in order to maintain only one instance of ImportDecl
    Import = Context->Imports.lookup(NewImport->getName());
    if (Import == nullptr) {
        Import = NewImport;
    } else {
        delete NewImport;
    }
    auto Pair = std::make_pair(Import->getName(), Import);

    // Add Import to Context
    Context->Imports.insert(Pair);

    // Add Import to Node
    Imports.insert(Pair);

    return true;
}

const llvm::StringMap<GlobalVarDecl *> &ASTNode::getVars() {
    return Vars;
}

bool ASTNode::addGlobalVar(GlobalVarDecl *Var) {

    // Lookup into namespace
    if(Var->Visibility == VisibilityKind::V_PUBLIC || Var->Visibility == VisibilityKind::V_DEFAULT) {
        GlobalVarDecl *LookupVar = NameSpace->Vars.lookup(Var->getName());
        if (LookupVar) {
            Context->Diag(LookupVar->getLocation(), diag::err_duplicate_gvar)  << LookupVar->getName();
            return false;
        }
        auto Pair = std::make_pair(Var->getName(), Var);
        NameSpace->Vars.insert(Pair);
    }

    // Lookup into module vars
    GlobalVarDecl *LookupVar = Vars.lookup(Var->getName());
    if (LookupVar) {
        Context->Diag(LookupVar->getLocation(), diag::err_duplicate_gvar)  << LookupVar->getName();
        return false;
    }
    auto Pair = std::make_pair(Var->getName(), Var);
    Vars.insert(Pair);

    return true;
}

bool ASTNode::addFunction(FunctionDecl *Func) {
    // Lookup into namespace
    if(Func->Visibility == VisibilityKind::V_PUBLIC || Func->Visibility == VisibilityKind::V_DEFAULT) {
        FunctionDecl *LookupFunc = NameSpace->Functions.lookup(Func->getName());
        if (LookupFunc) {
            Context->Diag(LookupFunc->getLocation(), diag::err_duplicate_func)  << LookupFunc->getName();
            return false;
        }
        auto Pair = std::make_pair(Func->getName(), Func);
        NameSpace->Functions.insert(Pair);
    }

    // Lookup into module vars
    GlobalVarDecl *LookupFunc = Vars.lookup(Func->getName());
    if (LookupFunc) {
        Context->Diag(LookupFunc->getLocation(), diag::err_duplicate_func)  << LookupFunc->getName();
        return false;
    }
    auto Pair = std::make_pair(Func->getName(), Func);
    Functions.insert(Pair);

    return true;
}

const llvm::StringMap<FunctionDecl *> &ASTNode::getFunctions() {
    return Functions;
}

bool ASTNode::addClass(ClassDecl *Class) {
    // Lookup into namespace
    if(Class->Visibility == VisibilityKind::V_PUBLIC || Class->Visibility == VisibilityKind::V_DEFAULT) {
        ClassDecl *LookupClass = NameSpace->Classes.lookup(Class->Name);
        if (LookupClass) {
            Context->Diag(LookupClass->Location, diag::err_duplicate_class)  << LookupClass->Name;
            return false;
        }
        auto Pair = std::make_pair(Class->Name, Class);
        NameSpace->Classes.insert(Pair);
    }

    // Lookup into module classes
    ClassDecl *LookupClass = Classes.lookup(Class->Name);
    if (LookupClass) {
        Context->Diag(LookupClass->Location, diag::err_duplicate_class)  << LookupClass->Name;
        return false;
    }
    auto Pair = std::make_pair(Class->Name, Class);
    Classes.insert(Pair);

    return true;
}

const llvm::StringMap<ClassDecl *> &ASTNode::getClasses() {
    return Classes;
}

bool ASTNode::Finalize() {
    return Context->AddNode(this);
}

bool ASTNode::isFirstNode() const {
    return FirstNode;
}

void ASTNode::setFirstNode(bool First) {
    ASTNode::FirstNode = First;
}

ASTNode::~ASTNode() {
    Vars.clear();
    Functions.clear();
    Imports.clear();
}
