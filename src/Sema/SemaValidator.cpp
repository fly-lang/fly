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
#include "Sema/SymTable.h"
#include "Sema/SemaNameSpace.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTModule.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
#include "AST/ASTImport.h"
#include "AST/ASTVar.h"
#include "AST/ASTVar.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTValue.h"
#include "AST/ASTTypeRef.h"
#include "Basic/Diagnostic.h"

#include <AST/ASTExpr.h>
#include <Sema/SemaClassType.h>
#include <Sema/SemaEnumType.h>
#include <Sema/SemaGlobalVar.h>
#include <Sema/SemaModule.h>
#include <Sema/SemaType.h>

using namespace fly;

SemaValidator::SemaValidator(Sema &S) : S(S) {

}

bool SemaValidator::CheckDuplicateModules(ASTModule *Module) {
	// Check Duplicate Module Names
	for (auto ModuleEntry : S.getSymTable().getModules()) {
		ASTModule * AST = ModuleEntry.getSecond()->getAST();
		if (AST->getId() != Module->getId() && AST->getName() == Module->getName()) {
			S.Diag(diag::err_sema_module_duplicated) << AST->getName();
			return false;
		}
	}
	return true;
}

bool SemaValidator::CheckDuplicateVars(const llvm::StringMap<SemaGlobalVar *> &Vars, ASTVar *Var) {
	SemaGlobalVar *DuplicateVar = Vars.lookup(Var->getName());
	if (DuplicateVar) { // This NameSpace already contains this GlobalVar
		S.Diag(DuplicateVar->getAST()->getLocation(), diag::err_duplicate_gvar) << DuplicateVar->getAST()->getName();
		return false;
	}
	return true;
}

// bool SemaValidator::CheckDuplicateIdentities(const llvm::StringMap<SymIdentity *> &Identities, ASTIdentity * Identity) {
// 	SymIdentity *DuplicateIdentity = Identities.lookup(Identity->getName());
// 	if (DuplicateIdentity) { // This NameSpace already contains this Identity
// 		S.Diag(Identity->getLocation(), diag::err_duplicate_identity) << Identity->getName();
// 		return false;
// 	}
//
// 	return true;
// }

/**
 * Check if this param name is already declared
 * @param Params
 * @param Param
 * @return
 */
