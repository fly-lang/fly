//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaValidator.cpp - semantic validation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaValidator.h"

#include "AST/ASTBinary.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
#include "AST/ASTImport.h"
#include "AST/ASTModule.h"
#include "AST/ASTValue.h"
#include "AST/ASTVar.h"
#include "Basic/Debug.h"
#include "Basic/Diagnostic.h"
#include "llvm/Support/Signals.h"

#include <AST/ASTExpr.h>
#include <Sema/SemaBuiltin.h>
#include <Sema/SemaClassType.h>
#include <Sema/SemaEnumType.h>
#include <Sema/SemaExpr.h>
#include <Sema/SemaNode.h>
#include <Sema/SemaType.h>
#include <Sema/SemaValue.h>

using namespace fly;

// ─── file-local helpers ──────────────────────────────────────────────────────

// Returns true for any type that lives in the numeric cast group (bool/int/float).
static bool isNumericGroup(const SemaType *T) {
    return T->isBool() || T->isInteger() || T->isFloat();
}

// Returns true when From is assignable / castable to To via inheritance rules.
// Handles class sub-typing and enum derivation; returns false for all other kinds.
static bool isCompatibleByInheritance(SemaType *From, SemaType *To) {
    if (From->isClass() && To->isClass())
        return static_cast<SemaClassType *>(From)->isDerivedOrEquals(
               static_cast<SemaClassType *>(To));
    if (From->isEnum() && To->isEnum())
        return static_cast<SemaEnumType *>(From)->isDerivedOrEquals(
               static_cast<SemaEnumType *>(To));
    return false;
}

// ─── SemaValidator ───────────────────────────────────────────────────────────

SemaValidator::SemaValidator(DiagnosticsEngine &Diags) : Diags(Diags) {
}

DiagnosticBuilder SemaValidator::Diag(const SourceLocation &Loc, unsigned DiagID) const {
	return Diags.Report(Loc, DiagID);
}

DiagnosticBuilder SemaValidator::Diag(unsigned DiagID) const {
	if (DebugLog && DiagID == diag::err_invalid_behavior)
		llvm::sys::PrintStackTrace(llvm::errs());
	return Diags.Report(DiagID);
}

void SemaValidator::CheckImport(const ASTImport &AST) {

	// Error: Empty Import
	if (AST.getNames().empty()) {
		Diag(AST.getLocation(), diag::err_sema_import_undefined);
	}
}

void SemaValidator::CheckCast(SemaExpr *From, SemaExpr *To) {
	if (!From || !To)
		return;

	SemaType *FromType = From->getType();
	SemaType *ToType   = To->getType();

	if (!FromType || !ToType)
		return;

	// Identity: same type is always valid
	if (FromType->isEquals(ToType))
		return;

	// Numeric group: bool / integer / float are mutually castable
	if (isNumericGroup(FromType) && isNumericGroup(ToType)) {
		// Warn on lossy narrowing
		if (FromType->isFloat() && ToType->isInteger()) {
			Diag(diag::warn_sema_cast_lossy)
				<< FromType->getName() << ToType->getName();
		} else if (FromType->isFloat() && ToType->isFloat()) {
			SemaFloatType *FromFP = static_cast<SemaFloatType *>(FromType);
			SemaFloatType *ToFP   = static_cast<SemaFloatType *>(ToType);
			if (static_cast<unsigned>(FromFP->getFloatKind()) >
			    static_cast<unsigned>(ToFP->getFloatKind()))
				Diag(diag::warn_sema_cast_lossy)
					<< FromType->getName() << ToType->getName();
		} else if (FromType->isInteger() && ToType->isInteger()) {
			SemaIntType *FromInt = static_cast<SemaIntType *>(FromType);
			SemaIntType *ToInt   = static_cast<SemaIntType *>(ToType);
			if (static_cast<unsigned>(FromInt->getIntKind()) >
			    static_cast<unsigned>(ToInt->getIntKind()))
				Diag(diag::warn_sema_cast_lossy)
					<< FromType->getName() << ToType->getName();
		}
		return;
	}

	// Same-kind identity casts (string, complex, array)
	if (FromType->isString()  && ToType->isString())  return;
	if (FromType->isComplex() && ToType->isComplex()) return;
	if (FromType->isArray()   && ToType->isArray())   return;

	// Class / enum: allow upcasting or same derivation
	if (isCompatibleByInheritance(FromType, ToType))
		return;

	// All other combinations are invalid
	Diag(diag::err_sema_invalid_cast)
		<< FromType->getName() << ToType->getName();
}

