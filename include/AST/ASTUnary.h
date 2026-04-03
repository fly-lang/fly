//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTUnary.h - AST Unary Expression header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_UNARY_H
#define FLY_AST_UNARY_H

#include "ASTExpr.h"

namespace fly {

    enum class ASTUnaryKind {
        OP_UNARY_PRE_INCR,
        OP_UNARY_POST_INCR,
        OP_UNARY_PRE_DECR,
        OP_UNARY_POST_DECR,
        OP_UNARY_NOT_LOG
    };

    /**
     * Unary Operator Expression
     */
    class ASTUnary : public ASTExpr {

        friend class ASTBuilder;

        ASTUnaryKind OpKind;

        ASTExpr *Expr;

        ASTUnary(const SourceLocation &Loc, ASTUnaryKind OpKind, ASTExpr *Expr);

    public:

        void accept(ASTVisitor& Visitor) override;

        const SourceLocation &getOpLocation() const;

        ASTUnaryKind getOpKind() const;

        ASTExpr *getExpr() const;


        std::string str() const override;
    };
}

#endif //FLY_AST_UNARY_H
