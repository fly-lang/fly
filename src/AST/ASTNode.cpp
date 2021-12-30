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
#include "AST/ASTUnref.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"
#include "llvm/ADT/StringMap.h"

using namespace fly;

ASTNode::ASTNode(const std::string FileName, ASTContext *Context, CodeGenModule * CGM) :
        ASTNodeBase(FileName, Context), CGM(CGM) {
    FLY_DEBUG_MESSAGE("ASTNode", "ASTNode", "FileName=" << FileName);
}

ASTNode::~ASTNode() {
    FLY_DEBUG("ASTNode", "~ASTNode");
    Imports.clear();
}

CodeGenModule *ASTNode::getCodeGen() const {
    return CGM;
}

ASTNameSpace* ASTNode::getNameSpace() {
    return NameSpace;
}

void ASTNode::setDefaultNameSpace() {
    llvm::SmallVector<std::string, 4> Names;
    Names.push_back(ASTNameSpace::DEFAULT);
    FLY_DEBUG("ASTNode", "setDefaultNameSpace");
    setNameSpace(Names);
}

ASTNameSpace *ASTNode:: FindNameSpace(const std::string &Name) {
    FLY_DEBUG_MESSAGE("ASTNode", "findNameSpace", "Name=" << Name);
    if (Name.empty()) { // return current NameSpace if not set
        return NameSpace;
    } else {
        auto NS = Context->NameSpaces.find(Name);
        return NS == Context->NameSpaces.end() ? nullptr : NS->getValue();
    }
}

ASTNameSpace *ASTNode::setNameSpace(std::string Name) {
    SmallVector<std::string, 4> Names;
    Names.push_back(Name);
    return setNameSpace(Names);
}

ASTNameSpace *ASTNode::setNameSpace(llvm::SmallVector<std::string, 4> Names) {
    std::string NS = ASTNameSpace::flat(Names);
    FLY_DEBUG_MESSAGE("ASTNode", "setNameSpace", "Names=" << NS);

    // Check if Name exist or add it
    NameSpace = Context->NameSpaces.lookup(NS);
    if (NameSpace == nullptr) {
        NameSpace = new ASTNameSpace(Names, Context);
        Context->NameSpaces.insert(std::make_pair(NS, NameSpace));
    }
    return NameSpace;
}

const llvm::StringMap<ASTImport*> &ASTNode::getImports() {
    return Imports;
}

bool ASTNode::AddImport(ASTImport * Import) {
    FLY_DEBUG_MESSAGE("ASTNode", "AddImport", "Import=" << Import->str());
    // Check if this Node already own this Import
    ASTImport* FoundImport = Imports.lookup(Import->getName());
    if (FoundImport != nullptr) {
        Context->Diag(Import->getLocation(), diag::err_duplicate_import) << Import->getName();
        return false;
    }

    // Retrieve Import from Context if already exists in order to maintain only one instance of ImportDecl
    FoundImport = Context->Imports.lookup(Import->getName());
    if (FoundImport == nullptr) {
        FoundImport = Import;
    }
    auto Pair = std::make_pair(FoundImport->getName(), FoundImport);

    // Add Import to Context
    Context->Imports.insert(Pair);

    // Add Import to Node
    Imports.insert(Pair);

    return true;
}

bool ASTNode::AddGlobalVar(ASTGlobalVar *GVar) {
    assert(GVar->Visibility && "Function Visibility is unset");
    FLY_DEBUG_MESSAGE("ASTNode", "AddGlobalVar", "Var=" << GVar->str());

    // Lookup into namespace for public var
    if(GVar->Visibility == VisibilityKind::V_PUBLIC || GVar->Visibility == VisibilityKind::V_DEFAULT) {
        ASTGlobalVar *LookupVar = NameSpace->getGlobalVars().lookup(GVar->getName());
        if (LookupVar) { // This NameSpace already contains this GlobalVar
            Context->Diag(LookupVar->getLocation(), diag::err_duplicate_gvar) << LookupVar->getName();
            return false;
        }

        // Add into NameSpace for global resolution
        // Add into Node for local resolution
        auto Pair = std::make_pair(GVar->getName(), GVar);
        return GlobalVars.insert(Pair).second && NameSpace->GlobalVars.insert(Pair).second;
    }

    // Lookup into node for private var
    if(GVar->Visibility == VisibilityKind::V_PRIVATE) {
        ASTGlobalVar *LookupVar = GlobalVars.lookup(GVar->getName());
        if (LookupVar) { // This Node already contains this Function
            Context->Diag(LookupVar->getLocation(), diag::err_duplicate_gvar) << LookupVar->getName();
            return false;
        }

        // Add into Node for local resolution
        auto Pair = std::make_pair(GVar->getName(), GVar);
        return GlobalVars.insert(Pair).second;
    }

    assert(0 && "Error when adding GlobalVar");
}

