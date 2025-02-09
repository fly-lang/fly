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
#include "Sym/SymTable.h"
#include "Sym/SymNameSpace.h"
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

#include <Sym/SymClass.h>
#include <Sym/SymEnum.h>
#include <Sym/SymGlobalVar.h>
#include <Sym/SymModule.h>
#include <Sym/SymType.h>

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

bool SemaValidator::CheckDuplicateVars(const llvm::StringMap<SymGlobalVar *> &Vars, ASTVar *Var) {
	SymGlobalVar *DuplicateVar = Vars.lookup(Var->getName());
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

bool SemaValidator::CheckCommentParams(SymComment *Comment, const llvm::SmallVector<ASTVar *, 8> &Params) {
	// TODO
}

bool SemaValidator::CheckCommentReturn(SymComment *Comment, ASTTypeRef *ReturnType) {
	// TODO
}

bool SemaValidator::CheckCommentFail(SymComment *Comment) {
	// TODO
}

bool SemaValidator::CheckExpr(ASTExpr *Expr) {
    if (!Expr->getTypeRef()) {
        if (DiagEnabled)
            S.Diag(Expr->getLocation(), diag::err_expr_type_miss);
        return false;
    }
    return true;
}

bool SemaValidator::CheckEqualTypes(SymType *Type1, SymType *Type2) {
    if (Type1->getKind() == Type2->getKind()) {
        if (Type1->isArray()) {
        	SymTypeArray *ArrayType1 = static_cast<SymTypeArray *>(Type1);
        	SymTypeArray *ArrayType2 = static_cast<SymTypeArray *>(Type2);
            return CheckEqualTypes(ArrayType1->getType(), ArrayType2->getType());
        }
        if (Type1->isClass()) {
	        SymClass *ClassType1 = static_cast<SymClass *>(Type1);
	        SymClass *ClassType2 = static_cast<SymClass *>(Type2);
	        return ClassType1->getId() == ClassType2->getId();
        }
        if (Type1->isEnum()) {
	        SymEnum *EnumType1 = static_cast<SymEnum *>(Type1);
	        SymEnum *EnumType2 = static_cast<SymEnum *>(Type2);
        	return EnumType1->getId() == EnumType2->getId();
        }
        return true;
    }

    return false;
}

bool SemaValidator::CheckConvertibleTypes(SymType *FromType, SymType *ToType) {
    assert(FromType && "FromType cannot be null");
    assert(ToType && "ToType cannot be null");

	// Check Integer Type
    if (FromType->isInteger() && ToType->isInteger()) {
	    SymTypeInt *FromIntegerType = static_cast<SymTypeInt *>(FromType);
	    SymTypeInt *ToIntegerType = static_cast<SymTypeInt *>(ToType);
	    return FromIntegerType->getIntKind() <= ToIntegerType->getIntKind() ||
	           FromIntegerType->isSigned() == ToIntegerType->isSigned();
    }

	// Check Floating Point Type
    if (FromType->isFloatingPoint() && ToType->isFloatingPoint()) {
	    SymTypeFP *FromFloatingType = static_cast<SymTypeFP *>(FromType);
	    SymTypeFP *ToFloatingType = static_cast<SymTypeFP *>(ToType);
	    return FromFloatingType->getFPKind() <= ToFloatingType->getFPKind();
    }

	// Check Array Type
    if (FromType->isArray() && ToType->isArray()) {
    	SymTypeArray *FromArrayType = static_cast<SymTypeArray *>(FromType);
    	SymTypeArray *ToArrayType = static_cast<SymTypeArray *>(ToType);
    	return CheckConvertibleTypes(FromArrayType->getType(), ToArrayType->getType());
    }

    // Check Enum name is equals
    if (FromType->isEnum() && ToType->isEnum()) {
    	SymEnum *FromEnumType = static_cast<SymEnum *>(FromType);
    	SymEnum *ToEnumType = static_cast<SymEnum *>(ToType);
    	return CheckInheritance(FromEnumType, ToEnumType);
    }

    // Check Class Inheritance
    if (FromType->isClass() && ToType->isClass()) {
    	SymClass *FromClassType = static_cast<SymClass *>(FromType);
    	SymClass *ToClassType = static_cast<SymClass *>(ToType);
    	return CheckInheritance(FromClassType, ToClassType);
    }

	// Check Void Type
    if (FromType->isBool() && ToType->isBool()) {
        return true;
    }

    if (FromType->isString() && ToType->isString()) {
	    return true;
    }

	if (FromType->isChar() && ToType->isChar()) {
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

bool SemaValidator::CheckInheritance(SymClass *TheClass, SymClass *SuperClass) {
	// Check if TheClass is equals to SuperClass
	if (TheClass->getId() == SuperClass->getId()) {
		return true;
	}

	// Check if TheClass is a subclass of SuperClass
	for (auto SuperClassEntry : TheClass->getSuperClasses()) {
		if (CheckInheritance(SuperClass, SuperClassEntry.getValue())) {
			return true;
		}
	}

	return false;
}

bool SemaValidator::CheckInheritance(SymEnum *TheEnum, SymEnum *SuperEnum) {
	// Check if TheClass is equals to SuperClass
	if (TheEnum->getId() == SuperEnum->getId()) {
		return true;
	}

	// Check if TheClass is a subclass of SuperClass
	for (auto SuperClassEntry : TheEnum->getSuperEnums()) {
		if (CheckInheritance(SuperClassEntry.getValue(), SuperEnum)) {
			return true;
		}
	}

	return false;
}

bool SemaValidator::CheckArithTypes(SymType *Type1, SymType *Type2) {
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

bool SemaValidator::CheckLogicalTypes(SymType *Type1, SymType *Type2) {
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