bool SemaValidator::CheckDuplicateParams(llvm::SmallVector<ASTVar *, 8> Params, ASTVar *Param) {
    for (ASTVar *P : Params) {
        if (P->getName() == Param->getName())
            return false;
    }
    return true;
}

bool SemaValidator::CheckDuplicateLocalVars(ASTStmt *Stmt, llvm::StringRef VarName) {
    if (Stmt->getStmtKind() != ASTStmtKind::STMT_BLOCK)
        return true;

    ASTBlockStmt *Block = (ASTBlockStmt *) Stmt;
    if (Block->getParent() != nullptr)
        return CheckDuplicateLocalVars(Block->getParent(), VarName);

    return true;
}

bool SemaValidator::CheckExpr(SemaExpr *Expr) {
    if (!Expr || !Expr->getType())
        return false;
    return true;
}

bool SemaValidator::CheckEqualTypes(SemaType *Type1, SemaType *Type2) {
    if (Type1->getKind() == Type2->getKind()) {
        if (Type1->isArray()) {
        	SemaArrayType *ArrayType1 = static_cast<SemaArrayType *>(Type1);
        	SemaArrayType *ArrayType2 = static_cast<SemaArrayType *>(Type2);
            return CheckEqualTypes(ArrayType1->getElementType(), ArrayType2->getElementType());
        }
        return Type1->getId() == Type2->getId();
    }
    return false;
}

bool SemaValidator::CheckConvertibleTypes(SemaType *FromType, SemaType *ToType) {
    assert(FromType && "FromType cannot be null");
    assert(ToType && "ToType cannot be null");

	// Identical types are always convertible
	if (FromType->isEquals(ToType))
		return true;

	// Integer widening / same-signedness
    if (FromType->isInteger() && ToType->isInteger()) {
	    SemaIntType *FromInt = static_cast<SemaIntType *>(FromType);
	    SemaIntType *ToInt   = static_cast<SemaIntType *>(ToType);
	    return FromInt->getIntKind() <= ToInt->getIntKind() ||
	           FromInt->isSigned() == ToInt->isSigned();
    }

	// Float widening
    if (FromType->isFloat() && ToType->isFloat()) {
	    SemaFloatType *FromFP = static_cast<SemaFloatType *>(FromType);
	    SemaFloatType *ToFP   = static_cast<SemaFloatType *>(ToType);
	    return FromFP->getFloatKind() <= ToFP->getFloatKind();
    }

	// Array element-wise convertibility
    if (FromType->isArray() && ToType->isArray()) {
    	SemaArrayType *FromArr = static_cast<SemaArrayType *>(FromType);
    	SemaArrayType *ToArr   = static_cast<SemaArrayType *>(ToType);
    	return CheckConvertibleTypes(FromArr->getElementType(), ToArr->getElementType());
    }

    // Class / enum: allow upcasting or same derivation
    return isCompatibleByInheritance(FromType, ToType);
}

bool SemaValidator::CheckInheritance(SemaClassType *ClassType, SemaClassType *SuperClassType) {
	if (ClassType->getId() == SuperClassType->getId())
		return true;
	// Recurse on each base asking "is this base (or an ancestor of it) SuperClassType?".
	// The arguments must stay (Base, Super) — passing (Super, Base) only ever matched at
	// depth 1, so a 2+-level upcast (ASTNumberValue → ASTExpr) was wrongly rejected.
	for (auto &Entry : ClassType->getBaseClasses()) {
		if (CheckInheritance(Entry, SuperClassType))
			return true;
	}
	return false;
}

