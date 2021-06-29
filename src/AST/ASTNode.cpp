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
#include "AST/GlobalVarDecl.h"
#include "AST/FuncDecl.h"
#include "AST/ClassDecl.h"
#include "llvm/ADT/StringMap.h"

using namespace fly;

ASTNode::ASTNode(const llvm::StringRef &fileName, const FileID &fid, ASTContext *Context) :
        ASTNodeBase(fileName, fid, Context) {
}

ASTNameSpace* ASTNode::getNameSpace() {
    return NameSpace;
}

void ASTNode::setDefaultNameSpace() {
    setNameSpace(ASTNameSpace::DEFAULT);
}

ASTNameSpace *ASTNode::findNameSpace(const StringRef &Name) {
    auto NS = Context->NameSpaces.find(Name);
    return NS == Context->NameSpaces.end() ? nullptr : NS->getValue();
}

ASTNameSpace *ASTNode::setNameSpace(llvm::StringRef NS) {
    // Check if NS exist or add
    NameSpace = Context->NameSpaces.lookup(NS);
    if (NameSpace == nullptr) {
        NameSpace = new ASTNameSpace(NS);
        Context->NameSpaces.insert(std::make_pair(NS, NameSpace));
    }
    return NameSpace;
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

const llvm::StringMap<GlobalVarDecl *> &ASTNode::getGlobalVars() {
    return GlobalVars;
}

bool ASTNode::addGlobalVar(GlobalVarDecl *Var) {

    // Lookup into namespace
    if(Var->Visibility == VisibilityKind::V_PUBLIC || Var->Visibility == VisibilityKind::V_DEFAULT) {
        GlobalVarDecl *LookupVar = NameSpace->GlobalVars.lookup(Var->getName());
        if (LookupVar) {
            Context->Diag(LookupVar->getLocation(), diag::err_duplicate_gvar)  << LookupVar->getName();
            return false;
        }
        auto Pair = std::make_pair(Var->getName(), Var);
        NameSpace->GlobalVars.insert(Pair);
    }

    // Lookup into node vars
    GlobalVarDecl *LookupVar = GlobalVars.lookup(Var->getName());
    if (LookupVar) {
        Context->Diag(LookupVar->getLocation(), diag::err_duplicate_gvar)  << LookupVar->getName();
        return false;
    }
    auto Pair = std::make_pair(Var->getName(), Var);
    GlobalVars.insert(Pair);

    return true;
}

bool ASTNode::addFunction(FuncDecl *Func) {
    assert(Func->Visibility && "Function Visibility is unset");

    // Works into namespace
    if(Func->Visibility == VisibilityKind::V_PUBLIC || Func->Visibility == VisibilityKind::V_DEFAULT) {
        const std::unordered_set<FuncDecl *>::iterator &FuncIt = NameSpace->Functions.find(Func);
        if (FuncIt != NameSpace->Functions.end()) {
            Context->Diag(Func->getLocation(), diag::err_duplicate_func)  << Func->getName();
            return false;
        }
        assert(NameSpace->Functions.insert(Func).second && "Error on Function insert into NameSpace");
    }

    // Works into node functions
    if (Func->Visibility == VisibilityKind::V_PRIVATE) {
        const std::unordered_set<FuncDecl *>::iterator &FuncIt = Functions.find(Func);
        if (FuncIt != Functions.end()) {
            Context->Diag(Func->getLocation(), diag::err_duplicate_func) << Func->getName();
            return false;
        }
        assert(Functions.insert(Func).second && "Error on Function insert into Node");
    }

    return NameSpace->Calls.insert(FuncCall::CreateCall(Func)).second;
}

const std::unordered_set<FuncDecl *> &ASTNode::getFunctions() {
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
    for (auto *Function : Functions) {
        Function->Finalize();
    }

    return Context->AddNode(this);
}

bool ASTNode::isFirstNode() const {
    return FirstNode;
}

void ASTNode::setFirstNode(bool First) {
    ASTNode::FirstNode = First;
}

ASTNode::~ASTNode() {
    GlobalVars.clear();
    Functions.clear();
    Imports.clear();
}
