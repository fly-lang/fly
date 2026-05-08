//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/Sema.h - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_VALIDATOR_H
#define FLY_SEMA_VALIDATOR_H

#include <AST/ASTEnum.h>
#include <llvm/ADT/DenseMap.h>

#include "AST/ASTType.h"
#include "AST/ASTVar.h"

namespace fly {

    class SemaContext;
    class ASTBlockStmt;
    class ASTStmt;
    class ASTIdentifier;
    class ASTModule;
    class ASTImport;
    class ASTExpr;
    class ASTVar;
    class ASTClass;
    class ASTType;
    class ASTCall;
    class SourceLocation;
    class SemaModule;
    class SemaFunction;
    class SemaClassType;
    class SemaEnumType;
    class SemaComment;
    class ASTValue;
    class SemaClassMethod;
    class SemaFunctionBase;
	class ASTBinary;
	class DiagnosticsEngine;
	class DiagnosticBuilder;
	class SemaType;
	class SemaExpr;
    enum class SemaTypeKind;

    class SemaValidator {

        friend class SemaContext;

    	DiagnosticsEngine &Diags;

    public:

    	explicit SemaValidator(DiagnosticsEngine &Diags);

    	// Diagnostics
    	DiagnosticBuilder Diag(const SourceLocation &Loc, unsigned DiagID) const;
    	DiagnosticBuilder Diag(unsigned DiagID) const;

    	void CheckImport(const ASTImport &AST);

    	void CheckCast(SemaExpr * From, SemaExpr * To);

		// static bool CheckDuplicateModules(ASTModule * Module, const llvm::DenseMap<uint64_t, SemaModule *> &Modules);

        static bool CheckDuplicateParams(llvm::SmallVector<ASTVar *, 8> Params, ASTVar *Param);

        static bool CheckDuplicateLocalVars(ASTStmt *Stmt, llvm::StringRef VarName);

        static bool CheckExpr(SemaExpr *Expr);

        static bool CheckEqualTypes(SemaType *Type1, SemaType *Type2);

        bool CheckConvertibleTypes(SemaType *FromType, SemaType *ToType);

        bool CheckInheritance(SemaClassType *ClassType, SemaClassType *SuperClassType);

        bool CheckInheritance(SemaEnumType *EnumType, SemaEnumType *SuperEnumType);

    	bool CheckBinary(ASTBinary &AST, SemaExpr *LeftSema, SemaExpr *RightSema);

		static bool CheckNameEmpty(const SourceLocation &Loc, llvm::StringRef Name);

        static bool CheckIsValueExpr(ASTExpr *Expr);

        static bool CheckVarRefExpr(ASTExpr *Expr);

        static bool CheckValue(ASTValue* Value);

        static bool CheckVar(ASTStmt* Stmt, fly::ASTIdentifier* Ref);

        static bool CheckCall(ASTStmt* Stmt, fly::ASTCall* Ref);
    };

}  // end namespace fly

#endif // FLY_SEMA_VALIDATOR_H