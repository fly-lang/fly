//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaBuilderLoopStmt.h - Sema Builder If Stmt
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_BUILDERLOOPSTMT_H
#define FLY_SEMA_BUILDERLOOPSTMT_H

namespace fly {

    class Sema;
    class ASTLoopStmt;
    class SourceLocation;
    class ASTStmt;
    class ASTExpr;

    class SemaBuilderLoopStmt {

        Sema &S;

        ASTStmt *Parent;

        ASTLoopStmt *LoopStmt;

        explicit SemaBuilderLoopStmt(Sema &S, ASTStmt *Parent);

    public:

        static SemaBuilderLoopStmt *Create(Sema &S, ASTStmt *Parent);

        SemaBuilderLoopStmt *Loop(const SourceLocation &Loc, ASTExpr *Expr, ASTStmt *Stmt);

        void Init(ASTStmt *Stmt);

        void Post(ASTStmt *Stmt);

        void VerifyConditionAtEnd();
    };
}

#endif //FLY_SEMA_BUILDERLOOPSTMT_H
