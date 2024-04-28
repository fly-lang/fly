//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/SwitchBlock.h - AST Switch Block Statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_SWITCHBLOCK_H
#define FLY_AST_SWITCHBLOCK_H

#include "ASTBlock.h"

namespace fly {

    class ASTSwitchCaseBlock;
    class ASTSwitchDefaultBlock;

    class ASTSwitchBlock : public ASTBlock {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class ASTSwitchCaseBlock;
        friend class ASTSwitchDefaultBlock;

        // The Switch Expression
        ASTExpr *Expr = nullptr;

        // The Case Blocks
        llvm::SmallVector<ASTSwitchCaseBlock *, 8> Cases;

        // The Default Block
        ASTSwitchDefaultBlock *Default = nullptr;

        ASTSwitchBlock(ASTBlock *Parent, const SourceLocation &Loc);

    public:

        ASTBlock *getParent() const override;

        ASTExpr *getExpr() const;

        llvm::SmallVector<ASTSwitchCaseBlock *, 8> &getCases();

        ASTSwitchDefaultBlock *getDefault();

        std::string str() const override;
    };

    class ASTSwitchCaseBlock : public ASTBlock{

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTSwitchBlock *SwitchBlock;

        ASTExpr *Expr = nullptr;

        ASTSwitchCaseBlock(ASTSwitchBlock *SwitchBlock, const SourceLocation &Loc);

    public:

        ASTExpr *getExpr();

        std::string str() const override;
    };

    class ASTSwitchDefaultBlock : public ASTBlock {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTSwitchBlock *SwitchBlock;

        ASTSwitchDefaultBlock(ASTSwitchBlock *SwitchBlock, const SourceLocation &Loc);

    public:

        std::string str() const override;

    };
}


#endif //FLY_AST_SWITCHBLOCK_H
