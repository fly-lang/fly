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

#include "ASTRuleStmt.h"
#include "ASTStmt.h"

namespace fly {

    class ASTValueExpr;
    class ASTRef;

    class ASTSwitchStmt : public ASTStmt {

        friend class ASTBuilder;

        // The Switch Expression
        ASTRef *Ref = nullptr;

        // The Case Blocks
        llvm::SmallVector<ASTRuleStmt *, 8> Cases;

        // The Default Block
        ASTStmt *Default = nullptr;

        explicit ASTSwitchStmt(const SourceLocation &Loc);

    public:

        void accept(ASTVisitor& Visitor) override;

        ASTRef *getRef() const;

        llvm::SmallVector<ASTRuleStmt *, 8> &getCases();

        ASTStmt *getDefault();

        std::string str() const override;
    };
}


#endif //FLY_AST_SWITCHBLOCK_H
