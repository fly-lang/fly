//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTOp.h - AST Group Expression header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_OP_H
#define FLY_AST_OP_H

#include "ASTExpr.h"
#include "Sema/SemaBinary.h"
#include "Sema/SemaTernary.h"
#include "Sema/SemaUnary.h"

namespace fly {

    enum class Precedence {
        LOWEST,         // No operators
        ASSIGNMENT,     // =, +=, -=, etc.
        TERNARY,        // ?
        LOGICAL,        // ||, &&
        RELATIONAL,     // ==, !=, <, >, <=, >=
        ADDITIVE,       // +, -
        MULTIPLICATIVE, // *, /
        UNARY,          // -a, !a, ++a, --a, a++, a--
        PRIMARY         // Literals, Identifiers, Calls
    };

    enum class ASTUnaryOpKind {
        OP_UNARY_PRE_INCR,
        OP_UNARY_POST_INCR,
        OP_UNARY_PRE_DECR,
        OP_UNARY_POST_DECR,
        OP_UNARY_NOT_LOG
    };

    enum class ASTBinaryOpKind {

        // Arithmetic
        OP_BINARY_ARITH_ADD,
        OP_BINARY_ARITH_SUB,
        OP_BINARY_ARITH_MUL,
        OP_BINARY_ARITH_DIV,
        OP_BINARY_ARITH_MOD,
        OP_BINARY_ARITH_AND,
        OP_BINARY_ARITH_OR ,
        OP_BINARY_ARITH_XOR,
        OP_BINARY_ARITH_SHIFT_L,
        OP_BINARY_ARITH_SHIFT_R,

        // Logic
        OP_BINARY_LOGIC_AND,
        OP_BINARY_LOGIC_OR ,

        // Comparison
        OP_BINARY_COMPARE_EQ ,
        OP_BINARY_COMPARE_NE ,
        OP_BINARY_COMPARE_GT ,
        OP_BINARY_COMPARE_GTE,
        OP_BINARY_COMPARE_LT ,
        OP_BINARY_COMPARE_LTE,

        // ASSIGN
        OP_BINARY_ASSIGN,
        OP_BINARY_ASSIGN_AND,
        OP_BINARY_ASSIGN_MUL,
        OP_BINARY_ASSIGN_ADD,
        OP_BINARY_ASSIGN_SUB,
        OP_BINARY_ASSIGN_DIV,
        OP_BINARY_ASSIGN_MOD,
        OP_BINARY_ASSIGN_SHIFT_L,
        OP_BINARY_ASSIGN_SHIFT_R,
        OP_BINARY_ASSIGN_XOR,
        OP_BINARY_ASSIGN_OR
    };

    enum class ASTBinaryKind {
        OP_BINARY_ARITH,
        OP_BINARY_COMPARE,
        OP_BINARY_ASSIGN,
        OP_BINARY_LOGIC,
    };

    /**
     * Unary Operator Expression
     */
    class ASTUnaryOp : public ASTExpr {

        friend class ASTBuilder;

        ASTUnaryOpKind OpKind;

        ASTExpr *Expr;

        ASTUnaryOp(const SourceLocation &Loc, ASTUnaryOpKind OpKind, ASTExpr *Expr);

    public:

        void accept(ASTVisitor& Visitor) override;

        const SourceLocation &getOpLocation() const;

        ASTUnaryOpKind getOpKind() const;

        ASTExpr *getExpr() const;

        SemaUnary *getSema() const override;

    	void setSema(SemaUnary *Sema);

        std::string str() const override;
    };

    /**
     * Binary Operator Expression
     */
    class ASTBinaryOp : public ASTExpr {

        friend class ASTBuilder;

        ASTBinaryKind BinaryKind;

        ASTBinaryOpKind OpKind;

        SourceLocation OpLocation;

        ASTExpr *LeftExpr = nullptr;

        ASTExpr *RightExpr = nullptr;

        ASTBinaryOp(ASTBinaryOpKind OpKind, const SourceLocation &OpLocation,
                        ASTExpr *LeftExpr, ASTExpr *RightExpr);

        ASTBinaryKind setBinaryKind(ASTBinaryOpKind OpKind);

    public:

        void accept(ASTVisitor& Visitor) override;

        ASTBinaryKind getBinaryKind() const;

        ASTBinaryOpKind getOpKind() const;

        SourceLocation &getOpLocation();

        ASTExpr *getLeftExpr() const;

        ASTExpr *getRightExpr() const;

        SemaBinary *getSema() const override;

    	void setSema(SemaBinary *Sema);

        std::string str() const override;
    };

    /**
     * Ternary Operator Expression
     */
    class ASTTernaryOp : public ASTExpr {

        friend class ASTBuilder;

        ASTExpr *ConditionExpr;

        SourceLocation TrueOpLocation;

        ASTExpr *TrueExpr;

        SourceLocation FalseOpLocation;

        ASTExpr *FalseExpr;

        ASTTernaryOp(ASTExpr *ConditionExpr, const SourceLocation &TrueOpLocation,
                         ASTExpr *TrueExpr, const SourceLocation &FalseOpLocation, ASTExpr *FalseExpr);

    public:

        void accept(ASTVisitor& Visitor) override;

        ASTExpr *getConditionExpr() const;

        SourceLocation &getTrueOpLocation();

        ASTExpr *getTrueExpr() const;

        SourceLocation &getFalseOpLocation();

        ASTExpr *getFalseExpr() const;

        SemaTernary *getSema() const override;

    	void setSema(SemaTernary *Sema);

        std::string str() const override;
    };
}

#endif //FLY_AST_OP_H
