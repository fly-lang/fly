//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTVarAssignStmt.h - AST Variable Assign statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_ASTFAILSTMT_H
#define FLY_ASTFAILSTMT_H

#include "ASTStmt.h"

namespace fly {

    class ASTVarRef;

    class ASTFailStmt : public ASTStmt {

        friend class SemaBuilder;

        ASTExpr *Expr = nullptr;

        ASTFailStmt(ASTStmt *Parent, const SourceLocation &Loc);

    public:

        ASTExpr *getExpr() const;

        void setExpr(ASTExpr *);

        std::string str() const override;
    };
}

#endif //FLY_ASTFAILSTMT_H
