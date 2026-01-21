//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTBuilderLoopStmt.h - Sema Builder If Stmt
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_BUILDERLOOPSTMT_H
#define FLY_AST_BUILDERLOOPSTMT_H

namespace fly {

    class SemaContext;
    class ASTLoopStmt;
    class ASTLoopInStmt;
    class SourceLocation;
    class ASTStmt;
    class ASTBlockStmt;
    class ASTExpr;

    class ASTBuilderLoopStmt {

    	friend class ASTLoopStmt;

        ASTBlockStmt *Parent;

        ASTLoopStmt *LoopStmt;

        ASTLoopInStmt *LoopInStmt;

        explicit ASTBuilderLoopStmt(ASTBlockStmt *Parent);

    public:

        static ASTBuilderLoopStmt *CreateLoop(ASTBlockStmt *Parent, const SourceLocation &Loc);

        static ASTLoopInStmt *CreateLoopIn(ASTBlockStmt *Parent, const SourceLocation &Loc, ASTExpr *Item, ASTExpr *List, ASTBlockStmt *Stmt);

        ASTBuilderLoopStmt *Loop(ASTExpr *Expr, ASTBlockStmt *Stmt);

        void Init(ASTBlockStmt *Stmt);

        void Post(ASTBlockStmt *Stmt);

        void VerifyConditionAtEnd();
    };
}

#endif //FLY_AST_BUILDERLOOPSTMT_H
