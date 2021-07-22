//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTIfBlock.h - AST If Block Statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTIFBLOCK_H
#define FLY_ASTIFBLOCK_H

#include "ASTBlock.h"

namespace fly {

    class ASTIfBlock;
    class ElsifBlockStmt;
    class ElseBlockStmt;

    class ASTIfBlock : public ConditionBlockStmt {

        friend class Parser;
        friend class ElsifBlockStmt;
        friend class ElseBlockStmt;

        enum BlockStmtKind StmtKind = BlockStmtKind::BLOCK_STMT_IF;
        ASTGroupExpr *Condition;
        std::vector<ElsifBlockStmt *> Elsif;
        ElseBlockStmt *Else = NULL;

    public:
        ASTIfBlock(const SourceLocation &Loc, ASTBlock *Parent);

        static void AddBranch(ASTBlock *Parent, ConditionBlockStmt *Cond);

        enum BlockStmtKind getBlockKind() const override;

        ASTGroupExpr *getCondition();

        std::vector<ElsifBlockStmt *> getElsif();

        ElseBlockStmt *getElse();
    };

    class ElsifBlockStmt : public ASTIfBlock {

        friend class Parser;
        friend class ASTIfBlock;

        enum BlockStmtKind StmtKind = BlockStmtKind::BLOCK_STMT_ELSIF;
        ASTIfBlock *Head;

    public:
        ElsifBlockStmt(const SourceLocation &Loc, ASTBlock *Parent);

        enum BlockStmtKind getBlockKind() const override;
    };

    class ElseBlockStmt : public ConditionBlockStmt {

        friend class Parser;
        friend class ASTIfBlock;

        enum BlockStmtKind StmtKind = BlockStmtKind::BLOCK_STMT_ELSE;
        ASTIfBlock *Head;

    public:
        ElseBlockStmt(const SourceLocation &Loc, ASTBlock *Parent);

        enum BlockStmtKind getBlockKind() const override;
    };
}


#endif //FLY_ASTIFBLOCK_H
