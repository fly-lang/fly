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

    class ASTUnaryOpExpr;
    class ASTBinaryOpExpr;
    class ASTTernaryOpExpr;

    enum class Precedence {
        LOWEST,         // No operators
        ASSIGNMENT,     // =, +=, -=, etc.
        LOGICAL,        // ||, &&
        RELATIONAL,     // ==, !=, <, >, <=, >=
        ADDITIVE,       // +, -
        MULTIPLICATIVE, // *, /
        UNARY,          // -a, !a, ++a, --a, a++, a--
        PRIMARY         // Literals, Identifiers, Calls
    };

    enum class ASTUnaryOpExprKind {
        OP_UNARY_PRE_INCR,
        OP_UNARY_POST_INCR,
        OP_UNARY_PRE_DECR,
        OP_UNARY_POST_DECR,
        OP_UNARY_NOT_LOG
    };

    enum class ASTBinaryOpExprKind {

        // Arithmetic
        OP_BINARY_ADD,
        OP_BINARY_SUB,
        OP_BINARY_MUL,
        OP_BINARY_DIV,
        OP_BINARY_MOD,
        OP_BINARY_AND,
        OP_BINARY_OR ,
        OP_BINARY_XOR,
        OP_BINARY_SHIFT_L,
        OP_BINARY_SHIFT_R,

        // Logic
        OP_BINARY_AND_LOG,
        OP_BINARY_OR_LOG ,

        // Comparison
        OP_BINARY_EQ ,
        OP_BINARY_NE ,
        OP_BINARY_GT ,
        OP_BINARY_GTE,
        OP_BINARY_LT ,
        OP_BINARY_LTE,

        // ASSIGN
        OP_BINARY_ASSINGN,
        OP_BINARY_AND_ASSIGN,
        OP_BINARY_ASSIGN_MUL,
        OP_BINARY_ADD_ASSIGN,
        OP_BINARY_SUB_ASSIGN,
        OP_BINARY_DIV_ASSIGN,
        OP_BINARY_MOD_ASSIGN,
        OP_BINARY_SHIFT_L_ASSIGN,
        OP_BINARY_SHIFT_R_ASSIGN,
        OP_BINARY_XOR_ASSIGN,
        OP_BINARY_OR_ASSIGN
    };

    enum class ASTBinaryOpTypeExprKind {
        OP_BINARY_ARITH,
        OP_BINARY_COMPARISON,
        OP_BINARY_ASSIGN,
        OP_BINARY_LOGIC,
    };

    enum class ASTTernaryOpExprKind {
        OP_TERNARY_IF,
        OP_TERNARY_ELSE,
    };

    enum class ASTOpExprKind {
        OP_UNARY,
        OP_BINARY,
        OP_TERNARY
    };

    /**
     * Operator Expression
     */
    class ASTOpExpr : public ASTExpr {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        const ASTOpExprKind OpExprKind;

    protected:

        ASTOpExpr(const SourceLocation &Loc, ASTOpExprKind OpKind);

    public:

        virtual ASTOpExprKind getOpExprKind();
    };

    /**
     * Unary Operator Expression
     */
    class ASTUnaryOpExpr : public ASTOpExpr {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        SourceLocation OpLocation;

        ASTUnaryOpExprKind OpKind;

        const ASTExpr *Expr = nullptr;

        ASTUnaryOpExpr(const SourceLocation &Loc, ASTUnaryOpExprKind OpKind, ASTExpr *Expr);

    public:

        SourceLocation &getOpLocation();

        ASTUnaryOpExprKind getOpKind() const;

        const ASTExpr *getExpr() const;

        std::string str() const override;
    };

    /**
     * Binary Operator Expression
     */
    class ASTBinaryOpExpr : public ASTOpExpr {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTBinaryOpTypeExprKind TypeKind;

        ASTBinaryOpExprKind OpKind;

        SourceLocation OpLocation;

        ASTExpr *LeftExpr = nullptr;

        ASTExpr *RightExpr = nullptr;

        ASTBinaryOpExpr(ASTBinaryOpExprKind OpKind, const SourceLocation &OpLocation,
                        ASTExpr *LeftExpr, ASTExpr *RightExpr);

    public:

        ASTBinaryOpTypeExprKind setTypeKind(ASTBinaryOpExprKind OpKind);

        ASTBinaryOpTypeExprKind getTypeKind() const;

        ASTBinaryOpExprKind getOpKind() const;

        SourceLocation &getOpLocation();

        const ASTExpr *getLeftExpr() const;

        const ASTExpr *getRightExpr() const;

        std::string str() const override;
    };

    /**
     * Ternary Operator Expression
     */
    class ASTTernaryOpExpr : public ASTOpExpr {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTExpr *ConditionExpr;

        SourceLocation TrueOpLocation;

        ASTExpr *TrueExpr;

        SourceLocation FalseOpLocation;

        ASTExpr *FalseExpr;

        ASTTernaryOpExpr(ASTExpr *ConditionExpr, const SourceLocation &TrueOpLocation,
                         ASTExpr *TrueExpr, const SourceLocation &FalseOpLocation, ASTExpr *FalseExpr);

    public:

        const ASTExpr *getConditionExpr() const;

        SourceLocation &getTrueOpLocation();

        const ASTExpr *getTrueExpr() const;

        SourceLocation &getFalseOpLocation();

        const ASTExpr *getFalseExpr() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_GROUPEXPR_H
