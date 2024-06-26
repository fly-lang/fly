//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTVarAssignStmt.h - AST Variable Assign statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_ASTVARSTMT_H
#define FLY_ASTVARSTMT_H

#include "ASTStmt.h"

namespace fly {

    class ASTVarRef;

    /**
     * Assign somethings to a Local Var
     * Ex.
     *  a = 1
     */
    class ASTVarStmt : public ASTStmt {

        friend class SemaBuilder;

        ASTVarRef *VarRef = nullptr;

        ASTExpr *Expr = nullptr;

        ASTBlock *Block = nullptr;

        ASTVarStmt(ASTBlock *Parent, const SourceLocation &Loc, ASTVarRef *VarRef);

    public:

        ASTVarRef *getVarRef() const;

        ASTExpr *getExpr() const;

        void setExpr(ASTExpr *);

        ASTBlock *getBlock() const;

        std::string str() const;
    };
}

#endif //FLY_ASTVARSTMT_H
