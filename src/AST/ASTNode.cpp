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

ASTNode::ASTNode(const llvm::StringRef &FileName, const FileID &FID, ASTContext *Context) :
        ASTNodeBase(FileName, FID, Context) {
}

ASTNode::~ASTNode() {
    Imports.clear();
}

bool ASTNode::isFirstNode() const {
    return FirstNode;
}

void ASTNode::setFirstNode(bool First) {
    ASTNode::FirstNode = First;
}

ASTNameSpace* ASTNode::getNameSpace() {
    return NameSpace;
}

void ASTNode::setDefaultNameSpace() {
    setNameSpace(ASTNameSpace::DEFAULT);
}

ASTNameSpace *ASTNode::findNameSpace(const StringRef &Name) {
    if (Name.empty()) { // return current NameSpace if not set
        return NameSpace;
    } else {
        auto NS = Context->NameSpaces.find(Name);
        return NS == Context->NameSpaces.end() ? NULL : NS->getValue();
    }
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
    assert(Var->Visibility && "Function Visibility is unset");

    // Lookup into namespace for public var
    if(Var->Visibility == VisibilityKind::V_PUBLIC || Var->Visibility == VisibilityKind::V_DEFAULT) {
        GlobalVarDecl *LookupVar = NameSpace->getGlobalVars().lookup(Var->getName());
        if (LookupVar) {
            Context->Diag(LookupVar->getLocation(), diag::err_duplicate_gvar) << LookupVar->getName();
            return false;
        }
        auto Pair = std::make_pair(Var->getName(), Var);
        return GlobalVars.insert(Pair).second && NameSpace->addGlobalVar(Var);
    }

    // Lookup into node for private var
    if(Var->Visibility == VisibilityKind::V_PRIVATE) {
        GlobalVarDecl *LookupVar = GlobalVars.lookup(Var->getName());
        if (LookupVar) {
            Context->Diag(LookupVar->getLocation(), diag::err_duplicate_gvar) << LookupVar->getName();
            return false;
        }
        auto Pair = std::make_pair(Var->getName(), Var);
        return GlobalVars.insert(Pair).second;
    }

    assert(0 && "Error when adding GlobalVar");
}

bool ASTNode::addResolvedCall(FuncCall *Call) {
    const auto &It = ResolvedCalls.find(Call->getName());
    if (It == ResolvedCalls.end()) {
        std::vector<FuncCall *> Functions;
        Functions.push_back(Call);
        return ResolvedCalls.insert(std::make_pair(Call->getName(), Functions)).second;
    }
    It->getValue().push_back(Call);
    return true;
}

const llvm::StringMap<std::vector<FuncCall *>> &ASTNode::getResolvedCalls() const {
    return ResolvedCalls;
}

bool ASTNode::addFunction(FuncDecl *Func) {
    assert(Func->Visibility && "Function Visibility is unset");

    // Lookup into namespace for public var
    if(Func->Visibility == VisibilityKind::V_PUBLIC || Func->Visibility == VisibilityKind::V_DEFAULT) {
        const auto &FuncIt = NameSpace->getFunctions().find(Func);
        if (FuncIt != NameSpace->getFunctions().end()) {
            Context->Diag(Func->getLocation(), diag::err_duplicate_func) << Func->getName();
            return false;
        }

        // Add into NameSpace for global resolution
        // Add into Node for local resolution
        if (NameSpace->addFunction(Func) && Functions.insert(Func).second) {
            return true;
        }

        Context->Diag(Func->getLocation(), diag::err_add_func) << Func->getName();
        return false;
    }

    // Lookup into node for private var
    if (Func->Visibility == VisibilityKind::V_PRIVATE) {
        const auto &FuncIt = Functions.find(Func);
        if (FuncIt != Functions.end()) {
            Context->Diag(Func->getLocation(), diag::err_duplicate_func) << Func->getName();
            return false;
        }

        // Add into Node for local resolution
        if (Functions.insert(Func).second && addResolvedCall(FuncCall::CreateCall(Func))) {
            return true;
        }

        Context->Diag(Func->getLocation(), diag::err_add_func) << Func->getName();
        return false;
    }

    assert(0 && "Error when adding Function");
}

const std::unordered_set<FuncDecl*> &ASTNode::getFunctions() const {
    return Functions;
}

bool ASTNode::addClass(ClassDecl *Class) {
    // Lookup into namespace
    ClassDecl *LookupClass = NameSpace->getClasses().lookup(Class->getName());
    if (LookupClass) {
        Context->Diag(LookupClass->Location, diag::err_duplicate_class)  << LookupClass->getName();
        return false;
    }
    return NameSpace->addClass(Class);
}

const llvm::StringMap<ClassDecl *> &ASTNode::getClasses() {
    return NameSpace->getClasses();
}

TypeBase *ASTNode::ResolveExprType(Expr *E) {
    switch (E->getKind()) {

        case EXPR_VALUE:
            return ((ValueExpr *) E)->getValue().getType();
        case EXPR_REF_VAR:
            return ((VarRefExpr *) E)->getVarRef()->getDecl()->getType();
        case EXPR_REF_FUNC:
            return ((FuncCallExpr *) E)->getCall()->getDecl()->getType();
        case EXPR_GROUP:
            return ResolveExprType(((GroupExpr *) E)->getGroup().at(0));
    }
    return nullptr;
}

bool ASTNode::Finalize() {
    for (const auto &Function : Functions) {
        if (!Function->Finalize()) {
            return false;
        }
    }

    return Context->AddNode(this);
}
