//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTVarAssignStmt.h - AST Variable Assign statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_ASTVARASSIGN_H
#define FLY_ASTVARASSIGN_H

#include "ASTStmt.h"

namespace fly {

    class ASTVarRef;

    /**
     * Assign somethings to a Local Var
     * Ex.
     *  a = 1
     */
    class ASTVarAssign : public ASTStmt {

        ASTVarRef *VarRef;

        ASTExpr *Expr;

    public:

        ASTVarAssign(const SourceLocation &Loc, ASTVarRef *VarRef, ASTExpr *Expr);

        StmtKind getKind() const override;

        ASTVarRef *getVarRef() const;

        ASTExpr *getExpr() const;

        std::string str() const override;
    };
}

#endif //FLY_ASTVARASSIGN_H
