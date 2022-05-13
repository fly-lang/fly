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
    class ASTElsifBlock;
    class ASTElseBlock;

    class ASTIfBlock : public ASTBlock {

        friend class SemaBuilder;

        enum ASTBlockKind StmtKind = ASTBlockKind::BLOCK_STMT_IF;

        // The If expression condition
        ASTExpr *Condition;

        // The list of Elseif Blocks
        std::vector<ASTElsifBlock *> ElsifBlocks;

        // The Else Block
        ASTElseBlock *ElseBlock = nullptr;

    public:
        ASTIfBlock(const SourceLocation &Loc, ASTExpr *Condition);

        std::vector<ASTElsifBlock *> getElsifBlocks();

        ASTElseBlock *getElseBlock();

        enum ASTBlockKind getBlockKind() const override;

        ASTExpr *getCondition();
    };

    class ASTElsifBlock : public ASTBlock {

        friend class SemaBuilder;

        enum ASTBlockKind StmtKind = ASTBlockKind::BLOCK_STMT_ELSIF;

        // The Else If expression condition
        ASTExpr *Condition;

    public:
        ASTElsifBlock(const SourceLocation &Loc, ASTExpr *Condition);

        enum ASTBlockKind getBlockKind() const override;

        ASTExpr *getCondition();
    };

    class ASTElseBlock : public ASTBlock {

        friend class SemaBuilder;

        enum ASTBlockKind StmtKind = ASTBlockKind::BLOCK_STMT_ELSE;

    public:
        ASTElseBlock(const SourceLocation &Loc);

        enum ASTBlockKind getBlockKind() const override;
    };
}


#endif //FLY_ASTIFBLOCK_H
