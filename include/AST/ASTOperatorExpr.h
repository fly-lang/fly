//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTOperator.h - AST Operator Expr header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_OPERATOREXPR_H
#define FLY_AST_OPERATOREXPR_H

#include "AST/ASTExpr.h"

namespace fly {

    enum class ASTUnaryOperatorKind {
        UNARY_ARITH_PRE_INCR,
        UNARY_ARITH_POST_INCR,
        UNARY_ARITH_PRE_DECR,
        UNARY_ARITH_POST_DECR,
        UNARY_LOGIC_NOT
    };

    enum class ASTBinaryOptionKind {
        BINARY_ARITH,
        BINARY_LOGIC,
        BINARY_COMPARISON
    };

    enum class ASTBinaryOperatorKind {

        // Arithmetic
        BINARY_ARITH_ADD = 101,
        BINARY_ARITH_SUB = 102,
        BINARY_ARITH_MUL = 103,
        BINARY_ARITH_DIV = 104,
        BINARY_ARITH_MOD = 105,
        BINARY_ARITH_AND = 106,
        BINARY_ARITH_OR  = 107,
        BINARY_ARITH_XOR = 108,
        BINARY_ARITH_SHIFT_L = 109,
        BINARY_ARITH_SHIFT_R = 110,

        // Logic
        BINARY_LOGIC_AND = 201,
        BINARY_LOGIC_OR  = 202,

        // Comparison
        BINARY_COMP_EQ  = 301,
        BINARY_COMP_NE  = 302,
        BINARY_COMP_GT  = 303,
        BINARY_COMP_GTE = 304,
        BINARY_COMP_LT  = 305,
        BINARY_COMP_LTE = 306
    };

    enum class ASTTernaryOperatorKind {
        TERNARY_IF,
        TERNARY_ELSE,
    };

    class ASTOperatorExpr : public ASTExpr {

    public:
        explicit ASTOperatorExpr(const SourceLocation &Loc);

    };

    /**
     * AST Unary Operator
     */
    class ASTUnaryOperatorExpr : public ASTOperatorExpr {

        ASTUnaryOperatorKind OperatorKind;

    public:
        ASTUnaryOperatorExpr(const SourceLocation &Loc, ASTUnaryOperatorKind Op);

        ASTUnaryOperatorKind getOperatorKind() const;

        std::string str() const override;
    };

    /**
     * AST Binary Operator
     */
    class ASTBinaryOperatorExpr : public ASTOperatorExpr {

        ASTBinaryOperatorKind OperatorKind;

        ASTBinaryOptionKind OptionKind;

        bool Precedence;

    public:
        ASTBinaryOperatorExpr(const SourceLocation &Loc, ASTBinaryOperatorKind Op);

        ASTBinaryOperatorKind getOperatorKind() const;

        ASTBinaryOptionKind getOptionKind() const;

        bool isPrecedence() const;

        std::string str() const override;
    };

    /**
     * AST Ternary Operator
     */
    class ASTTernaryOperatorExpr : public ASTOperatorExpr {

        ASTTernaryOperatorKind OperatorKind;

    public:
        ASTTernaryOperatorExpr(const SourceLocation &Loc, ASTTernaryOperatorKind Op);

        ASTTernaryOperatorKind getOperatorKind() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_OPERATOREXPR_H