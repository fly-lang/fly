//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTExprStmt.h - AST Expression Statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_EXPRSTMT_H
#define FLY_AST_EXPRSTMT_H

#include "ASTStmt.h"

namespace fly {

    class ASTExprStmt : public ASTStmt {

        friend class SemaResolver;
        friend class SemaBuilder;

    protected:

        ASTExpr *Expr = nullptr;

        explicit ASTExprStmt(const SourceLocation &Loc);

        ASTExprStmt(const SourceLocation &Loc, ASTStmtKind Kind);

    public:

        ASTExpr *getExpr() const;

        std::string str() const override;
    };
}


#endif //FLY_AST_EXPRSTMT_H
