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
    class ASTValue;
    class ASTType;

    class ASTSwitchBlock : public ASTBlock {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class ASTSwitchCaseBlock;

        // The Switch Expression
        ASTVarRef *VarRef = nullptr;

        // The Case Blocks
        llvm::SmallVector<ASTSwitchCaseBlock *, 8> Cases;

        // The Default Block
        ASTBlock *Default = nullptr;

        explicit ASTSwitchBlock(const SourceLocation &Loc);

    public:

        ASTVarRef *getVarRef() const;

        llvm::SmallVector<ASTSwitchCaseBlock *, 8> &getCases();

        ASTBlock *getDefault();

        std::string str() const override;
    };

    class ASTSwitchCaseBlock : public ASTBlock{

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTSwitchBlock *SwitchBlock;

        ASTType *Type = nullptr;

        ASTValue *Value = nullptr;

        explicit ASTSwitchCaseBlock(const SourceLocation &Loc);

    public:

        ASTType *getType() const;

        ASTValue *getValue();

        std::string str() const override;
    };
}


#endif //FLY_AST_SWITCHBLOCK_H