bool SemaValidator::CheckDuplicateParams(llvm::SmallVector<ASTVar *, 8> Params, ASTVar *Param) {
    for (ASTVar *P : Params) {
        if (P->getName() == Param->getName()) {
            if (DiagEnabled)
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
    if (Stmt->getStmtKind() != ASTStmtKind::STMT_BLOCK) {
        // Error: need stmt block, cannot search duplicate var
        return true;
    }

    ASTBlockStmt *Block = (ASTBlockStmt *) Stmt;
    ASTVar *DuplicateVar = Block->getLocalVars().lookup(VarName);
    if (DuplicateVar != nullptr) {
        if (DiagEnabled)
            S.Diag(DuplicateVar->getLocation(), diag::err_conflict_vardecl) << DuplicateVar->getName();
        return false;
    }

    // Check with parents
    if (Block->getParent() != nullptr) {
        return CheckDuplicateLocalVars(Block->getParent(), VarName);
    }

    return true;
}

bool SemaValidator::CheckCommentParams(SemaComment *Comment, const llvm::SmallVector<ASTVar *, 8> &Params) {
	// TODO
	return true;
}

bool SemaValidator::CheckCommentReturn(SemaComment *Comment, ASTTypeRef *ReturnType) {
	// TODO
	return true;
}

bool SemaValidator::CheckCommentFail(SemaComment *Comment) {
	// TODO
	return true;
}

bool SemaValidator::CheckExpr(ASTExpr *Expr) {
    if (!Expr->getType()) {
        if (DiagEnabled)
            S.Diag(Expr->getLocation(), diag::err_expr_type_miss);
        return false;
    }
    return true;
}

bool SemaValidator::CheckEqualTypes(SemaType *Type1, SemaType *Type2) {
    if (Type1->getKind() == Type2->getKind()) {
        if (Type1->isArray()) {
        	SemaArrayType *ArrayType1 = static_cast<SemaArrayType *>(Type1);
        	SemaArrayType *ArrayType2 = static_cast<SemaArrayType *>(Type2);
            return CheckEqualTypes(ArrayType1->getType(), ArrayType2->getType());
        }

        return Type1->getId() == Type2->getId();
    }

    return false;
}

bool SemaValidator::CheckConvertibleTypes(SemaType *FromType, SemaType *ToType) {
    assert(FromType && "FromType cannot be null");
    assert(ToType && "ToType cannot be null");

	// Check Integer Type
    if (FromType->isInteger() && ToType->isInteger()) {
	    SemaIntType *FromIntegerType = static_cast<SemaIntType *>(FromType);
	    SemaIntType *ToIntegerType = static_cast<SemaIntType *>(ToType);
	    return FromIntegerType->getIntKind() <= ToIntegerType->getIntKind() ||
	           FromIntegerType->isSigned() == ToIntegerType->isSigned();
    }

	// Check Floating Point Type
    if (FromType->isFloatingPoint() && ToType->isFloatingPoint()) {
	    SemaFloatType *FromFloatingType = static_cast<SemaFloatType *>(FromType);
	    SemaFloatType *ToFloatingType = static_cast<SemaFloatType *>(ToType);
	    return FromFloatingType->getFPKind() <= ToFloatingType->getFPKind();
    }

	// Check Array Type
    if (FromType->isArray() && ToType->isArray()) {
    	SemaArrayType *FromArrayType = static_cast<SemaArrayType *>(FromType);
    	SemaArrayType *ToArrayType = static_cast<SemaArrayType *>(ToType);
    	return CheckConvertibleTypes(FromArrayType->getType(), ToArrayType->getType());
    }

    // Check Enum name is equals
    if (FromType->isEnum() && ToType->isEnum()) {
    	SemaEnumType *FromEnumType = static_cast<SemaEnumType *>(FromType);
    	SemaEnumType *ToEnumType = static_cast<SemaEnumType *>(ToType);
    	return CheckInheritance(FromEnumType, ToEnumType);
    }

    // Check Class Inheritance
    if (FromType->isClass() && ToType->isClass()) {
    	SemaClassType *FromClassType = static_cast<SemaClassType *>(FromType);
    	SemaClassType *ToClassType = static_cast<SemaClassType *>(ToType);
    	return CheckInheritance(FromClassType, ToClassType);
    }

	// Check Void Type
    if (FromType->isBool() && ToType->isBool()) {
        return true;
    }

    if (FromType->isString() && ToType->isString()) {
	    return true;
    }

    if (FromType->isError() && ToType->isError()) {
    	return true;
    }

	if (FromType->isVoid() && ToType->isVoid()) {
		return true;
	}

    return false;
}

bool SemaValidator::CheckInheritance(SemaClassType *TheClass, SemaClassType *SuperClass) {
	// Check if TheClass is equals to SuperClass
	if (TheClass->getId() == SuperClass->getId()) {
		return true;
	}

	// Check if TheClass is a subclass of SuperClass
	for (auto &SuperClassEntry : TheClass->getSuperClasses()) {
		if (CheckInheritance(SuperClass, SuperClassEntry.getValue())) {
			return true;
		}
	}

	return false;
}

bool SemaValidator::CheckInheritance(SemaEnumType *TheEnum, SemaEnumType *SuperEnum) {
	// Check if TheClass is equals to SuperClass
	if (TheEnum->getId() == SuperEnum->getId()) {
		return true;
	}

	// Check if TheClass is a subclass of SuperClass
	for (auto &SuperClassEntry : TheEnum->getSuperEnums()) {
		if (CheckInheritance(SuperClassEntry.getValue(), SuperEnum)) {
			return true;
		}
	}

	return false;
}

bool SemaValidator::CheckArithTypes(SemaType *Type1, SemaType *Type2) {
	// Check if one of the types is an integer
	if (Type1->isInteger() && Type2->isInteger()) {
        return true;
    }

	// Check if one of the types is a floating point
	if (Type1->isFloatingPoint() && Type2->isFloatingPoint()) {
		return true;
	}

	// Check if one of the types is a boolean
	if (Type1->isBool() && Type2->isBool()) {
		return true;
	}

    return false;
}

bool SemaValidator::CheckLogicalTypes(SemaType *Type1, SemaType *Type2) {
    if (Type1->isBool() && Type2->isBool()) {
        return true;
    }

    return false;
}

void SemaValidator::CheckNameEmpty(const SourceLocation &Loc,llvm::StringRef Name) {
    if (Name.empty()) {
        S.Diag(Loc, diag::err_sema_identifier_empty);
    }
}


bool SemaValidator::CheckIsValueExpr(ASTExpr *Expr) {
    if (Expr->getExprKind() == ASTExprKind::EXPR_VALUE) {
        return true;
    }
    return false;
}

bool SemaValidator::CheckVarRefExpr(ASTExpr *Expr) {
    if (Expr->getExprKind() == ASTExprKind::EXPR_VAR_REF) {
        return true;
    }
    return false;
}

bool SemaValidator::CheckValue(ASTValue *Value) {
	return true;
}