bool ASTNode::AddFunction(ASTFunc *Func) {
    assert(Func->Visibility && "Function Visibility is unset");
    FLY_DEBUG_MESSAGE("ASTNode", "AddFunction", "Func=" << Func->str());

    // Lookup into namespace for public var
    if(Func->Visibility == VisibilityKind::V_PUBLIC || Func->Visibility == VisibilityKind::V_DEFAULT) {
        const auto &FuncIt = NameSpace->getFunctions().find(Func);
        if (FuncIt != NameSpace->getFunctions().end()) { // This NameSpace already contains this Function
            Context->Diag(Func->getLocation(), diag::err_duplicate_func) << Func->getName();
            return false;
        }

        // Add into NameSpace for global resolution
        // Add into Node for local resolution
        ASTFuncCall *Call = ASTFuncCall::CreateCall(Func);
        if (NameSpace->Functions.insert(Func).second &&
                NameSpace->AddFunctionCall(Call) &&
                Functions.insert(Func).second &&
                AddFunctionCall(Call)) {
            return true;
        }

        Context->Diag(Func->getLocation(), diag::err_add_func) << Func->getName();
        return false;
    }

    // Lookup into node for private var
    if (Func->Visibility == VisibilityKind::V_PRIVATE) {
        const auto &FuncIt = Functions.find(Func);
        if (FuncIt != Functions.end()) { // This Node already contains this Function
            Context->Diag(Func->getLocation(), diag::err_duplicate_func) << Func->getName();
            return false;
        }

        // Add into Node for local resolution (Private)
        if (Functions.insert(Func).second && AddFunctionCall(ASTFuncCall::CreateCall(Func))) {
            return true;
        }

        Context->Diag(Func->getLocation(), diag::err_add_func) << Func->getName();
        return false;
    }

    assert(0 && "Error when adding Function");
}

bool ASTNode::AddClass(ASTClass *Class) {
    FLY_DEBUG_MESSAGE("ASTNode", "AddFunction", "Class" << Class->str());

    // Lookup into namespace
    // TODO Class scope differences
    ASTClass *LookupClass = NameSpace->getClasses().lookup(Class->getName());
    if (LookupClass) { // This NameSpace already contains this Function
        Context->Diag(LookupClass->Location, diag::err_duplicate_class)  << LookupClass->getName();
        return false;
    }
    NameSpace->Classes.insert(std::make_pair(Class->getName(), Class));
}

bool ASTNode::AddExternalGlobalVar(ASTGlobalVar *Var) {
    FLY_DEBUG_MESSAGE("ASTNode", "AddExternalGlobalVar", "Var=" << Var->str());
    return ExternalGlobalVars.insert(std::make_pair(Var->getName(), Var)).second;
}

const llvm::StringMap<ASTGlobalVar *> &ASTNode::getExternalGlobalVars() const {
    return ExternalGlobalVars;
}

bool ASTNode::AddExternalFunction(ASTFunc *Call) {
    FLY_DEBUG_MESSAGE("ASTNode", "AddExternalFunction", "Call=" << Call->str());
    return ExternalFunctions.insert(Call).second;
}

const std::unordered_set<ASTFunc *> &ASTNode::getExternalFunctions() const {
    return ExternalFunctions;
}

bool ASTNode::AddUnrefCall(ASTFuncCall *Call) {
    FLY_DEBUG_MESSAGE("ASTNode", "AddUnrefCall", "Node.Name=" << getName() <<
        ", Call=" << Call->str());
    ASTUnrefCall *Unref = new ASTUnrefCall(this, Call);
    if (Call->getNameSpace().empty()) {
        UnrefFunctionCalls.push_back(Unref);
    } else if (Call->getNameSpace() == getNameSpace()->getName()) {
        getNameSpace()->UnrefFunctionCalls.push_back(Unref);
    } else {
        ASTNameSpace *FoundNS = FindNameSpace(Call->getNameSpace());
        if (FoundNS == nullptr) {
            Context->Diag(Call->getLocation(), diag::err_namespace_notfound)
                    << Call->getNameSpace();
            return false;
        }
        FoundNS->UnrefFunctionCalls.push_back(Unref);
    }
    return true;
}

bool ASTNode::AddUnrefGlobalVar(ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("ASTNode", "AddUnrefGlobalVar", "Node.Name=" << getName() <<
        ", VarRef=" << VarRef->str());
    ASTUnrefGlobalVar *Unref = new ASTUnrefGlobalVar(this, *VarRef);
    if (VarRef->getNameSpace().empty()) {
        UnrefGlobalVars.push_back(Unref);
    } else if (VarRef->getNameSpace() == getNameSpace()->getName()) {
        getNameSpace()->UnrefGlobalVars.push_back(Unref);
    } else {
        ASTNameSpace *FoundNS = FindNameSpace(VarRef->getNameSpace());
        if (FoundNS == nullptr) {
            Context->Diag(VarRef->getLocation(), diag::err_namespace_notfound)
                    << VarRef->getNameSpace();
            return false;
        }
        FoundNS->UnrefGlobalVars.push_back(Unref);
    }
    return true;
}

bool ASTNode::Resolve() {
    return ASTResolver::Resolve(this);
}
