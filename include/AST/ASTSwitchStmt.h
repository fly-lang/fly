//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTSwitchStmt.h - AST switch statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_SWITCHSTMT_H
#define FLY_AST_SWITCHSTMT_H

#include "ASTRuleStmt.h"
#include "ASTStmt.h"

namespace fly {

    class ASTIdentifier;

    class ASTSwitchStmt : public ASTStmt {

        friend class ASTBuilderSwitchStmt;

        // The Switch Expression
        ASTExpr *Expr = nullptr;

        // The Case Blocks
        llvm::SmallVector<ASTRuleStmt *, 8> Cases;

        // The Default Block
        ASTStmt *Default = nullptr;

        explicit ASTSwitchStmt(const SourceLocation &Loc);

    public:

        void accept(ASTVisitor& Visitor) override;

        ASTExpr *getExpr() const;

        llvm::SmallVector<ASTRuleStmt *, 8> &getCases();

        ASTStmt *getDefault();

        std::string str() const override;
    };
}


#endif //FLY_AST_SWITCHSTMT_H