bool SemaValidator::CheckInheritance(SemaEnumType *EnumType, SemaEnumType *SuperEnumType) {
	if (EnumType->getId() == SuperEnumType->getId())
		return true;
	for (auto &SuperClassEntry : EnumType->getBaseEnums()) {
		if (CheckInheritance(SuperClassEntry.getValue(), SuperEnumType))
			return true;
	}
	return false;
}

bool SemaValidator::CheckBinary(ASTBinary &AST, SemaExpr *LeftSema, SemaExpr *RightSema) {
	if (!LeftSema || !RightSema)
		return false;

	SemaType *LeftType  = LeftSema->getType();
	SemaType *RightType = RightSema->getType();

	// Allow comparisons involving a null literal (null value has no type).
	if (AST.isCompare() && (!LeftType || !RightType))
		return true;

	// Allow assignment of unset/null (RightType == nullptr); CheckAssignment handles it.
	if (AST.isAssign() && LeftType && !RightType)
		return CheckAssignment(AST.getLocation(), LeftType, RightSema);

	if (!LeftType || !RightType)
		return false;

	auto diagnoseTypes = [&]() {
		Diag(AST.getLocation(), diag::err_sema_types_operation)
			<< LeftType->getName() << RightType->getName();
	};

	// Arithmetic: numbers or strings (+ only for strings)
	if (AST.isArith()) {
		if (LeftType->isNumber() && RightType->isNumber()) return true;
		if (LeftType->isString() && RightType->isString()) return true;
		diagnoseTypes();
		return false;
	}

	// Comparison: same-kind scalars, classes, enums
	if (AST.isCompare()) {
		if (LeftType->isBool()   && RightType->isBool())   return true;
		if (LeftType->isNumber() && RightType->isNumber()) return true;
		if (LeftType->isString() && RightType->isString()) return true;
		if (LeftType->isClass()  && RightType->isClass())  return true;
		if (LeftType->isEnum()   && RightType->isEnum())   return true;
		diagnoseTypes();
		return false;
	}

	// Logical: booleans only
	if (AST.isLogic()) {
		if (LeftType->isBool() && RightType->isBool()) return true;
		diagnoseTypes();
		return false;
	}

	// Assignment: compatible types
	if (AST.isAssign()) {
		return CheckAssignment(AST.getLocation(), LeftType, RightSema);
	}

	diagnoseTypes();
	return false;
}

