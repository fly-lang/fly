//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaBuilderStmt.h - Sema Builder Stmt
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_BUILDERSTMT_H
#define FLY_SEMA_BUILDERSTMT_H

namespace fly {

    class ASTBuilder;
    class ASTStmt;
    class ASTBlockStmt;
    class SourceLocation;
    class ASTExpr;
    class ASTRef;
    class ASTVar;
    enum class ASTAssignOperatorKind;

    class SemaBuilderStmt {

        SemaBuilderStmt();

        ASTStmt *Stmt;

    public:

        static SemaBuilderStmt *CreateAssignment(ASTBlockStmt *Parent, ASTRef *VarRef, ASTAssignOperatorKind AssignOperatorKind);

        static SemaBuilderStmt *CreateVar(ASTBlockStmt *Parent, ASTVar *Var);

        static SemaBuilderStmt *CreateReturn(ASTBlockStmt *Parent, const SourceLocation &Loc);

        static SemaBuilderStmt *CreateFail(ASTBlockStmt *Parent, const SourceLocation &Loc);

        static SemaBuilderStmt *CreateExpr(ASTBlockStmt *Parent, const SourceLocation &Loc);

        void setExpr(ASTExpr *Expr);
    };
}

#endif //FLY_SEMA_BUILDERSTMT_H
