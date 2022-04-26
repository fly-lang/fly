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

        friend class Parser;
        friend class ASTElsifBlock;
        friend class ASTElseBlock;

        enum ASTBlockKind StmtKind = ASTBlockKind::BLOCK_STMT_IF;

        // The If expression condition
        ASTExpr *Condition;

        // The list of Elseif Blocks
        std::vector<ASTElsifBlock *> ElsifBlocks;

        // The Else Block
        ASTElseBlock *ElseBlock = nullptr;

    protected:
        ASTIfBlock(const SourceLocation &Loc, ASTBlock *Parent);

    public:
        ASTIfBlock(const SourceLocation &Loc, ASTBlock *Parent, ASTExpr *Condition);

        ASTElsifBlock *AddElsifBlock(const SourceLocation &Loc, ASTExpr *Expr);

        std::vector<ASTElsifBlock *> getElsifBlocks();

        ASTElseBlock *AddElseBlock(const SourceLocation &Loc);

        ASTElseBlock *getElseBlock();

        enum ASTBlockKind getBlockKind() const override;

        ASTExpr *getCondition();
    };

    class ASTElsifBlock : public ASTIfBlock {

        friend class Parser;
        friend class ASTIfBlock;

        enum ASTBlockKind StmtKind = ASTBlockKind::BLOCK_STMT_ELSIF;

    public:
        ASTElsifBlock(const SourceLocation &Loc, ASTBlock *Parent, ASTExpr *Condition);

        enum ASTBlockKind getBlockKind() const override;
    };

    class ASTElseBlock : public ASTIfBlock {

        friend class Parser;
        friend class ASTIfBlock;

        enum ASTBlockKind StmtKind = ASTBlockKind::BLOCK_STMT_ELSE;

    public:
        ASTElseBlock(const SourceLocation &Loc, ASTBlock *Parent);

        enum ASTBlockKind getBlockKind() const override;
    };
}


#endif //FLY_ASTIFBLOCK_H
