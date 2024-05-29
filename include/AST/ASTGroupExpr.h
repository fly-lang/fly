//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTGroupExpr.h - AST Group Expression header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_GROUPEXPR_H
#define FLY_AST_GROUPEXPR_H

#include "ASTExpr.h"

namespace fly {

    class ASTUnaryOperatorExpr;
    class ASTBinaryOperatorExpr;
    class ASTTernaryOperatorExpr;

    enum class ASTExprGroupKind : char {
        GROUP_UNARY,
        GROUP_BINARY,
        GROUP_TERNARY
    };

    /**
     * Group Expression
     */
    class ASTGroupExpr : public ASTExpr {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        const ASTExprGroupKind GroupKind;

    protected:

        ASTGroupExpr(const SourceLocation &Loc, ASTExprGroupKind GroupKind);

    public:

        virtual ASTExprGroupKind getGroupKind();
    };

    /**
     * Unary Group Expression
     */
    class ASTUnaryGroupExpr : public ASTGroupExpr {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTUnaryOperatorExpr *Operator;

        const ASTVarRefExpr *First = nullptr;

        ASTUnaryGroupExpr(const SourceLocation &Loc, ASTUnaryOperatorExpr *Operator, ASTVarRefExpr *First);

    public:

        ASTUnaryOperatorExpr *getOperator() const;

        const ASTVarRefExpr *getFirst() const;

        std::string str() const override;
    };

    /**
     * Binary Group Expression
     */
    class ASTBinaryGroupExpr : public ASTGroupExpr {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTBinaryOperatorExpr *Operator;

        ASTExpr *First = nullptr;

        ASTExpr *Second = nullptr;

        ASTBinaryGroupExpr(const SourceLocation &Loc, ASTBinaryOperatorExpr *Operator, ASTExpr *First, ASTExpr *Second);

    public:

        ASTBinaryOperatorExpr *getOperator() const;

        const ASTExpr *getFirst() const;

        const ASTExpr *getSecond() const;

        std::string str() const override;
    };

    /**
     * Ternary Group Expression
     */
    class ASTTernaryGroupExpr : public ASTGroupExpr {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        // Only Ternary Condition (if ? then : else)
        ASTTernaryOperatorExpr *FirstOperator;

        ASTTernaryOperatorExpr *SecondOperator;

        ASTExpr *First = nullptr;

        ASTExpr *Second = nullptr;

        ASTExpr *Third = nullptr;

        ASTTernaryGroupExpr(const SourceLocation &Loc, ASTExpr *First, ASTTernaryOperatorExpr *FirstOperator,
                            ASTExpr *Second, ASTTernaryOperatorExpr *SecondOperator, ASTExpr *Third);

    public:

        ASTTernaryOperatorExpr *getFirstOperator() const;

        ASTTernaryOperatorExpr *getSecondOperator() const;

        const ASTExpr *getFirst() const;

        const ASTExpr *getSecond() const;

        const ASTExpr *getThird() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_GROUPEXPR_H
