//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTVarAssignStmt.h - AST Variable Assign statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_ASTVARDEFINE_H
#define FLY_ASTVARDEFINE_H

#include "ASTStmt.h"

namespace fly {

    class ASTVarRef;

    /**
     * Assign somethings to a Local Var
     * Ex.
     *  a = 1
     */
    class ASTVarDefine : public ASTStmt {

        friend class SemaBuilder;

        ASTVarRef *VarRef = nullptr;

        ASTExpr *Expr = nullptr;

        ASTBlock *Block = nullptr;

        bool FirstDefined = false;

        ASTVarDefine(ASTBlock *Parent, const SourceLocation &Loc, ASTVarRef *VarRef, bool Init = false);

    public:

        ASTVarRef *getVarRef() const;

        ASTExpr *getExpr() const;

        ASTBlock *getBlock() const;

        bool isFirstDefined() const;

        std::string str() const;
    };
}

#endif //FLY_ASTVARDEFINE_H
