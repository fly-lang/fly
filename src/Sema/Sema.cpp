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
#include "AST/ASTClassFunction.h"
#include "AST/ASTVarRef.h"
#include "Basic/Diagnostic.h"
#include "Basic/SourceLocation.h"
#include "Basic/Debug.h"

#include "llvm/ADT/StringRef.h"

using namespace fly;

Sema::Sema(DiagnosticsEngine &Diags) : Diags(Diags) {

}

SemaBuilder* Sema::CreateBuilder(DiagnosticsEngine &Diags) {
    Sema *S = new Sema(Diags);
    S->Builder = new SemaBuilder(*S);
    S->Resolver = new SemaResolver(*S);
    S->Validator = new SemaValidator(*S);
    return S->Builder;
}

ASTNameSpace *Sema::FindNameSpace(llvm::StringRef Name) const {
    FLY_DEBUG_MESSAGE("Sema", "FindNameSpace", "Name=" << Name);
    ASTNameSpace *NameSpace = Builder->Context->NameSpaces.lookup(Name);
    if (!NameSpace) {
        Diag(diag::err_unref_namespace) << Name;
    }
    return NameSpace;
}

ASTNameSpace *Sema::FindNameSpace(ASTFunctionBase *Base) const {
    if (Base->getKind() == ASTFunctionKind::FUNCTION) {
        return ((ASTFunction *) Base)->getNameSpace();
    } else if (Base->getKind() == ASTFunctionKind::CLASS_FUNCTION) {
        return ((ASTClassFunction *) Base)->getClass()->getNameSpace();
    } else {
        assert("Unknown Function Kind");
    }
}

ASTNode *Sema::FindNode(ASTFunctionBase *Base) const {
    if (Base->getKind() == ASTFunctionKind::FUNCTION) {
        return ((ASTFunction *) Base)->getNode();
    } else if (Base->getKind() == ASTFunctionKind::CLASS_FUNCTION) {
        return ((ASTClassFunction *) Base)->getClass()->getNode();
    } else {
        assert("Unknown Function Kind");
    }
}


ASTNode *Sema::FindNode(llvm::StringRef Name, ASTNameSpace *NameSpace) const {
    FLY_DEBUG_MESSAGE("Sema", "FindNode", "Name=" << Name);
    ASTNode *Node = NameSpace->Nodes.lookup(Name);
    if (!Node) {
        Diag(diag::err_unref_node) << Name;
    }
    return Node;
}

ASTClass *Sema::FindClass(llvm::StringRef Name, ASTNameSpace *NameSpace) const {
    FLY_DEBUG_MESSAGE("Sema", "FindClass", "Name=" << Name);
    ASTClass *Class = NameSpace->Classes.lookup(Name);
    if (!Class) {
        Diag(diag::err_unref_node) << Name;
    }
    return Class;
}

/**
 * Search a VarRef into declared Block's vars
 * If found set LocalVar
 * @param Block
 * @param LocalVar
 * @param VarRef
 * @return the found LocalVar
 */
ASTLocalVar *Sema::FindVarDef(ASTBlock *Block, ASTVarRef *VarRef) const {
    FLY_DEBUG_MESSAGE("Sema", "FindVarDef", "VarRef=" << VarRef->str());
    const auto &It = Block->getLocalVars().find(VarRef->getName());
    if (It != Block->getLocalVars().end()) { // Search into this Block
        FLY_DEBUG_MESSAGE("Sema", "FindVarDef", "Found=" << It->getValue()->str());
        return It->getValue();
    } else if (Block->getParent()) { // Traverse Parent Block to find the right VarDeclStmt
        if (Block->Parent->getKind() == StmtKind::STMT_BLOCK)
            return FindVarDef((ASTBlock *) Block->getParent(), VarRef);
    }
    return nullptr;
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

