//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTVarStmt.h - AST Variable statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_VARSTMT_H
#define FLY_AST_VARSTMT_H

#include "ASTStmt.h"

namespace fly {

    class ASTVarRef;

    enum class ASTAssignOperatorKind {
        EQUAL,
        PLUS_EQUAL,
        MINUS_EQUAL,
        AMP_EQUAL,
        PIPE_EQUAL,
        STAR_EQUAL,
        SLASH_EQUAL,
        PERCENT_EQUAL,
        LESSLESS_EQUAL,
        GREATERGREATER_EQUAL,
        CARET_EQUAL
    };

    /**
     * Assign somethings to a Local Var
     * Ex.
     *  a = 1
     */
    class ASTAssignmentStmt : public ASTStmt {

        friend class ASTBuilder;
        friend class SemaBuilderStmt;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTVarRef *VarRef;

        ASTAssignOperatorKind Kind;

        ASTExpr *Expr = nullptr;

        ASTAssignmentStmt(const SourceLocation &Loc, ASTVarRef *VarRef, ASTAssignOperatorKind AssignOperatorKind);

    public:

        ASTVarRef *getVarRef() const;

        ASTAssignOperatorKind getKind1() const;

        ASTExpr *getExpr() const;

        void setExpr(ASTExpr *);

        std::string str() const override;
    };
}

#endif //FLY_AST_VARSTMT_H
