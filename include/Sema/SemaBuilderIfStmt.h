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
    class SourceLocation;
    class ASTExpr;

    class SemaBuilderIfStmt {

        Sema &S;

        ASTStmt *Parent;

        ASTIfStmt *IfStmt;

        explicit SemaBuilderIfStmt(Sema &S, ASTStmt *Parent);

    public:

        static SemaBuilderIfStmt *Create(Sema &S, ASTStmt *Parent);

        SemaBuilderIfStmt *If(const SourceLocation &Loc, ASTExpr *Expr, ASTStmt *Stmt);

        SemaBuilderIfStmt *ElseIf(const SourceLocation &Loc, ASTExpr *Expr, ASTStmt *Stmt);

        SemaBuilderIfStmt *Else(const SourceLocation &Loc, ASTStmt *Stmt);
    };
}

#endif //FLY_SEMA_BUILDERSTMT_H
