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

#include "ASTExprStmt.h"

namespace fly {

    class ASTVarRef;

    /**
     * Assign somethings to a Local Var
     * Ex.
     *  a = 1
     */
    class ASTVarAssign : public ASTExprStmt {

        friend class SemaBuilder;

        ASTVarRef *VarRef;

        ASTVarAssign(const SourceLocation &Loc, ASTVarRef *VarRef);

    public:

        StmtKind getKind() const override;

        ASTVarRef *getVarRef() const;

        std::string str() const override;
    };
}

#endif //FLY_ASTVARASSIGN_H
