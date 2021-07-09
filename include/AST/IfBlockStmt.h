//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/IfBlockStmt.cpp - If Block Statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_IFBLOCKSTMT_H
#define FLY_IFBLOCKSTMT_H

#include "BlockStmt.h"

namespace fly {

    class IfBlockStmt;
    class ElsifBlockStmt;
    class ElseBlockStmt;

    class IfBlockStmt : public ConditionBlockStmt {

        friend class Parser;
        friend class ElsifBlockStmt;
        friend class ElseBlockStmt;

        enum BlockStmtKind StmtKind = BlockStmtKind::BLOCK_STMT_IF;
        GroupExpr *Condition;
        std::vector<ElsifBlockStmt *> Elsif;
        ElseBlockStmt *Else = NULL;

    public:
        IfBlockStmt(const SourceLocation &Loc, BlockStmt *Parent);

        static void AddBranch(BlockStmt *Parent, ConditionBlockStmt *Cond);

        enum BlockStmtKind getBlockKind() const override;

        GroupExpr *getCondition();

        std::vector<ElsifBlockStmt *> getElsif();

        ElseBlockStmt *getElse();
    };

    class ElsifBlockStmt : public IfBlockStmt {

        friend class Parser;
        friend class IfBlockStmt;

        enum BlockStmtKind StmtKind = BlockStmtKind::BLOCK_STMT_ELSIF;
        IfBlockStmt *Head;

    public:
        ElsifBlockStmt(const SourceLocation &Loc, BlockStmt *Parent);

        enum BlockStmtKind getBlockKind() const override;
    };

    class ElseBlockStmt : public ConditionBlockStmt {

        friend class Parser;
        friend class IfBlockStmt;

        enum BlockStmtKind StmtKind = BlockStmtKind::BLOCK_STMT_ELSE;
        IfBlockStmt *Head;

    public:
        ElseBlockStmt(const SourceLocation &Loc, BlockStmt *Parent);

        enum BlockStmtKind getBlockKind() const override;
    };
}


#endif //FLY_IFBLOCKSTMT_H
