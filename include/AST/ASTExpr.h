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

#include <Sema/SemaType.h>

#include "ASTNode.h"

namespace fly {

    class SemaExpr;

    enum class ASTExprKind : char {
        EXPR_VALUE,
        EXPR_IDENTIFIER,
        EXPR_MEMBER,
        EXPR_CALL,
        EXPR_UNARY,
        EXPR_BINARY,
        EXPR_TERNARY,
        EXPR_CAST
    };

    /**
     * Expression Abstract Class
     */
    class ASTExpr : public ASTNode {

        friend class ASTBuilder;

    protected:

        const ASTExprKind ExprKind;

        ASTExpr *Parent;

        ASTExpr *Child;

        SemaExpr *Sema;

        SemaType *Type;

        ASTExpr(const SourceLocation &Loc, ASTExprKind Kind, ASTExpr *Parent = nullptr, ASTExpr *Child = nullptr);

    public:

        ASTExprKind getExprKind() const;

        ASTExpr *getParent() const;

        ASTExpr *getChild() const;

        void setParent(ASTExpr *Parent);

        void setChild(ASTExpr *Child);

        virtual SemaExpr *getSema() const;

        void setSema(SemaExpr *ExpSemar);

        SemaType *getType() const;

        void setType(SemaType *Type);

        std::string str() const override;
    };
}

#endif //FLY_AST_EXPR_H
