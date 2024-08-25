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

    class Sema;
    class SemaBuilder;
    class ASTIfStmt;
    class ASTStmt;
    class ASTBlockStmt;
    class SourceLocation;
    class ASTExpr;

    class SemaBuilderIfStmt {

        Sema &S;

        ASTBlockStmt *Parent;

        ASTIfStmt *IfStmt;

        explicit SemaBuilderIfStmt(Sema &S, ASTBlockStmt *Parent);

    public:

        static SemaBuilderIfStmt *Create(Sema &S, ASTBlockStmt *Parent);

        SemaBuilderIfStmt *If(const SourceLocation &Loc, ASTExpr *Expr, ASTBlockStmt *Stmt);

        SemaBuilderIfStmt *ElseIf(const SourceLocation &Loc, ASTExpr *Expr, ASTBlockStmt *Stmt);

        SemaBuilderIfStmt *Else(const SourceLocation &Loc, ASTBlockStmt *Stmt);
    };
}

#endif //FLY_SEMA_BUILDERSTMT_H
