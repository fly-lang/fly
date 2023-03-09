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
#include "AST/ASTClassVar.h"
#include "AST/ASTImport.h"
#include "AST/ASTClassFunction.h"
#include "AST/ASTNode.h"
#include "AST/ASTVarRef.h"
#include "Basic/Diagnostic.h"
#include "Basic/SourceLocation.h"
#include "Basic/Debuggable.h"
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

ASTNode *Sema::FindNode(ASTFunctionBase *FunctionBase) const {
    FLY_DEBUG_MESSAGE("Sema", "FindNode", Logger().Attr("FunctionBase", FunctionBase).End());
    if (FunctionBase->getKind() == ASTFunctionKind::FUNCTION) {
        return ((ASTFunction *) FunctionBase)->getNode();
    } else if (FunctionBase->getKind() == ASTFunctionKind::CLASS_FUNCTION) {
        return ((ASTClassFunction *) FunctionBase)->getClass()->getNode();
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

ASTClass *Sema::FindClass(llvm::StringRef ClassName, ASTNameSpace *NameSpace) const {
    FLY_DEBUG_MESSAGE("Sema", "FindClass", Logger().Attr("ClassName", ClassName).Attr("NameSpace", NameSpace).End());
    ASTClass *Class = NameSpace->Classes.lookup(ClassName);
    return Class;
}

/**
 * Search a VarRef into declared Block's vars
 * If found set LocalVar
 * @param Block
 * @param Identifier
 * @return the found LocalVar
 */
ASTLocalVar *Sema::FindLocalVar(ASTBlock *Block, ASTIdentifier *Identifier) const {
    FLY_DEBUG_MESSAGE("Sema", "FindLocalVar", Logger().Attr("Name", Block).Attr("Identifier", Identifier).End());
    const auto &It = Block->getLocalVars().find(Identifier->getName());
    if (It != Block->getLocalVars().end()) { // Search into this Block
        return It->getValue();
    } else if (Block->getParent()) { // Traverse Parent Block to find the right VarDeclStmt
        if (Block->Parent->getKind() == ASTStmtKind::STMT_BLOCK)
            return FindLocalVar((ASTBlock *) Block->getParent(), Identifier);
    }
    return nullptr;
}

ASTImport *Sema:: FindImport(ASTNode *Node, llvm::StringRef Name) {
    // Search into Node imports
    return Node->Imports.lookup(Name);
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
