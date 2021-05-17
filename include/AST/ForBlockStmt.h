//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ForBlockStmt.h - For Block Statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_FORBLOCKSTMT_H
#define FLY_FORBLOCKSTMT_H

#include "BlockStmt.h"

namespace fly {

    class ForBlockStmt : public BlockStmt {

        friend class Parser;

        enum BlockStmtKind StmtKind = BlockStmtKind::BLOCK_STMT_FOR;

        BlockStmt *Init;
        GroupExpr *Cond;
        BlockStmt *Post;

    public:
        ForBlockStmt(const SourceLocation &Loc, BlockStmt *Parent);

        enum BlockStmtKind getBlockKind() const override;

        const BlockStmt *getInit() const;

        const GroupExpr *getCondition() const;

        const BlockStmt *getPost() const;

    };
}


#endif //FLY_FORBLOCKSTMT_H