//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTBuilderLoopStmt.h - Sema Builder If Stmt
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_BUILDERLOOPINSTMT_H
#define FLY_AST_BUILDERLOOPINSTMT_H

namespace fly {

    class SemaContext;
    class ASTLoopInStmt;
    class SourceLocation;
    class ASTStmt;
    class ASTBlockStmt;
    class ASTExpr;

    class ASTBuilderLoopInStmt {

    	friend class ASTLoopInStmt;

    	ASTLoopInStmt * LoopStmt;

		explicit ASTBuilderLoopInStmt();

    public:

        static ASTBuilderLoopInStmt *Create(ASTBlockStmt *Parent, const SourceLocation &Loc, ASTExpr *Item, ASTExpr *List, ASTBlockStmt *Stmt);

        ASTBuilderLoopInStmt *setCycle(ASTBlockStmt *Stmt);

    };
}

#endif //FLY_AST_BUILDERLOOPINSTMT_H
