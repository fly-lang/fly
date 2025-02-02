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

#include <Sym/SymGlobalVar.h>
#include <Sym/SymModule.h>

using namespace fly;

SemaValidator::SemaValidator(Sema &S) : S(S) {

}

bool SemaValidator::CheckDuplicateModules(ASTModule *Module) {
	// Check Duplicate Module Names
	for (auto ModuleEntry : S.getSymTable().getModules()) {
		ASTModule * AST = ModuleEntry.getValue()->getAST();
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

bool SemaValidator::CheckDuplicateFunctions(
	const llvm::SmallVector<SymFunction *, 8> &Functions, ASTFunction *function) {

	// Create the Functions Structure for check duplicates


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

bool SemaValidator::CheckClass(ASTClass *Class) {
	return true;
}

bool SemaValidator::CheckEnum(ASTEnum *Enum) {
	return true;
}

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
    ASTLocalVar *DuplicateVar = Block->getLocalVars().lookup(VarName);
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

bool SemaValidator::CheckCommentParams(const llvm::SmallVector<ASTVar *, 8> &Params) {
	// TODO
}

bool SemaValidator::CheckCommentReturn(ASTTypeRef *ReturnType) {
	// TODO
}

/**
 * Check Name and Alias on ASTImport
 * @param Module
 * @param Import
 * @return
 */
bool SemaValidator::CheckImport(ASTImport *Import) {
	ASTModule *Module = Import->getModule();

    // Error: Empty Import
    if (Import->getName().empty()) {
        if (DiagEnabled)
            S.Diag(Import->getLocation(), diag::err_sema_import_undefined);
        return false;
    }

    // Error: name is equals to the current ASTModule namespace
    if (Import->getName() == Module->getNameSpace()->getDef()->getName()) {
        if (DiagEnabled)
            S.Diag(Import->getLocation(), diag::err_import_conflict_namespace) << Import->getName();
        return false;
    }

    // Error: alias is equals to the current ASTModule namespace
    if (Import->getAlias() && Import->getAlias()->getName() == Module->getNameSpace()->getDef()->getName()) {
        if (DiagEnabled)
            S.Diag(Import->getAlias()->getLocation(), diag::err_alias_conflict_namespace) << Import->getAlias()->getName();
        return false;
    }

	// Search Namespace in Symbol Table
	SymNameSpace *ImportNameSpace = S.getSymTable().getNameSpaces().lookup(Import->getName());
	if (!ImportNameSpace) {
		// Error: NameSpace not found
		S.Diag(Import->getLocation(), diag::err_namespace_notfound) << Import->getName();
		return false;
	}

    return true;
}

bool SemaValidator::CheckExpr(ASTExpr *Expr) {
    if (!Expr->getTypeRef()) {
        if (DiagEnabled)
            S.Diag(Expr->getLocation(), diag::err_expr_type_miss);
        return false;
    }
    return true;
}

bool SemaValidator::CheckEqualTypes(ASTType *Type1, ASTType *Type2) {
    if (Type1->getStmtKind() == Type2->getStmtKind()) {
        if (Type1->isArray()) {
            return CheckEqualTypes(((ASTArrayType *) Type1)->getType(), ((ASTArrayType *) Type2)->getType());
        } else if (Type1->isIdentity()) {
            if (((ASTRefType *) Type1)->isClass()) {
                return CheckClassInheritance((ASTClassType *) Type1, (ASTClassType *) Type2) ||
                        CheckClassInheritance((ASTClassType *) Type2, (ASTClassType *) Type1);
            } else {
                return ((ASTRefType *) Type1)->getFullName() == ((ASTRefType *) Type2)->getFullName();
            }
        }
        return true;
    }

    return false;
}

bool SemaValidator::CheckEqualTypes(ASTType *Type, ASTValueKind Kind) {
    if (Type->getStmtKind() != Kind) {
        if (DiagEnabled)
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
        return ((ASTArrayType *) FromType)->getType()->getStmtKind() ==
               ((ASTArrayType *) ToType)->getType()->getStmtKind();
    }

    else if (FromType->isIdentity() && ToType->isIdentity()) {
        ASTRefType *FromIdentityType = (ASTRefType *) FromType;
        ASTRefType *ToIdentityType = (ASTRefType *) ToType;

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

    else if (FromType->isString() && ToType->isString()) {
        return true;
    }

    else if (FromType->isError()) {
        if (ToType->isBool() || ToType->isInteger() || ToType->isString()) {
            return true;
        }
    }

    if (DiagEnabled)
        S.Diag(FromType->getLocation(), diag::err_sema_types_convert)
                << FromType->print()
                << ToType->print();

    return false;
}

bool SemaValidator::CheckArithTypes(const SourceLocation &Loc, ASTType *Type1, ASTType *Type2) {
    if (Type1->getStmtKind() == Type2->getStmtKind()) {
        return true;
    }

    if (DiagEnabled)
        S.Diag(Loc, diag::err_sema_types_operation)
                << Type1->print()
                << Type2->print();

    return false;
}

bool SemaValidator::CheckLogicalTypes(const SourceLocation &Loc, ASTType *Type1, ASTType *Type2) {
    if (Type1->getStmtKind() == ASTValueKind::TYPE_BOOL && Type2->getStmtKind() == ASTValueKind::TYPE_BOOL) {
        return true;
    }

    if (DiagEnabled)
        S.Diag(Loc, diag::err_sema_types_logical)
                << Type1->print()
                << Type2->print();

    return false;
}

// bool SemaValidator::CheckClassInheritance(ASTClassType *FromType, ASTClassType *ToType) {
//     const StringRef &FromName = FromType->getDef()->getName();
//     const StringRef &ToName = ToType->getDef()->getName();
//     if (FromName == ToName) {
//         return true;
//     } else {
//         const SmallVector<ASTClassType *, 4> &SuperClasses = ((ASTClass *) FromType->getDef())->getSuperClasses();
//         for (ASTClassType *SuperClass : SuperClasses) {
//             if (CheckClassInheritance(SuperClass, ToType)) {
//                 return true;
//             }
//         }
//     }
//     return false;
// }

void SemaValidator::CheckCreateModule(const std::string &Name) {
    if (Name.empty()) {
        S.Diag(diag::err_sema_module_name_empty);
    }
}

void SemaValidator::CheckCreateNameSpace(const SourceLocation &Loc, llvm::StringRef Name) {
    if (Name.empty()) {
        S.Diag(Loc, diag::err_sema_namespace_empty);
    }
}

void SemaValidator::CheckCreateIdentifier(const SourceLocation &Loc,llvm::StringRef Name) {
    if (Name.empty()) {
        S.Diag(Loc, diag::err_sema_identifier_empty);
    }
}

void SemaValidator::CheckCreateImport(const SourceLocation &Loc, StringRef Name) {

}

void SemaValidator::CheckCreateAlias(const SourceLocation &Loc, StringRef Name) {

}

void
SemaValidator::CheckCreateGlobalVar(const SourceLocation &Loc, ASTTypeRef *TypeRef, const StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes) {

}

void
SemaValidator::CheckCreateFunction(const SourceLocation &Loc, ASTTypeRef *TypeRef, const StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes) {

}

void SemaValidator::CheckCreateClass(const SourceLocation &Loc, StringRef Name, ASTClassKind ClassKind,
                                     llvm::SmallVector<ASTScope *, 8> &Scopes, SmallVector<ASTClassType *, 4> &ClassTypes) {

}

void SemaValidator::CheckCreateClassAttribute(const SourceLocation &Loc, StringRef Name, ASTTypeRef *TypeRef, llvm::SmallVector<ASTScope *, 8> &Scopes) {

}

void SemaValidator::CheckCreateClassConstructor(const SourceLocation &Loc, llvm::SmallVector<ASTScope *, 8> &Scopes) {

}

void
SemaValidator::CheckCreateClassMethod(const SourceLocation &Loc, ASTTypeRef *TypeRef, StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes) {

}

void SemaValidator::CheckCreateEnum(const SourceLocation &Loc, const StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes,
                                    SmallVector<ASTEnumType *, 4> EnumTypes) {

}

void SemaValidator::CheckCreateEnumEntry(const SourceLocation &Loc, StringRef Name) {

}

void SemaValidator::CheckCreateParam(const SourceLocation &Loc, ASTTypeRef *TypeRef, StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes) {

}

void SemaValidator::CheckCreateLocalVar(const SourceLocation &Loc, ASTTypeRef *TypeRef, StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes) {

}

bool SemaValidator::CheckValueExpr(ASTExpr *Expr) {
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

bool SemaValidator::CheckScopes(const SmallVector<ASTScope *, 8> &vector) {
	return true;
}

