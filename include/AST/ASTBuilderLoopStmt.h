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

    class Sema;
    class ASTLoopStmt;
    class SourceLocation;
    class ASTStmt;
    class ASTBlockStmt;
    class ASTExpr;

    class ASTBuilderLoopStmt {

    	friend class ASTLoopStmt;

        ASTBlockStmt *Parent;

        ASTLoopStmt *LoopStmt;

        explicit ASTBuilderLoopStmt(ASTBlockStmt *Parent);

    public:

        static ASTBuilderLoopStmt *Create(ASTBlockStmt *Parent, const SourceLocation &Loc);

        ASTBuilderLoopStmt *Loop(ASTExpr *Expr, ASTBlockStmt *Stmt);

        void Init(ASTBlockStmt *Stmt);

        void Post(ASTBlockStmt *Stmt);

        void VerifyConditionAtEnd();
    };
}

#endif //FLY_AST_BUILDERLOOPSTMT_H
