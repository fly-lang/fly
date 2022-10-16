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

    class ASTForLoopBlock;
    class ASTForPostBlock;

    class ASTForBlock : public ASTBlock {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class ASTForLoopBlock;
        friend class ASTForPostBlock;

        ASTExpr *Condition = nullptr;
        ASTForPostBlock *Post = nullptr;
        ASTForLoopBlock *Loop = nullptr;

        ASTForBlock(ASTBlock *Parent, const SourceLocation &Loc);

    public:

        virtual ~ASTForBlock();

        ASTBlock *getParent() const;

        ASTExpr *getCondition();

        ASTForPostBlock *getPost();

        ASTForLoopBlock *getLoop();
    };

    class ASTForLoopBlock : public ASTBlock {

        friend class SemaBuilder;
        friend class SemaResolver;

        // The For Block
        ASTForBlock *ForBlock = nullptr;

        ASTForLoopBlock(ASTForBlock *ForBlock, const SourceLocation &Loc);
    };

    class ASTForPostBlock : public ASTBlock {

        friend class SemaBuilder;
        friend class SemaResolver;

        // The For Block
        ASTForBlock *ForBlock = nullptr;

        ASTForPostBlock(ASTForBlock *ForBlock, const SourceLocation &Loc);

        std::string str() const;
    };
}


#endif //FLY_ASTFORBLOCK_H
