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

        ASTExpr *Expr = nullptr;

    public:
        ASTExprStmt(const SourceLocation &Loc);

        StmtKind getKind() const override;

        ASTExpr *getExpr() const;

        void setExpr(ASTExpr *E);

        std::string str() const override;
    };
}


#endif //FLY_ASTEXPRSTMT_H
