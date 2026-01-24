//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaValidator.cpp - The Sema Validator
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
#include "Basic/Diagnostic.h"
#include "Sema/SemaNameSpace.h"

#include <AST/ASTExpr.h>
#include <Sema/SemaClassType.h>
#include <Sema/SemaEnumType.h>
#include <Sema/SemaType.h>

using namespace fly;

SemaValidator::SemaValidator(DiagnosticsEngine &Diags) : Diags(Diags) {
}

DiagnosticBuilder SemaValidator::Diag(const SourceLocation &Loc, unsigned DiagID) const {
	return Diags.Report(Loc, DiagID);
}

DiagnosticBuilder SemaValidator::Diag(unsigned DiagID) const {
	return Diags.Report(DiagID);
}

void SemaValidator::CheckImport(const ASTImport &AST) {

	// Error: Empty Import
	if (AST.getNames().empty()) {
		Diag(AST.getLocation(), diag::err_sema_import_undefined);
	}

	// TODO
	// Error: name is equals to the current ASTModule namespace
	// if (AST.getName() == Module->getNameSpace()->getName()) {
	// 	Diag(AST.getLocation(), diag::err_import_conflict_namespace) << AST.getName();
	// }

	// Replace with alias name if exists
	// llvm::StringRef Name = AST.getImport()->getName();

	// Error: alias is equals to the current ASTModule namespace
	// if (AST.getAlias()) {
	//
	// 	// Set Import Name
	// 	Name = AST.getAlias()->getName();
	//
	// 	// Check Alias
	// 	if (Module->getImports().lookup(Name) != nullptr) {
	// 		Diag(AST.getLocation(), diag::err_conflict_import_alias) << Name;
	// 		return;
	// 	}
	//
	// 	if (AST.getAlias()->getName() == Module->getNameSpace()->getName()) {
	// 		Diag(AST.getAlias()->getLocation(), diag::err_alias_conflict_namespace) << AST.getAlias()->getName();
	// 		return;
	// 	}
	// }
}

// bool SemaValidator::CheckDuplicateModules(ASTModule *Module, const llvm::DenseMap<uint64_t, SemaModule *> &Modules) {
// 	// Check Duplicate Module Names
// 	for (auto ModuleEntry : Modules) {
// 		ASTModule * AST = ModuleEntry.getSecond()->getAST();
// 		if (AST->getId() != Module->getId() && AST->getName() == Module->getName()) {
// 			// S.Diag(diag::err_sema_module_duplicated) << AST->getName();
// 			return false;
// 		}
// 	}
// 	return true;
// }

// bool SemaValidator::CheckDuplicateIdentities(const llvm::StringMap<SymIdentity *> &Identities, ASTIdentity * Identity) {
// 	SymIdentity *DuplicateIdentity = Identities.lookup(Identity->getName());
// 	if (DuplicateIdentity) { // This CurrentNameSpace already contains this Identity
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
            // if (DiagEnabled)
            //     S.Diag(Param->getLocation(), diag::err_conflict_params) << Param->getName();
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
    // ASTLocalVar *DuplicateVar = Block->getLocalVars().lookup(VarName);
    // if (DuplicateVar != nullptr) {
        // if (DiagEnabled)
        //     S.Diag(DuplicateVar->getLocation(), diag::err_conflict_vardecl) << DuplicateVar->getName();
        // return false;
    // }

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

bool SemaValidator::CheckCommentReturn(SemaComment *Comment, ASTType *ReturnType) {
	// TODO
	return true;
}

bool SemaValidator::CheckCommentFail(SemaComment *Comment) {
	// TODO
	return true;
}

