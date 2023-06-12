//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaValidator.cpp - The Sema Validator
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/Sema.h"
#include "Sema/SemaValidator.h"
#include "AST/ASTContext.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTNode.h"
#include "AST/ASTImport.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunctionBase.h"
#include "AST/ASTParams.h"
#include "AST/ASTBlock.h"
#include "AST/ASTValue.h"
#include "AST/ASTVarAssign.h"
#include "AST/ASTVarRef.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"

using namespace fly;

SemaValidator::SemaValidator(Sema &S) : S(S) {

}

/**
 * Check if this param name is already declared
 * @param Params
 * @param Param
 * @return
 */
bool SemaValidator::CheckDuplicateParams(ASTParams *Params, ASTParam *Param) {
    for (ASTParam *P : Params->getList()) {
        if (P->getName() == Param->getName()) {
            S.Diag(Param->getLocation(), diag::err_conflict_params) << Param->getName();
            return false;
        }
    }
    return true;
}

/**
 * Check if this var name is already declared
 * @param Block
 * @param Var
 * @return
 */
bool SemaValidator::CheckDuplicateLocalVars(ASTStmt *Stmt, llvm::StringRef VarName) {
    if (Stmt->getKind() != ASTStmtKind::STMT_BLOCK) {
        // Error: need stmt block, cannot search duplicate var
        return true;
    }

    ASTBlock *Block = (ASTBlock *) Stmt;
    ASTLocalVar *DuplicateVar = Block->getLocalVars().lookup(VarName);
    if (DuplicateVar != nullptr) {
        S.Diag(DuplicateVar->getLocation(), diag::err_conflict_vardecl) << DuplicateVar->getName();
        return false;
    }

    // Check with parents
    if (Block->getParent() != nullptr) {
        return CheckDuplicateLocalVars(Block->getParent(), VarName);
    }

    return true;
}

bool SemaValidator::CheckUninitialized(ASTBlock *Block, ASTVarRef *VarRef) {
    if (VarRef->isLocalVar() && Block->getUnInitVars().lookup(VarRef->getDef()->getName())) {
        S.Diag(VarRef->getLocation(), diag::err_sema_uninit_var) << VarRef->print();
        return false;
    }
    return true;
}

/**
 * Check Name and Alias on ASTImport
 * @param Node
 * @param Import
 * @return
 */
bool SemaValidator::CheckImport(ASTNode *Node, ASTImport *Import) {
    // Error: Empty Import
    if (Import->getName().empty()) {
        S.Diag(Import->getLocation(), diag::err_import_undefined);
        return false;
    }

    // Error: name is equals to the current ASTNode namespace
    if (Import->getName() == Node->getNameSpace()->getName()) {
        S.Diag(Import->getLocation(), diag::err_import_conflict_namespace) << Import->getName();
        return false;
    }

    // Error: alias is equals to the current ASTNode namespace
    if (Import->getAlias() == Node->getNameSpace()->getName()) {
        S.Diag(Import->getAliasLocation(), diag::err_alias_conflict_namespace) << Import->getAlias();
        return false;
    }

    return true;
}

bool SemaValidator::CheckExpr(ASTExpr *Expr) {
    if (!Expr->getType()) {
        S.Diag(Expr->getLocation(), diag::err_expr_type_miss);
        return false;
    }
    return true;
}

bool SemaValidator::isEquals(ASTType *Type1, ASTType *Type2) {
    if (Type1->getMacroKind() == Type2->getMacroKind()) {
        if (Type1->getMacroKind() == ASTMacroTypeKind::MACRO_TYPE_ARRAY) {
            return isEquals(((ASTArrayType *) Type1)->getType(), ((ASTArrayType *) Type2)->getType());
        } else if (Type1->getMacroKind() == ASTMacroTypeKind::MACRO_TYPE_CLASS) {
            return ((ASTClassType *) Type1)->getName() == ((ASTClassType *) Type2)->getName();
        }
        return true;
    }

    return false;
}

bool SemaValidator::CheckMacroType(ASTType *Type, ASTMacroTypeKind Kind) {
    if (Type->getMacroKind() != Kind) {
        S.Diag(Type->getLocation(), diag::err_sema_macro_type) << Type->printMacroType();
        return false;
    }

    return true;
}

