//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTCaseStmt.h - AST case statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_CASESTMT_H
#define FLY_AST_CASESTMT_H

#include "ASTStmt.h"

namespace fly {

    class ASTExpr;

    // Represents a single "case" clause.
    // Used in two contexts:
    //   1. As a child of ASTSwitchStmt (Expr = integer/enum dispatch value)
    //   2. As a standalone statement in a suite test-method block
    //      (Expr = string-literal label, validated by Sema)
    class ASTCaseStmt : public ASTStmt {

        friend class ASTBuilder;
        friend class ASTBuilderSwitchStmt;

        ASTExpr *Expr = nullptr;   // case value or string label
        ASTStmt *Stmt = nullptr;   // body block

        explicit ASTCaseStmt(const SourceLocation &Loc);

    public:

        void accept(ASTVisitor& Visitor) override;

        ASTExpr *getExpr() const;

        ASTStmt *getStmt() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_CASESTMT_H
