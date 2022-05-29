//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/Sema.cpp - GlobalVar Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/Sema.h"
#include "Sema/SemaBuilder.h"
#include "Sema/SemaResolver.h"
#include "AST/ASTContext.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTNode.h"
#include "AST/ASTImport.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunctionCall.h"
#include "AST/ASTParams.h"
#include "AST/ASTBlock.h"
#include "AST/ASTValue.h"
#include "AST/ASTVar.h"
#include "AST/ASTVarAssign.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"

using namespace fly;

Sema::Sema(DiagnosticsEngine &Diags) : Diags(Diags) {

}

SemaBuilder* Sema::Builder(DiagnosticsEngine &Diags) {
    Sema *S = new Sema(Diags);
    SemaBuilder *B = new SemaBuilder(*S);
    S->Resolver = new SemaResolver(*S, *B);
    return B;
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

/**
 * Check if this var is already declared
 * @param Block
 * @param Var
 * @return
 */
bool Sema::CheckDuplicatedLocalVars(ASTBlock *Block, ASTLocalVar *LocalVar) {
    if (Block->LocalVars.find(LocalVar->getName()) != Block->LocalVars.end()) {
        Diag(LocalVar->getLocation(), diag::err_conflict_vardecl) << LocalVar->getName();
        return true;
    }
    return Block->Parent && CheckDuplicatedLocalVars(Block->getParent(), LocalVar);
}

bool Sema::CheckUndefVar(ASTBlock *Block, ASTVarRef *VarRef) {
    if (Block->UndefVars.lookup(VarRef->getName())) {
        Diag(VarRef->getLocation(), diag::err_undef_var) << VarRef->getName();
        return false;
    }
    return true;
}

bool Sema::CheckOnCloseBlock(ASTBlock *Block) {

    return false;
}

bool Sema::CheckImport(ASTNode *Node, ASTImport *Import) {
    // Syntax Error Quote
    if (Import->Name.empty()) {
        Diag(Import->NameLocation, diag::err_import_undefined);
        return false;
    }

    if (Import->Name == Node->NameSpace->getName()) {
        Diag(Import->NameLocation, diag::err_import_conflict_namespace) << Import->Name;
        return false;
    }

    if (Import->Alias == Node->NameSpace->getName()) {
        Diag(Import->AliasLocation, diag::err_alias_conflict_namespace) << Import->Alias;
        return false;
    }

    return true;
}

bool Sema::Check(ASTExpr *Expr) {
    if (!Expr->getType()) {
        Diag(Expr->getLocation(), diag::err_expr_type_miss);
        return false;
    }
    return true;
}

bool Sema::isEquals(ASTParam *Param1, ASTParam *Param2) {
    return Param1 && Param2 &&
    Param1->getType() && Param2->getType() &&
    Param1->getType()->getKind() == Param2->getType()->getKind();
}

bool Sema::isTypeDerivate(ASTType *T1, ASTType *T2) {
    return T1 && T2 && T1->getKind() == T2->getKind();
}

bool Sema::VerifyValueType(ASTValueExpr *ValueExpr, ASTType *Type) {
    if (Type->isBool()) {
        return ValueExpr->getValue().getKind() == VALUE_BOOL;
    } else if (Type->isInteger()) {
        return ValueExpr->getValue().getKind() == VALUE_INTEGER;
    } else if (Type->isFloatingPoint()) {
        return ValueExpr->getValue().getKind() == VALUE_FLOATING_POINT;
    } else if (Type->isArray()) {
        return ValueExpr->getValue().getKind() == VALUE_ARRAY;
    } else if (Type->isClass()) {
        return ValueExpr->getValue().getKind() == VALUE_NULL;
    }
    assert("Unknown Value type");
}



