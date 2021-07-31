//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTOperatorExpr.h - Expression for operations
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_ASTOPERATOREXPR_H
#define FLY_ASTOPERATOREXPR_H

#include "ASTExpr.h"

namespace fly {

    enum OpKind {
        OP_ARITH,
        OP_LOGIC,
        OP_BOOL,
        OP_INCDEC,
        OP_COND
    };

    enum ArithOpKind {
        ARITH_ADD,
        ARITH_SUB,
        ARITH_MUL,
        ARITH_DIV,
        ARITH_MOD,
        ARITH_AND,
        ARITH_OR,
        ARITH_XOR,
        ARITH_SHIFT_L,
        ARITH_SHIFT_R
    };

    enum LogicOpKind {
        LOGIC_AND,
        LOGIC_OR,
        LOGIC_NOT
    };

    enum ComparisonOpKind {
        COMP_EQ,
        COMP_NE,
        COMP_GT,
        COMP_GTE,
        COMP_LT,
        COMP_LTE
    };

    enum IncDecOpKind {
        PRE_INCR,
        POST_INCR,
        PRE_DECR,
        POST_DECR
    };

    enum CondOpKind {
        COND_IF,
        COND_THAN,
        COND_ELSE
    };

    class ASTOperatorExpr : public ASTExpr {

        const ExprKind Kind = ExprKind::EXPR_OPERATOR;

    public:
        explicit ASTOperatorExpr(const SourceLocation &Loc);

        ExprKind getKind() const override;

        ASTType *getType() const override;

        virtual OpKind getOpKind() = 0;
    };

    class ASTArithExpr : public ASTOperatorExpr {

        const OpKind OperatorKind = OpKind::OP_ARITH;
        const ArithOpKind ArithKind;

    public:
        ASTArithExpr(const SourceLocation &Loc, const ArithOpKind &AKind);

        OpKind getOpKind() override;

        ArithOpKind getArithKind() const;
    };

    class ASTLogicExpr : public ASTOperatorExpr {

        const OpKind OperatorKind = OpKind::OP_BOOL;
        const LogicOpKind BoolKind;

    public:
        ASTLogicExpr(const SourceLocation &Loc, const LogicOpKind &BKind);

        OpKind getOpKind() override;

        LogicOpKind getBoolKind() const;
    };

    class ASTComparisonExpr : public ASTOperatorExpr {

        const OpKind OperatorKind = OpKind::OP_LOGIC;
        const ComparisonOpKind ComparisonKind;

    public:
        ASTComparisonExpr(const SourceLocation &Loc, const ComparisonOpKind &LKind);

        OpKind getOpKind() override;

        ComparisonOpKind getComparisonKind() const;
    };

    class ASTIncDecExpr : public ASTOperatorExpr {
        const OpKind OperatorKind = OpKind::OP_INCDEC;
        const IncDecOpKind IncDecKind;

    public:
        ASTIncDecExpr(const SourceLocation &Loc, const IncDecOpKind &Kind);

        OpKind getOpKind() override;

        IncDecOpKind getIncDecKind() const;
    };

    class ASTCondExpr : public ASTOperatorExpr {

        const OpKind OperatorKind = OpKind::OP_COND;
        const CondOpKind CondKind;

    public:
        ASTCondExpr(const SourceLocation &Loc, const CondOpKind &CondKind);

        OpKind getOpKind() override;

        CondOpKind getCondKind() const;
    };
}


#endif //FLY_ASTOPERATOREXPR_H
