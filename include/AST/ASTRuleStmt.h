//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTRuleStmt.h - AST Rule Statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_RULESTMT_H
#define FLY_AST_RULESTMT_H

#include "ASTStmt.h"

namespace fly {

    class ASTRuleStmt : public ASTStmt {

        friend class ASTBuilder;
        friend class SemaBuilderIfStmt;
        friend class SemaBuilderSwitchStmt;
        friend class SemaBuilderLoopStmt;

        // The Rule expression can be used with different Modifiers
        ASTExpr *Rule = nullptr;

        // The If Block statement
        ASTStmt *Stmt = nullptr;

    protected:

        explicit ASTRuleStmt(const SourceLocation &Loc);

        ASTRuleStmt(const SourceLocation &Loc, ASTStmtKind Kind);

    public:

        void accept(ASTVisitor& Visitor) override;

        ASTExpr *getRule();

        ASTStmt *getStmt() const;

        std::string str() const override;
    };
}


#endif //FLY_AST_IFSTMT_H
