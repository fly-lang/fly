//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaBuilderSwitchStmt.h - Sema Builder Switch Stmt
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_BUILDERSWITCHSTMT_H
#define FLY_SEMA_BUILDERSWITCHSTMT_H

namespace fly {

    class Sema;
    class ASTSwitchStmt;
    class SourceLocation;
    class ASTStmt;
    class ASTBlockStmt;
    class ASTVarRef;
    class ASTExpr;

    class SemaBuilderSwitchStmt {

        Sema &S;

        ASTBlockStmt *Parent;

        ASTSwitchStmt *SwitchStmt;

        explicit SemaBuilderSwitchStmt(Sema &S, ASTBlockStmt *Parent);


    public:

        static SemaBuilderSwitchStmt *Create(Sema &S, ASTBlockStmt *Parent);

        SemaBuilderSwitchStmt *Switch(const SourceLocation &Loc, ASTExpr *Expr);

        SemaBuilderSwitchStmt *Case(const SourceLocation &Loc, ASTExpr *Expr, ASTBlockStmt *Stmt);

        SemaBuilderSwitchStmt *Default(const SourceLocation &Loc, ASTBlockStmt *Stmt);
    };
}

#endif //FLY_SEMA_BUILDERSWITCHSTMT_H
