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
    class ASTVarRef;
    class ASTExpr;

    class SemaBuilderSwitchStmt {

        Sema &S;

        ASTStmt *Parent;

        ASTSwitchStmt *SwitchStmt;

        explicit SemaBuilderSwitchStmt(Sema &S, ASTStmt *Parent);


    public:

        static SemaBuilderSwitchStmt *Create(Sema &S, ASTStmt *Parent);

        SemaBuilderSwitchStmt *Switch(const SourceLocation &Loc, ASTExpr *Expr);

        SemaBuilderSwitchStmt *Case(const SourceLocation &Loc, ASTExpr *Expr, ASTStmt *Stmt);

        SemaBuilderSwitchStmt *Default(const SourceLocation &Loc, ASTStmt *Stmt);
    };
}

#endif //FLY_SEMA_BUILDERSWITCHSTMT_H
