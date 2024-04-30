//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/Sema.cpp - The Sema
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/Sema.h"
#include "Sema/SemaBuilder.h"
#include "Sema/SemaResolver.h"
#include "Sema/SemaValidator.h"
#include "AST/ASTContext.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTBlock.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTFunctionBase.h"
#include "AST/ASTFunction.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassAttribute.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTImport.h"
#include "AST/ASTClassMethod.h"
#include "AST/ASTNode.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTParam.h"
#include "Basic/Diagnostic.h"
#include "Basic/SourceLocation.h"
#include "AST/ASTBase.h"
#include "Basic/Debug.h"

#include "llvm/ADT/StringRef.h"

using namespace fly;

Sema::Sema(DiagnosticsEngine &Diags) : Diags(Diags) {

}

Sema* Sema::CreateSema(DiagnosticsEngine &Diags) {
    Sema *S = new Sema(Diags);
    S->Builder = new SemaBuilder(*S);
    S->Context = S->Builder->CreateContext();
    S->Resolver = new SemaResolver(*S);
    S->Validator = new SemaValidator(*S);
    return S;
}

DiagnosticsEngine &Sema::getDiags() const {
    return Diags;
}

SemaBuilder &Sema::getBuilder() {
    return *Builder;
}

SemaResolver &Sema::getResolver() const {
    return *Resolver;
}

SemaValidator &Sema::getValidator() const {
    return *Validator;
}

ASTContext &Sema::getContext() const {
    return *Context;
}

/**
 * Write Diagnostics
 * @param Loc
 * @param DiagID
 * @return
 */
DiagnosticBuilder Sema::Diag(SourceLocation Loc, unsigned DiagID) const {
    return Diags.Report(Loc, DiagID);
}

/**
 * Write Diagnostics
 * @param Loc
 * @param DiagID
 * @return
 */
DiagnosticBuilder Sema::Diag(unsigned DiagID) const {
    return Diags.Report(DiagID);
}

bool Sema::Resolve() {
    return Resolver->Resolve();
}

ASTNameSpace *Sema::FindNameSpace(llvm::StringRef Name) const {
    FLY_DEBUG_MESSAGE("Sema", "FindNameSpace", "Name=" << Name);
    ASTNameSpace *NameSpace = Context->NameSpaces.lookup(Name);
    if (!NameSpace) {
        Diag(diag::err_unref_namespace) << Name;
    }
    return NameSpace;
}

ASTNode *Sema::FindNode(ASTFunctionBase *FunctionBase) const {
    FLY_DEBUG_MESSAGE("Sema", "FindNode", Logger().Attr("FunctionBase", FunctionBase).End());
    if (FunctionBase->getKind() == ASTFunctionKind::FUNCTION) {
        return ((ASTFunction *) FunctionBase)->getNode();
    } else if (FunctionBase->getKind() == ASTFunctionKind::CLASS_METHOD) {
        return ((ASTClassMethod *) FunctionBase)->getClass()->getNode();
    } else {
        assert("Unknown Function Kind");
        return nullptr;
    }
}


ASTNode *Sema::FindNode(llvm::StringRef Name, ASTNameSpace *NameSpace) const {
    FLY_DEBUG_MESSAGE("Sema", "FindNode", Logger().Attr("Name", Name).Attr("NameSpace", NameSpace).End());
    ASTNode *Node = NameSpace->Nodes.lookup(Name);
    if (!Node) {
        Diag(diag::err_unref_node) << Name;
    }
    return Node;
}

ASTIdentity *Sema::FindIdentity(llvm::StringRef Name, ASTNameSpace *NameSpace) const {
    FLY_DEBUG_MESSAGE("Sema", "FindIdentity", Logger().Attr("Name", Name).Attr("NameSpace", NameSpace).End());
    ASTIdentity *Identity = NameSpace->Identities.lookup(Name);
    return Identity;
}

/**
 * Search a VarRef into declared Block's vars
 * If found set LocalVar
 * @param Block
 * @param Identifier
 * @return the found LocalVar
 */
ASTVar *Sema::FindLocalVar(ASTBlock *Block, llvm::StringRef Name) const {
    FLY_DEBUG_MESSAGE("Sema", "FindLocalVar", Logger().Attr("Name", Block).Attr("Identifier", Name).End());
    const auto &It = Block->getLocalVars().find(Name);
    if (It != Block->getLocalVars().end()) { // Search into this Block
        return It->getValue();
    } else if (Block->getParent()) { // search recursively into Parent Blocks to find the right Var definition
        if (Block->Parent->getKind() == ASTStmtKind::STMT_BLOCK)
            return FindLocalVar((ASTBlock *) Block->getParent(), Name);
    } else {
        llvm::SmallVector<ASTParam *, 8> Params = Block->getTop()->getParams();
        for (auto &Param : Params) {
            if (Param->getName() == Name) { // Search into ASTParam list
                return Param;
            }
        }
    }
    return nullptr;
}

ASTIdentityType *Sema::FindIdentityType(llvm::StringRef Name, ASTNameSpace *NameSpace) const {
    FLY_DEBUG_MESSAGE("Sema", "FindIdentityType", Logger().Attr("Name", Name).Attr("NameSpace", NameSpace).End());
    ASTIdentityType *IdentityType = NameSpace->getIdentityTypes().lookup(Name);
    return IdentityType;
}

ASTImport *Sema:: FindImport(ASTNode *Node, llvm::StringRef Name) {
    // Search into Node imports
    ASTImport *Import = Node->Imports.lookup(Name);
    return Import == nullptr ? Node->AliasImports.lookup(Name) : Import;
}
