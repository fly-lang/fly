//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTExpr.h - AST Expression header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_EXPR_H
#define FLY_AST_EXPR_H

#include "ASTNode.h"

namespace fly {

    class SemaType;

    enum class ASTExprKind : char {
        EXPR_VALUE,
        EXPR_VAR_REF,
        EXPR_CALL,
        EXPR_NEW,
        EXPR_OP,
        EXPR_CAST
    };

    /**
     * Expression Abstract Class
     */
    class ASTExpr : public ASTNode {

        friend class ASTBuilder;

    protected:

        const ASTExprKind ExprKind;

        SemaType *Type = nullptr;

        ASTExpr(const SourceLocation &Loc, ASTExprKind Kind);

    public:

        SemaType *getType() const;

        ASTExprKind getExprKind() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_EXPR_H
