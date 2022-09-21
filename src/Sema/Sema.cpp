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
bool Sema::CheckDuplicatedLocalVars(ASTStmt *Stmt, ASTLocalVar *LocalVar) {
    if (Stmt->getKind() == STMT_BLOCK) {
        ASTBlock *Block = (ASTBlock *) Stmt;
        if (Block->LocalVars.find(LocalVar->getName()) != Block->LocalVars.end()) {
            Diag(LocalVar->getLocation(), diag::err_conflict_vardecl) << LocalVar->getName();
            return true;
        }
        return Block->Parent && CheckDuplicatedLocalVars(Stmt->getParent(), LocalVar);
    }
}

bool Sema::CheckUndef(ASTBlock *Block, ASTVarRef *VarRef) {
    if (Block->UndefVars.find(VarRef->getName()) != Block->UndefVars.end()) {
        Diag(VarRef->getLocation(), diag::err_undef_var) << VarRef->getName();
        return false;
    }
    return true;
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

bool Sema::CheckExpr(ASTExpr *Expr) {
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

bool Sema::CheckMacroType(ASTType *Type, MacroTypeKind Kind) {
    if (Type->getMacroKind() != Kind) {
        Diag(Type->getLocation(), diag::err_sema_macro_type) << Type->printMacroType();
        return false;
    }

    return true;
}

bool Sema::CheckConvertibleTypes(ASTType *FromType, ASTType *ToType) {
    assert(FromType && "FromType cannot be null");
    assert(FromType && "ToType cannot be null");

    // Simplest Case: Types are equals
    if (FromType->getKind() == ToType->getKind()) {
        return true;
    }

    if (FromType->isInteger() && ToType->isInteger()) {
        switch (FromType->getKind()) { // You can always convert from low integer to high int
            // Signed Integer

            case TYPE_SHORT:
                return ToType->getKind() != TYPE_BYTE && ToType->getKind() != TYPE_USHORT;
            case TYPE_INT:
                return ToType->getKind() != TYPE_BYTE
                && ToType->getKind() != TYPE_SHORT && ToType->getKind() != TYPE_USHORT
                && ToType->getKind() != TYPE_UINT;
            case TYPE_LONG:
                return ToType->getKind() != TYPE_BYTE
                && ToType->getKind() != TYPE_SHORT && ToType->getKind() != TYPE_USHORT
                && ToType->getKind() != TYPE_INT && ToType->getKind() != TYPE_UINT
                && ToType->getKind() != TYPE_ULONG;

            // Unsigned Integer

            case TYPE_BYTE:
                return true;
            case TYPE_USHORT:
                return ToType->getKind() != TYPE_BYTE && ToType->getKind() != TYPE_SHORT;
            case TYPE_UINT:
                return ToType->getKind() == TYPE_UINT || ToType->getKind() == TYPE_LONG
                || ToType->getKind() == TYPE_ULONG;
            case TYPE_ULONG:
                return ToType->getKind() == TYPE_ULONG;
        }
    } else if (FromType->isFloatingPoint() && ToType->isFloatingPoint()) {
        switch (FromType->getKind()) {
            case TYPE_FLOAT:
                return true;
            case TYPE_DOUBLE:
                return ToType->getKind() == TYPE_DOUBLE;
        }
    } else if (FromType->isClass()) {
        // Check Inheritance
    }

    Diag(FromType->getLocation(), diag::err_sema_types_convert)
        << FromType->print()
        << ToType->print();
    return false;
}

bool Sema::CheckArithTypes(const SourceLocation &Loc, ASTType *Type1, ASTType *Type2) {
    if ((Type1->getMacroKind() == MACRO_TYPE_INTEGER || Type1->getMacroKind() == MACRO_TYPE_FLOATING_POINT) &&
        (Type2->getMacroKind() == MACRO_TYPE_INTEGER || Type2->getMacroKind() == MACRO_TYPE_FLOATING_POINT)) {
        return true;
    }

    Diag(Loc, diag::err_sema_types_arithmetics)
            << Type1->print()
            << Type2->print();
    return false;
}

bool Sema::CheckLogicalTypes(const SourceLocation &Loc, ASTType *Type1, ASTType *Type2) {
    if (Type1->getKind() == TYPE_BOOL && Type2->getKind() == TYPE_BOOL) {
        return true;
    }

    Diag(Loc, diag::err_sema_types_logical)
            << Type1->print()
            << Type2->print();
    return false;
}
