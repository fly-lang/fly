//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/SwitchBlock.h - For Block Statements
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_ASTSWITCHBLOCK_H
#define FLY_ASTSWITCHBLOCK_H


#include "ASTBlock.h"

namespace fly {

    class ASTSwitchCaseBlock;
    class ASTSwitchDefaultBlock;

    class ASTSwitchBlock : public ASTBlock {

        friend class SemaBuilder;

        // The Stmt Block Kind
        enum ASTBlockKind StmtKind = ASTBlockKind::BLOCK_STMT_SWITCH;

        // The Switch Expression
        ASTExpr *Expr;

        // The Case Blocks
        std::vector<ASTSwitchCaseBlock *> Cases;

        // The Default Block
        ASTSwitchDefaultBlock *Default = nullptr;

        ASTSwitchBlock(const SourceLocation &Loc, ASTExpr *Expr);

    public:

        ASTExpr *getExpr() const;

        enum ASTBlockKind getBlockKind() const override;

        std::vector<ASTSwitchCaseBlock *> &getCases();

        ASTSwitchDefaultBlock *getDefault();
    };

    class ASTSwitchCaseBlock : public ASTBlock{

        friend class SemaBuilder;

        enum ASTBlockKind StmtKind = ASTBlockKind::BLOCK_STMT_CASE;
        ASTExpr *Expr;

    public:
        ASTSwitchCaseBlock(const SourceLocation &Loc, ASTExpr *Value);

        ASTExpr *getExpr();

        enum ASTBlockKind getBlockKind() const override;
    };

    class ASTSwitchDefaultBlock : public ASTBlock {

        friend class SemaBuilder;

        enum ASTBlockKind StmtKind = ASTBlockKind::BLOCK_STMT_DEFAULT;

    public:
        ASTSwitchDefaultBlock(const SourceLocation &Loc);

        enum ASTBlockKind getBlockKind() const override;
    };
}


#endif //FLY_ASTSWITCHBLOCK_H
