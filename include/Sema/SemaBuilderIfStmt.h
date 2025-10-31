//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaBuilderIfStmt.h - Sema Builder If Stmt
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_BUILDERIFSTMT_H
#define FLY_SEMA_BUILDERIFSTMT_H

namespace fly {

    class ASTBuilder;
    class ASTIfStmt;
    class ASTStmt;
    class ASTBlockStmt;
    class SourceLocation;
    class ASTExpr;

    class SemaBuilderIfStmt {

        ASTBlockStmt *Parent; // FIXME remove, extends Stmt?

        ASTIfStmt *IfStmt;

        explicit SemaBuilderIfStmt(ASTBlockStmt *Parent);

    public:

        static SemaBuilderIfStmt *Create(ASTBlockStmt *Parent);

        SemaBuilderIfStmt *If(const SourceLocation &Loc, ASTExpr *Expr, ASTBlockStmt *Stmt);

        SemaBuilderIfStmt *ElseIf(const SourceLocation &Loc, ASTExpr *Expr, ASTBlockStmt *Stmt);

        SemaBuilderIfStmt *Else(const SourceLocation &Loc, ASTBlockStmt *Stmt);
    };
}

#endif //FLY_SEMA_BUILDERSTMT_H
