//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTDeleteStmt.h - AST delete statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_DELETESTMT_H
#define FLY_AST_DELETESTMT_H

#include "ASTStmt.h"

namespace fly {

    class ASTBlockStmt;
    class ASTIdentifier;

/**
 * Delete Stmt
 */
    class ASTDeleteStmt : public ASTStmt {

        friend class ASTBuilder;

        ASTExpr *Expr = nullptr;

        ASTDeleteStmt(const SourceLocation &Loc, ASTExpr *Expr);

    public:

        void accept(ASTVisitor& Visitor) override;

        ASTExpr *getExpr();

        std::string str() const override;
    };
}
#endif //FLY_AST_DELETESTMT_H