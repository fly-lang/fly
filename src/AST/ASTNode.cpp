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
#include "AST/ASTFunc.h"
#include "AST/ASTClass.h"
#include "AST/ASTLocalVar.h"
#include "Basic/Diagnostic.h"
#include "llvm/ADT/StringMap.h"

using namespace fly;

ASTNode::ASTNode(const llvm::StringRef &FileName, ASTContext *Context, CodeGenModule * CGM) :
        ASTNodeBase(FileName, Context), CGM(CGM) {
}

ASTNode::~ASTNode() {
    Imports.clear();
}

CodeGenModule *ASTNode::getCodeGen() const {
    return CGM;
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
        return NS == Context->NameSpaces.end() ? nullptr : NS->getValue();
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

const llvm::StringMap<ASTImport*> &ASTNode::getImports() {
    return Imports;
}

bool ASTNode::addImport(ASTImport * NewImport) {
    // Check if this Node already own this Import
    ASTImport* Import = Imports.lookup(NewImport->getName());
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

const llvm::StringMap<ASTGlobalVar *> &ASTNode::getGlobalVars() {
    return GlobalVars;
}

bool ASTNode::addGlobalVar(ASTGlobalVar *Var) {
    assert(Var->Visibility && "Function Visibility is unset");

    // Lookup into namespace for public var
    if(Var->Visibility == VisibilityKind::V_PUBLIC || Var->Visibility == VisibilityKind::V_DEFAULT) {
        ASTGlobalVar *LookupVar = NameSpace->getGlobalVars().lookup(Var->getName());
        if (LookupVar) {
            Context->Diag(LookupVar->getLocation(), diag::err_duplicate_gvar) << LookupVar->getName();
            return false;
        }
        auto Pair = std::make_pair(Var->getName(), Var);
        return GlobalVars.insert(Pair).second && NameSpace->addGlobalVar(Var);
    }

    // Lookup into node for private var
    if(Var->Visibility == VisibilityKind::V_PRIVATE) {
        ASTGlobalVar *LookupVar = GlobalVars.lookup(Var->getName());
        if (LookupVar) {
            Context->Diag(LookupVar->getLocation(), diag::err_duplicate_gvar) << LookupVar->getName();
            return false;
        }
        auto Pair = std::make_pair(Var->getName(), Var);
        return GlobalVars.insert(Pair).second;
    }

    assert(0 && "Error when adding GlobalVar");
}

bool ASTNode::addResolvedCall(ASTFuncCall *Call) {
    const auto &It = ResolvedCalls.find(Call->getName());
    if (It == ResolvedCalls.end()) {
        std::vector<ASTFuncCall *> Functions;
        Functions.push_back(Call);
        return ResolvedCalls.insert(std::make_pair(Call->getName(), Functions)).second;
    }
    It->getValue().push_back(Call);
    return true;
}

const llvm::StringMap<std::vector<ASTFuncCall *>> &ASTNode::getResolvedCalls() const {
    return ResolvedCalls;
}

bool ASTNode::addFunction(ASTFunc *Func) {
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
        if (NameSpace->addFunction(Func) && Functions.insert(Func).second &&
                addResolvedCall(ASTFuncCall::CreateCall(Func))) {
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
        if (Functions.insert(Func).second && addResolvedCall(ASTFuncCall::CreateCall(Func))) {
            return true;
        }

        Context->Diag(Func->getLocation(), diag::err_add_func) << Func->getName();
        return false;
    }

    assert(0 && "Error when adding Function");
}

const std::unordered_set<ASTFunc*> &ASTNode::getFunctions() const {
    return Functions;
}

bool ASTNode::addClass(ASTClass *Class) {
    // Lookup into namespace
    ASTClass *LookupClass = NameSpace->getClasses().lookup(Class->getName());
    if (LookupClass) {
        Context->Diag(LookupClass->Location, diag::err_duplicate_class)  << LookupClass->getName();
        return false;
    }
    return NameSpace->addClass(Class);
}

const llvm::StringMap<ASTClass *> &ASTNode::getClasses() {
    return NameSpace->getClasses();
}

ASTType *ASTNode::ResolveExprType(ASTExpr *E) {
    switch (E->getKind()) {

        case EXPR_VALUE:
            return ((ASTValueExpr *) E)->getValue().getType();
        case EXPR_REF_VAR:
            return ((ASTVarRefExpr *) E)->getVarRef()->getDecl()->getType();
        case EXPR_REF_FUNC:
            return ((ASTFuncCallExpr *) E)->getCall()->getDecl()->getType();
        case EXPR_GROUP:
            return ResolveExprType(((ASTGroupExpr *) E)->getGroup().at(0));
    }
    return nullptr;
}

void ASTNode::addUnRefCall(ASTFuncCall *Call) {
    UnRefCalls.push_back(Call);
}

void ASTNode::addUnRefGlobalVar(ASTVarRef *Var) {
    UnRefGlobalVars.push_back(Var);
}

bool ASTNode::Resolve() {
    return Resolve(UnRefGlobalVars, GlobalVars, UnRefCalls,ResolvedCalls);
}

bool ASTNode::Resolve(std::vector<ASTVarRef *> &UnRefGlobalVars,
                      llvm::StringMap<ASTGlobalVar *> &GlobalVars,
                      std::vector<ASTFuncCall *> &UnRefCalls,
                      llvm::StringMap<std::vector<ASTFuncCall *>> &ResolvedCalls) {
    bool Success = ResolveGlobalVar(UnRefGlobalVars, GlobalVars);
    for (ASTFunc *Function : Functions) {
        Success &= ResolveFunction(Function,UnRefCalls,ResolvedCalls);
    }
    return Success;
}

bool ASTNode::ResolveGlobalVar(std::vector<ASTVarRef *> &UnRefGlobalVars,
                      llvm::StringMap<ASTGlobalVar *> &GlobalVars) {
    bool Success = true;

    // Resolve Unreferenced Global Var (node internal)
    for (const auto &UnRefGlobalVar : UnRefGlobalVars) {
        const auto &It = GlobalVars.find(UnRefGlobalVar->getName());
        if (It == GlobalVars.end()) {
            Context->Diag(UnRefGlobalVar->getLocation(), diag::err_gvar_notfound)
                    << UnRefGlobalVar->getName();
            Success = false; // collects other errors
        } else {
            UnRefGlobalVar->setDecl((ASTVar *)It->getValue());
        }
    }

    return Success;
}

bool ASTNode::ResolveFunction(ASTFunc *Function,
                      std::vector<ASTFuncCall *> &UnRefCalls,
                      llvm::StringMap<std::vector<ASTFuncCall *>> &ResolvedCalls) {
    bool Success = true;

    // Resolve Unreferenced Function Calls (node internal)
    for (auto *UnRefCall : UnRefCalls) {
        const auto &It = ResolvedCalls.find(UnRefCall->getName());
        if (It == ResolvedCalls.end()) {
            Context->Diag(UnRefCall->getLocation(), diag::err_unref_call)
                    << UnRefCall->getName();
            Success = false; // collects other errors
        } else {
            for (auto &ResolvedCall : It->getValue()) {
                if (Function->ResolveCall(ResolvedCall, UnRefCall)) {
                    UnRefCall->setDecl(ResolvedCall->getDecl());
                } else {
                    Success = false;
                }
            }
            if (!Success) { // Not Found
                Context->Diag(UnRefCall->getLocation(), diag::err_unref_call)
                        << UnRefCall->getName();
            }
        }
    }
    return Success;
}

