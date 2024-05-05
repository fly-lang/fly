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

    class ASTIfBlock : public ASTBlock {

        friend class ASTElsifBlock;
        friend class SemaBuilder;
        friend class SemaResolver;

        // The If expression condition
        ASTExpr *Condition = nullptr;

        // The list of Elseif Blocks
        llvm::SmallVector<ASTElsifBlock *, 8> ElsifBlocks;

        // The Else Block
        ASTBlock *ElseBlock = nullptr;

        explicit ASTIfBlock(const SourceLocation &Loc);

    public:

        ASTBlock *getParent() const override;

        ASTExpr *getCondition();

        llvm::SmallVector<ASTElsifBlock *, 8>  getElsifBlocks();

        ASTBlock *getElseBlock();

        std::string str() const override;
    };

    class ASTElsifBlock : public ASTBlock {

        friend class SemaBuilder;
        friend class SemaResolver;

        // The If Block
        ASTIfBlock *IfBlock = nullptr;

        // The Else If expression condition
        ASTExpr *Condition = nullptr;

        explicit ASTElsifBlock(const SourceLocation &Loc);

    public:

        ASTExpr *getCondition();

        std::string str() const override;
    };
}


#endif //FLY_AST_IFBLOCK_H
