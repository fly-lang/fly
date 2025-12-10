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

    class Sema;
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
    enum class SemaTypeKind;

    class SemaValidator {

        friend class Sema;

    public:

        // static bool CheckDuplicateModules(ASTModule * Module, const llvm::DenseMap<uint64_t, SemaModule *> &Modules);

        static bool CheckDuplicateParams(llvm::SmallVector<ASTVar *, 8> Params, ASTVar *Param);

        static bool CheckDuplicateLocalVars(ASTStmt *Stmt, llvm::StringRef VarName);

        static bool CheckCommentParams(SemaComment *Comment, const llvm::SmallVector<ASTVar*, 8> &Params);

        static bool CheckCommentReturn(SemaComment *Comment, ASTType* ReturnType);

        static bool CheckCommentFail(SemaComment *Comment);

        static bool CheckExpr(ASTExpr *Expr);

        static bool CheckEqualTypes(SemaType *Type1, SemaType *Type2);

        static bool CheckConvertibleTypes(SemaType *FromType, SemaType *ToType);

        static bool CheckInheritance(SemaClassType *ClassType, SemaClassType *SuperClassType);

        static bool CheckInheritance(SemaEnumType *EnumType, SemaEnumType *SuperEnumType);

        static bool CheckArithTypes(SemaType *Type1, SemaType *Type2);

        static bool CheckLogicalTypes(SemaType *Type1, SemaType *Type2);

        static bool CheckNameEmpty(const SourceLocation &Loc, llvm::StringRef Name);

        static bool CheckIsValueExpr(ASTExpr *Expr);

        static bool CheckVarRefExpr(ASTExpr *Expr);

        static bool CheckValue(ASTValue* Value);

        static bool CheckVar(ASTStmt* Stmt, fly::ASTIdentifier* Ref);

        static bool CheckCall(ASTStmt* Stmt, fly::ASTCall* Ref);
    };

}  // end namespace fly

#endif // FLY_SEMA_VALIDATOR_H