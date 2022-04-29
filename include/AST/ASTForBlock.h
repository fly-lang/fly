//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTForBlock.h - AST For Block Statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTFORBLOCK_H
#define FLY_ASTFORBLOCK_H

#include "ASTBlock.h"

namespace fly {

    class ASTForBlock : public ASTBlock {

        friend class Parser;

        enum ASTBlockKind StmtKind = ASTBlockKind::BLOCK_STMT_FOR;

        ASTBlock *Cond;
        ASTBlock *Post;
        ASTBlock *Loop;

    public:
        ASTForBlock(const SourceLocation &Loc, ASTBlock *Parent);

        virtual ~ASTForBlock();

        enum ASTBlockKind getBlockKind() const override;

        ASTBlock *getCondition();

        void setCondition(ASTExpr *Expr);

        ASTBlock *getPost();

        ASTBlock *getLoop();

    };
}


#endif //FLY_ASTFORBLOCK_H
