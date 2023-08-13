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
#include "AST/ASTClass.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunctionBase.h"
#include "AST/ASTParams.h"
#include "AST/ASTBlock.h"
#include "AST/ASTVarRef.h"
#include "Basic/Diagnostic.h"

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
    if (Type1->getKind() == Type2->getKind()) {
        if (Type1->getKind() == ASTTypeKind::TYPE_ARRAY) {
            return isEquals(((ASTArrayType *) Type1)->getType(), ((ASTArrayType *) Type2)->getType());
        } else if (Type1->getKind() == ASTTypeKind::TYPE_IDENTITY) {
            return ((ASTClassType *) Type1)->getName() == ((ASTClassType *) Type2)->getName();
        }
        return true;
    }

    return false;
}

bool SemaValidator::CheckMacroType(ASTType *Type, ASTTypeKind Kind) {
    if (Type->getKind() != Kind) {
        S.Diag(Type->getLocation(), diag::err_sema_macro_type) << Type->printType();
        return false;
    }

    return true;
}

bool SemaValidator::CheckConvertibleTypes(ASTType *FromType, ASTType *ToType) {
    assert(FromType && "FromType cannot be null");
    assert(ToType && "ToType cannot be null");

    if (FromType->isBool() && ToType->isBool()) {
        return true;
    }

    else if (FromType->isInteger() && ToType->isInteger()) {
        ASTIntegerType *FromIntegerType = ((ASTIntegerType *) FromType);
        ASTIntegerType *ToIntegerType = ((ASTIntegerType *) ToType);
        return FromIntegerType->getSize() <= ToIntegerType->getSize() ||
            FromIntegerType->isSigned() == ToIntegerType->isSigned();
    }

    else if (FromType->isFloatingPoint() && ToType->isFloatingPoint()) {
        ASTFloatingPointType *FromFloatingType = ((ASTFloatingPointType *) FromType);
        ASTFloatingPointType *ToFloatingType = ((ASTFloatingPointType *) ToType);
        return FromFloatingType->getSize() <= ToFloatingType->getSize();
    }

    else if (FromType->isArray() && ToType->isArray()) {
        // FIXME
        return ((ASTArrayType *) FromType)->getType()->getKind() ==
               ((ASTArrayType *) ToType)->getType()->getKind();
    }

    else if (FromType->isIdentity() && ToType->isIdentity()) {
        ASTIdentityType *FromIdentityType = (ASTIdentityType *) FromType;
        ASTIdentityType *ToIdentityType = (ASTIdentityType *) ToType;

        // Check Enum name is equals
        if (FromIdentityType->isEnum() && ToIdentityType->isEnum()) {
            if (FromIdentityType->getDef()->getName() == ToIdentityType->getDef()->getName()) {
                return true;
            }
        }

        // Check Class Inheritance
        else if (FromIdentityType->isClass() && FromIdentityType->isClass()) {
            return CheckClassInheritance((ASTClassType *) FromIdentityType, (ASTClassType *) ToIdentityType);
        }
    }

    S.Diag(FromType->getLocation(), diag::err_sema_types_convert)
            << FromType->print()
            << ToType->print();
    return false;
}

bool SemaValidator::CheckSameTypes(const SourceLocation &Loc, ASTType *Type1, ASTType *Type2) {
    if (Type1->getKind() == Type2->getKind()) {
        return true;
    }

    S.Diag(Loc, diag::err_sema_types_operation)
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

bool SemaValidator::CheckClassInheritance(fly::ASTClassType *FromType, fly::ASTClassType *ToType) {
    const StringRef &FromName = FromType->getDef()->getName();
    const StringRef &ToName = ToType->getDef()->getName();
    if (FromName == ToName) {
        return true;
    } else {
        const SmallVector<ASTClassType *, 4> &SuperClasses = ((ASTClass *) FromType->getDef())->getSuperClasses();
        for (ASTClassType *SuperClass : SuperClasses) {
            if (CheckClassInheritance(SuperClass, ToType)) {
                return true;
            }
        }
    }
    return false;
}
