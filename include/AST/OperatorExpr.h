//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/OperatorExpr.h - Expression for operations
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_OPERATOREXPR_H
#define FLY_OPERATOREXPR_H

#include "Expr.h"

namespace fly {

    enum OpKind {
        OP_ARITH,
        OP_LOGIC,
        OP_BOOL,
        OP_BIT
    };

    enum ArithOpKind {
        ARITH_ADD,
        ARITH_SUB,
        ARITH_MUL,
        ARITH_DIV,
        ARITH_MOD
    };

    enum BitOpKind {
        BIT_AND,
        BIT_OR,
        BIT_NOT,
        BIT_SHIFT_L,
        BIT_SHIFT_R
    };

    enum BoolOpKind {
        BOOL_AND,
        BOOL_OR,
        BOOL_NOT
    };

    enum LogicOpKind{
        LOGIC_EQ,
        LOGIC_NE,
        LOGIC_GT,
        LOGIC_GTE,
        LOGIC_LT,
        LOGIC_LTE
    };

    class OperatorExpr : public Expr {

        const ExprKind Kind = ExprKind::EXPR_OPERATOR;

    public:
        explicit OperatorExpr(const SourceLocation &Loc) : Expr(Loc) {}

        ExprKind getKind() override {
            return Kind;
        }

        virtual OpKind getOpKind() = 0;
    };

    class ArithExpr : public OperatorExpr {

        const OpKind OperatorKind = OpKind::OP_ARITH;
        const ArithOpKind ArithKind;

    public:
        ArithExpr(const SourceLocation Loc, const ArithOpKind &AKind) : OperatorExpr(Loc), ArithKind(AKind) {}

        OpKind getOpKind() override {
            return OperatorKind;
        }

        ArithOpKind getArithKind() const {
            return ArithKind;
        }

    };

    class BitExpr : public OperatorExpr {

        const OpKind OperatorKind = OpKind::OP_BIT;
        const BitOpKind BitKind;

    public:
        BitExpr(const SourceLocation Loc, const BitOpKind &BKind) : OperatorExpr(Loc), BitKind(BKind) {}

        OpKind getOpKind() {
            return OperatorKind;
        }

        BitOpKind getBitKind() const {
            return BitKind;
        }
    };

    class BoolExpr : public OperatorExpr {

        const OpKind OperatorKind = OpKind::OP_BOOL;
        const BoolOpKind BoolKind;

    public:
        BoolExpr(const SourceLocation Loc, const BoolOpKind &BKind) : OperatorExpr(Loc), BoolKind(BKind) {}

        OpKind getOpKind() {
            return OperatorKind;
        }

        BoolOpKind getBoolKind() const {
            return BoolKind;
        }
    };

    class LogicExpr : public OperatorExpr {

        const OpKind OperatorKind = OpKind::OP_LOGIC;
        const LogicOpKind LogicKind;

    public:
        LogicExpr(const SourceLocation Loc, const LogicOpKind &LKind) : OperatorExpr(Loc), LogicKind(LKind) {}

        OpKind getOpKind() {
            return OperatorKind;
        }

        LogicOpKind getBoolKind() const {
            return LogicKind;
        }
    };
}


#endif //FLY_OPERATOREXPR_H
