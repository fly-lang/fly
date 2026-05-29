//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTBuilderLoopStmt.h - AST builder for loop statements
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

        ASTLoopStmt *LoopStmt = nullptr;

        explicit ASTBuilderLoopStmt();

    public:

        static ASTBuilderLoopStmt *Create(ASTBlockStmt *Parent, const SourceLocation &Loc);

        ASTBuilderLoopStmt *setCycle(ASTExpr *Expr, ASTBlockStmt *Stmt);

        ASTBuilderLoopStmt *setInit(ASTBlockStmt *Stmt);

        ASTBuilderLoopStmt *setPost(ASTBlockStmt *Stmt);

        void VerifyConditionAtEnd();
    };
}

#endif //FLY_AST_BUILDERLOOPSTMT_H
