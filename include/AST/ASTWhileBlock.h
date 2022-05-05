//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTWhileBlock.h - AST While Block Statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTWHILEBLOCK_H
#define FLY_ASTWHILEBLOCK_H

#include "ASTBlock.h"

namespace fly {

    class ASTWhileBlock : public ASTBlock {

        friend class SemaBuilder;

        enum ASTBlockKind StmtKind = ASTBlockKind::BLOCK_STMT_WHILE;

        ASTExpr *Cond;

    public:
        ASTWhileBlock(const SourceLocation &Loc, ASTBlock *Parent, ASTExpr *Cond);

        enum ASTBlockKind getBlockKind() const override;

        ASTExpr *getCondition();

    };
}


#endif //FLY_ASTFORBLOCK_H