bool SemaValidator::CheckConvertibleTypes(ASTType *FromType, ASTType *ToType) {
    assert(FromType && "FromType cannot be null");
    assert(FromType && "ToType cannot be null");

    // Simplest Case: Types are equals
    if (FromType->getKind() == ToType->getKind()) {
        return true;
    }

    if (FromType->isInteger() && ToType->isInteger()) {
        switch (FromType->getKind()) { // You can always convert from low integer to high int
            // Signed Integer

            case ASTTypeKind::TYPE_SHORT:
                return ToType->getKind() != ASTTypeKind::TYPE_BYTE && ToType->getKind() != ASTTypeKind::TYPE_USHORT;
            case ASTTypeKind::TYPE_INT:
                return ToType->getKind() != ASTTypeKind::TYPE_BYTE
                       && ToType->getKind() != ASTTypeKind::TYPE_SHORT && ToType->getKind() != ASTTypeKind::TYPE_USHORT
                       && ToType->getKind() != ASTTypeKind::TYPE_UINT;
            case ASTTypeKind::TYPE_LONG:
                return ToType->getKind() != ASTTypeKind::TYPE_BYTE
                       && ToType->getKind() != ASTTypeKind::TYPE_SHORT && ToType->getKind() != ASTTypeKind::TYPE_USHORT
                       && ToType->getKind() != ASTTypeKind::TYPE_INT && ToType->getKind() != ASTTypeKind::TYPE_UINT
                       && ToType->getKind() != ASTTypeKind::TYPE_ULONG;

                // Unsigned Integer

            case ASTTypeKind::TYPE_BYTE:
                return true;
            case ASTTypeKind::TYPE_USHORT:
                return ToType->getKind() != ASTTypeKind::TYPE_BYTE && ToType->getKind() != ASTTypeKind::TYPE_SHORT;
            case ASTTypeKind::TYPE_UINT:
                return ToType->getKind() == ASTTypeKind::TYPE_UINT || ToType->getKind() == ASTTypeKind::TYPE_LONG
                       || ToType->getKind() == ASTTypeKind::TYPE_ULONG;
            case ASTTypeKind::TYPE_ULONG:
                return ToType->getKind() == ASTTypeKind::TYPE_ULONG;
        }
    } else if (FromType->isFloatingPoint() && ToType->isFloatingPoint()) {
        switch (FromType->getKind()) {
            case ASTTypeKind::TYPE_FLOAT:
                return true;
            case ASTTypeKind::TYPE_DOUBLE:
                return ToType->getKind() == ASTTypeKind::TYPE_DOUBLE;
        }
    } else if (FromType->isClass()) {
        // Check Inheritance
    }

    S.Diag(FromType->getLocation(), diag::err_sema_types_convert)
            << FromType->print()
            << ToType->print();
    return false;
}

bool SemaValidator::CheckArithTypes(const SourceLocation &Loc, ASTType *Type1, ASTType *Type2) {
    if ((Type1->getMacroKind() == ASTMacroTypeKind::MACRO_TYPE_INTEGER || Type1->getMacroKind() == ASTMacroTypeKind::MACRO_TYPE_FLOATING_POINT) &&
        (Type2->getMacroKind() == ASTMacroTypeKind::MACRO_TYPE_INTEGER || Type2->getMacroKind() == ASTMacroTypeKind::MACRO_TYPE_FLOATING_POINT)) {
        return true;
    }

    S.Diag(Loc, diag::err_sema_types_arithmetics)
            << Type1->print()
            << Type2->print();
    return false;
}

bool SemaValidator::CheckLogicalTypes(const SourceLocation &Loc, ASTType *Type1, ASTType *Type2) {
    if (Type1->getKind() == ASTTypeKind::TYPE_BOOL && Type2->getKind() == ASTTypeKind::TYPE_BOOL) {
        return true;
    }

    S.Diag(Loc, diag::err_sema_types_logical)
            << Type1->print()
            << Type2->print();
    return false;
}
