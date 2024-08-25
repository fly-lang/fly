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
    class ASTBlockStmt;
    class ASTExpr;

    class SemaBuilderLoopStmt {

        Sema &S;

        ASTBlockStmt *Parent;

        ASTLoopStmt *LoopStmt;

        explicit SemaBuilderLoopStmt(Sema &S, ASTBlockStmt *Parent);

    public:

        static SemaBuilderLoopStmt *Create(Sema &S, ASTBlockStmt *Parent, const SourceLocation &Loc);

        SemaBuilderLoopStmt *Loop(ASTExpr *Expr, ASTBlockStmt *Stmt);

        void Init(ASTBlockStmt *Stmt);

        void Post(ASTBlockStmt *Stmt);

        void VerifyConditionAtEnd();
    };
}

#endif //FLY_SEMA_BUILDERLOOPSTMT_H
