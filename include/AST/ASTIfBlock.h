//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTIfBlock.h - AST If Block Statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_IFBLOCK_H
#define FLY_AST_IFBLOCK_H

#include "ASTBlock.h"

namespace fly {

    class ASTIfBlock;
    class ASTElsifBlock;
    class ASTElseBlock;

    class ASTIfBlock : public ASTBlock {

        friend class ASTElsifBlock;
        friend class ASTElseBlock;
        friend class SemaBuilder;
        friend class SemaResolver;

        // The If expression condition
        ASTExpr *Condition = nullptr;

        // The list of Elseif Blocks
        llvm::SmallVector<ASTElsifBlock *, 8> ElsifBlocks;

        // The Else Block
        ASTElseBlock *ElseBlock = nullptr;

        ASTIfBlock(ASTBlock *Parent, const SourceLocation &Loc);

    public:

        ASTBlock *getParent() const override;

        ASTExpr *getCondition();

        llvm::SmallVector<ASTElsifBlock *, 8>  getElsifBlocks();

        ASTElseBlock *getElseBlock();

        std::string str() const override;
    };

    class ASTElsifBlock : public ASTBlock {

        friend class SemaBuilder;
        friend class SemaResolver;

        // The If Block
        ASTIfBlock *IfBlock = nullptr;

        // The Else If expression condition
        ASTExpr *Condition = nullptr;

        ASTElsifBlock(ASTIfBlock *IfBlock, const SourceLocation &Loc);

    public:

        ASTExpr *getCondition();

        std::string str() const override;
    };

    class ASTElseBlock : public ASTBlock {

        friend class SemaBuilder;
        friend class SemaResolver;

        // The If Block
        ASTIfBlock *IfBlock = nullptr;

        ASTElseBlock(ASTIfBlock *IfBlock, const SourceLocation &Loc);

        std::string str() const override;
    };
}


#endif //FLY_AST_IFBLOCK_H