bool SemaValidator::CheckExpr(ASTExpr *Expr) {
    if (!Expr->getSema()->getType()) {
        // if (DiagEnabled)
        //     S.Diag(Expr->getLocation(), diag::err_expr_type_miss);
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
    if (FromType->isFloat() && ToType->isFloat()) {
	    SemaFloatType *FromFloatingType = static_cast<SemaFloatType *>(FromType);
	    SemaFloatType *ToFloatingType = static_cast<SemaFloatType *>(ToType);
	    return FromFloatingType->getFloatKind() <= ToFloatingType->getFloatKind();
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
    	return FromEnumType->isDerivedOrEquals(ToEnumType);
    }

    // Check Class Inheritance
    if (FromType->isClass() && ToType->isClass()) {
    	SemaClassType *FromClassType = static_cast<SemaClassType *>(FromType);
    	SemaClassType *ToClassType = static_cast<SemaClassType *>(ToType);
    	return FromClassType->isDerivedOrEquals(ToClassType);
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

bool SemaValidator::CheckInheritance(SemaClassType *ClassType, SemaClassType *SuperClassType) {
	// Check if ClassType is equals to SuperClassType
	if (ClassType->getId() == SuperClassType->getId()) {
		return true;
	}

	// Check if ClassType is a subclass of SuperClassType
	for (auto &Entry : ClassType->getBaseClasses()) {
		if (CheckInheritance(SuperClassType, Entry)) {
			return true;
		}
	}

	return false;
}

bool SemaValidator::CheckInheritance(SemaEnumType *EnumType, SemaEnumType *SuperEnumType) {
	// Check if TheClass is equals to SuperClass
	if (EnumType->getId() == SuperEnumType->getId()) {
		return true;
	}

	// Check if TheClass is a subclass of SuperClass
	for (auto &SuperClassEntry : EnumType->getBaseEnums()) {
		if (CheckInheritance(SuperClassEntry.getValue(), SuperEnumType)) {
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
	if (Type1->isFloat() && Type2->isFloat()) {
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

bool SemaValidator::CheckBinary(ASTBinary &AST) {
	// Check if Left and Right Expr have Sema
	if (!AST.getLeftExpr()->getSema() || !AST.getRightExpr()->getSema()) {
		return false;
	}

	// Check if Left and Right Expr are resolved
	SemaType * LeftType = AST.getLeftExpr()->getType();
	SemaType * RightType = AST.getRightExpr()->getType();

	// Arithmetic Operations: Integer/Float, Float/Float, Integer/Integer
	if (AST.isArith()) {

		// Check between numbers
		if (LeftType->isNumber() && RightType->isNumber()) {
			return true;
		}

		// Error: Binary Arithmetic operation can be made only with numbers
		Diag(AST.getLocation(), diag::err_sema_types_operation)
				  << LeftType->getName()
				  << RightType->getName();
		return false;
	}

	// Comparison Operations: Bool/Bool, Integer/Integer, Float/Float, Class/Class, Enum/Enum
	if (AST.isCompare()) {
		if (LeftType->isBool() && RightType->isBool()) {
			// OK: Boolean Comparison
			return true;
		}
		if (LeftType->isNumber() && RightType->isNumber()) {
			// OK: Numeric Comparison
			return true;
		}
		if (LeftType->isClass() && RightType->isClass()) {
			// OK: Class Comparison
			return true;
		}
		if (LeftType->isEnum() && RightType->isEnum()) {
			// OK: Enum Comparison
			return true;
		}
		// Error: Binary Comparison operation can be made only with numbers, bools, classes or enums
		Diag(AST.getLocation(), diag::err_sema_types_operation)
				  << LeftType->getName()
				  << RightType->getName();
		return false;
	}

	// Logical Operations: Bool/Bool
	if (AST.isLogic()) {
		if (LeftType->isBool() && RightType->isBool()) {
			// OK: Boolean Operation
			return true;
		}

		// Error: Binary Comparison operation can be made only with numbers, bools, classes or enums
		Diag(AST.getLocation(), diag::err_sema_types_operation)
				  << LeftType->getName()
				  << RightType->getName();
		return false;
	}

	// Assignment Operations: Compatible Types
	if (AST.isAssign()) {

		// Right Type can be always converted to Boolean
		if (LeftType->isBool()) {
			return true;
		}

		if (LeftType->isNumber()) {
			if (!RightType->isNumber()) {
				// Cannot convert non-number to number
				Diag(AST.getLocation(), diag::err_sema_types_operation)
				  << LeftType->getName()
				  << RightType->getName();
				return false;
			}
			if (RightType->isNumber() && static_cast<SemaNumberType *>(LeftType)->getRank() < static_cast<SemaNumberType *>(RightType)->getRank()) {
				// Cannot convert number with higher rank to lower rank
				Diag(AST.getLocation(), diag::err_sema_types_operation)
				  << LeftType->getName()
				  << RightType->getName();
				return false;
			}
		}

		if (LeftType->isString()) {
			if (!RightType->isString()) {
				// Number rank not compatible, cannot be converted
				Diag(AST.getLocation(), diag::err_sema_types_operation)
				  << LeftType->getName()
				  << RightType->getName();
				return true;
			}
		}

		if (LeftType->isArray()) {
			if (!RightType->isArray()) {
				// Array type not compatible, cannot be converted
				Diag(AST.getLocation(), diag::err_sema_types_operation)
				  << LeftType->getName()
				  << RightType->getName();
				return false;
			}
			return CheckEqualTypes(LeftType, RightType);
		}

		if (LeftType->isClass()) {
			return CheckInheritance(static_cast<SemaClassType *>(RightType), static_cast<SemaClassType *>(LeftType));
		}

		if (LeftType->isEnum()) {
			return CheckEqualTypes(LeftType, RightType);
		}

		return true;
	}

	// Error: Invalid Binary Operation
	Diag(AST.getLocation(), diag::err_invalid_behavior)
				  << LeftType->getName()
				  << RightType->getName();
	return false;
}

bool SemaValidator::CheckNameEmpty(const SourceLocation &Loc,llvm::StringRef Name) {
    return Name.empty() ? false : true;
}


bool SemaValidator::CheckIsValueExpr(ASTExpr *Expr) {
    if (Expr->getExprKind() == ASTExprKind::EXPR_VALUE) {
        return true;
    }
    return false;
}

bool SemaValidator::CheckVarRefExpr(ASTExpr *Expr) {
    if (Expr->getExprKind() == ASTExprKind::EXPR_IDENTIFIER) {
        return true;
    }
    return false;
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
