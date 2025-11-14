//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTBuilderSwitchStmt.h - Sema Builder Switch Stmt
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_BUILDERSWITCHSTMT_H
#define FLY_AST_BUILDERSWITCHSTMT_H

namespace fly {

    class ASTSwitchStmt;
    class SourceLocation;
    class ASTStmt;
    class ASTBlockStmt;
    class ASTExpr;

    class ASTBuilderSwitchStmt {

        friend class ASTSwitchStmt;

        ASTBlockStmt *Parent;

        ASTSwitchStmt *SwitchStmt;

        explicit ASTBuilderSwitchStmt(ASTBlockStmt *Parent);

    public:

        static ASTBuilderSwitchStmt *Create(ASTBlockStmt *Parent);

        ASTBuilderSwitchStmt *Switch(const SourceLocation &Loc, ASTExpr *Expr);

        ASTBuilderSwitchStmt *Case(const SourceLocation &Loc, ASTExpr *Expr, ASTBlockStmt *Stmt);

        ASTBuilderSwitchStmt *Default(const SourceLocation &Loc, ASTBlockStmt *Stmt);

        bool hasDefault();
    };
}

#endif //FLY_AST_BUILDERSWITCHSTMT_H
