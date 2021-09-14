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
        ASTExpr *Condition;
        std::vector<ASTElsifBlock *> Elsif;
        ASTElseBlock *Else = nullptr;

    protected:
        ASTIfBlock(const SourceLocation &Loc, ASTBlock *Parent);

    public:
        ASTIfBlock(const SourceLocation &Loc, ASTBlock *Parent, ASTExpr *Condition);

        static bool AddBranch(ASTBlock *Parent, ASTIfBlock *Cond);

        enum ASTBlockKind getBlockKind() const override;

        ASTExpr *getCondition();

        std::vector<ASTElsifBlock *> getElsif();

        ASTElseBlock *getElse();
    };

    class ASTElsifBlock : public ASTIfBlock {

        friend class Parser;
        friend class ASTIfBlock;

        enum ASTBlockKind StmtKind = ASTBlockKind::BLOCK_STMT_ELSIF;
        ASTIfBlock *Head;

    public:
        ASTElsifBlock(const SourceLocation &Loc, ASTBlock *Parent, ASTExpr *Condition);

        enum ASTBlockKind getBlockKind() const override;
    };

    class ASTElseBlock : public ASTIfBlock {

        friend class Parser;
        friend class ASTIfBlock;

        enum ASTBlockKind StmtKind = ASTBlockKind::BLOCK_STMT_ELSE;
        ASTIfBlock *Head;

    public:
        ASTElseBlock(const SourceLocation &Loc, ASTBlock *Parent);

        enum ASTBlockKind getBlockKind() const override;
    };
}


#endif //FLY_ASTIFBLOCK_H
