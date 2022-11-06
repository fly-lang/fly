//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTStmt.h - AST Statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTEXPRSTMT_H
#define FLY_ASTEXPRSTMT_H

#include "ASTStmt.h"

namespace fly {

    class ASTExprStmt : public ASTStmt {

        friend class SemaResolver;
        friend class SemaBuilder;

    protected:

        ASTExpr *Expr = nullptr;

        ASTExprStmt(ASTStmt *Parent, const SourceLocation &Loc);

        ASTExprStmt(ASTStmt *Parent, const SourceLocation &Loc, ASTStmtKind Kind);

    public:

        ASTExpr *getExpr() const;

        std::string str() const override;
    };
}


#endif //FLY_ASTEXPRSTMT_H
