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

        enum ASTBlockKind StmtKind = ASTBlockKind::BLOCK_STMT_SWITCH;
        ASTExpr *Expr;
        std::vector<ASTSwitchCaseBlock *> Cases;
        ASTSwitchDefaultBlock *Default = nullptr;

    public:
        ASTSwitchBlock(const SourceLocation &Loc, ASTBlock *Parent, ASTExpr *Expr);

        ASTExpr *getExpr() const;

        ASTSwitchCaseBlock * AddCase(const SourceLocation &Loc, ASTExpr *Value);

        ASTSwitchDefaultBlock * setDefault(const SourceLocation &Loc);

        enum ASTBlockKind getBlockKind() const override;

        std::vector<ASTSwitchCaseBlock *> &getCases();

        ASTSwitchDefaultBlock *getDefault();
    };

    class ASTSwitchCaseBlock : public ASTBlock{

        enum ASTBlockKind StmtKind = ASTBlockKind::BLOCK_STMT_CASE;
        ASTExpr *Expr;

    public:
        ASTSwitchCaseBlock(const SourceLocation &Loc, ASTSwitchBlock *Switch, ASTExpr *Value);

        ASTExpr *getExpr();

        enum ASTBlockKind getBlockKind() const override;
    };

    class ASTSwitchDefaultBlock : public ASTBlock{

        enum ASTBlockKind StmtKind = ASTBlockKind::BLOCK_STMT_DEFAULT;

    public:
        ASTSwitchDefaultBlock(const SourceLocation &Loc, ASTSwitchBlock *Switch);

        enum ASTBlockKind getBlockKind() const override;
    };
}


#endif //FLY_ASTSWITCHBLOCK_H