bool SemaValidator::CheckAssignment(const SourceLocation &Loc, SemaType *LhsType, SemaExpr *RhsExpr) {
	if (!LhsType || !RhsExpr) return true;
	SemaType *RhsType = RhsExpr->getType();

	auto diagnose = [&]() {
		if (RhsType)
			Diag(Loc, diag::err_sema_type_mismatch) << RhsType->getName() << LhsType->getName();
	};

	if (LhsType->isBool()) return true;

	if (LhsType->isNumber()) {
		// Allow class → long: class pointers are pointer-sized (i64).
		if (static_cast<SemaNumberType *>(LhsType)->isInteger() &&
		    static_cast<SemaIntType *>(LhsType)->getIntKind() == SemaIntTypeKind::TYPE_LONG &&
		    RhsType && RhsType->isClass())
			return true;
		if (!RhsType || !RhsType->isNumber()) { diagnose(); return false; }
		if (static_cast<SemaNumberType *>(LhsType)->getRank() <
		    static_cast<SemaNumberType *>(RhsType)->getRank()) {
			diagnose(); return false;
		}
		return true;
	}

	if (LhsType->isString()) {
		if (!RhsType || !RhsType->isString()) { diagnose(); return false; }
		return true;
	}

	if (LhsType->isArray()) {
		if (!RhsType || !RhsType->isArray()) { diagnose(); return false; }
		SemaArrayType *LhsArr = static_cast<SemaArrayType *>(LhsType);
		SemaArrayType *RhsArr = static_cast<SemaArrayType *>(RhsType);
		if (!CheckEqualTypes(LhsArr->getElementType(), RhsArr->getElementType())) {
			diagnose(); return false;
		}
		return true;
	}

	if (LhsType->isClass()) {
		// Allow long → class: class slots are pointer-sized (i64).
		if (RhsType && RhsType->isNumber() && RhsType->isInteger() &&
		    static_cast<SemaIntType *>(RhsType)->getIntKind() == SemaIntTypeKind::TYPE_LONG)
			return true;
		if (!RhsType || !RhsType->isClass()) { diagnose(); return false; }
		if (!CheckInheritance(static_cast<SemaClassType *>(RhsType),
		                      static_cast<SemaClassType *>(LhsType))) {
			diagnose(); return false;
		}
		return true;
	}

	if (LhsType->isEnum()) {
		// Allow assigning the unset sentinel (null/unset value)
		if (RhsExpr->getKind() == SemaKind::VALUE && RhsType == nullptr)
			return true;
		if (!RhsType || !CheckEqualTypes(LhsType, RhsType)) {
			diagnose(); return false;
		}
		return true;
	}

	return true;
}

bool SemaValidator::CheckNameEmpty(const SourceLocation &Loc, llvm::StringRef Name) {
    return !Name.empty();
}

bool SemaValidator::CheckIsValueExpr(ASTExpr *Expr) {
    return Expr->getExprKind() == ASTExprKind::EXPR_VALUE;
}

bool SemaValidator::CheckVarRefExpr(ASTExpr *Expr) {
    return Expr->getExprKind() == ASTExprKind::EXPR_IDENTIFIER;
}

bool SemaValidator::CheckValue(ASTValue *Value) {
	return true;
}

bool SemaValidator::CheckVar(ASTStmt *Stmt, fly::ASTIdentifier *Ref) {
	return true;
}

bool SemaValidator::CheckCall(ASTStmt *Stmt, fly::ASTCall *Ref) {
	return true;
}

bool SemaValidator::CheckUnary(ASTUnary &AST, SemaExpr *Expr) {
	if (!Expr || !Expr->getType())
		return false;
	SemaType *T = Expr->getType();
	switch (AST.getOpKind()) {
		case ASTUnaryKind::OP_UNARY_PRE_INCR:
		case ASTUnaryKind::OP_UNARY_POST_INCR:
		case ASTUnaryKind::OP_UNARY_PRE_DECR:
		case ASTUnaryKind::OP_UNARY_POST_DECR:
			if (!T->isNumber()) {
				Diag(AST.getLocation(), diag::err_sema_unary_invalid_type)
					<< "++/--" << T->getName();
				return false;
			}
			return true;
		case ASTUnaryKind::OP_UNARY_NOT_LOG:
			if (!T->isBool()) {
				Diag(AST.getLocation(), diag::err_sema_unary_invalid_type)
					<< "!" << T->getName();
				return false;
			}
			return true;
	}
	return true;
}

bool SemaValidator::CheckCondition(const SourceLocation &Loc, SemaExpr *Expr) {
	if (!Expr || !Expr->getType())
		return false;
	if (!CheckConvertibleTypes(Expr->getType(), SemaBuiltin::getBoolType())) {
		Diag(Loc, diag::err_sema_condition_not_bool) << Expr->getType()->getName();
		return false;
	}
	return true;
}

bool SemaValidator::CheckLoopIn(const SourceLocation &Loc, SemaExpr *ListExpr) {
	if (!ListExpr || !ListExpr->getType())
		return false;
	if (!ListExpr->getType()->isArray()) {
		Diag(Loc, diag::err_sema_loopin_not_array) << ListExpr->getType()->getName();
		return false;
	}
	return true;
}
