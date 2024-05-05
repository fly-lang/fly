//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTWhileBlock.h - AST While Block Statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_WHILEBLOCK_H
#define FLY_AST_WHILEBLOCK_H

#include "ASTBlock.h"

namespace fly {

    class ASTLoopBlock : public ASTBlock {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTExpr *Condition = nullptr;

        bool VerifyConditionOnEnd = false;

        ASTBlock *Init = nullptr;

        ASTBlock *Post = nullptr;

        explicit ASTLoopBlock(const SourceLocation &Loc);

    public:

        ASTBlock *getParent() const override;

        ASTExpr *getCondition();

        bool isVerifyConditionOnEnd() const;

        ASTBlock *getInit() const;

        ASTBlock *getPost() const;

        std::string str() const override;

    };
}

#endif //FLY_AST_WHILEBLOCK_H
