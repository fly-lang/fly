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
        OP_BIT,
        OP_INCDEC,
        OP_COND
    };

    enum ArithOpKind {
        ARITH_ADD,
        ARITH_SUB,
        ARITH_MUL,
        ARITH_DIV,
        ARITH_MOD,
        ARITH_INCR,
        ARITH_DECR
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

    enum IncDecOpKind {
        PRE_INCREMENT,
        POST_INCREMENT,
        PRE_DECREMENT,
        POST_DECREMENT
    };

    enum CondOpKind {
        COND_IF,
        COND_THAN,
        COND_ELSE
    };

    class ASTOperatorExpr : public ASTExpr {

        const SourceLocation &Loc;
        const ExprKind Kind = ExprKind::EXPR_OPERATOR;

    public:
        explicit ASTOperatorExpr(const SourceLocation &Loc) : Loc(Loc) {}

        const SourceLocation &getLocation() const {
            return Loc;
        }

        ExprKind getKind() const override {
            return Kind;
        }

        virtual OpKind getOpKind() = 0;
    };

    class ASTArithExpr : public ASTOperatorExpr {

        const OpKind OperatorKind = OpKind::OP_ARITH;
        const ArithOpKind ArithKind;

    public:
        ASTArithExpr(const SourceLocation Loc, const ArithOpKind &AKind) : ASTOperatorExpr(Loc), ArithKind(AKind) {}

        OpKind getOpKind() override {
            return OperatorKind;
        }

        ArithOpKind getArithKind() const {
            return ArithKind;
        }

    };

    class ASTBitExpr : public ASTOperatorExpr {

        const OpKind OperatorKind = OpKind::OP_BIT;
        const BitOpKind BitKind;

    public:
        ASTBitExpr(const SourceLocation Loc, const BitOpKind &BKind) : ASTOperatorExpr(Loc), BitKind(BKind) {}

        OpKind getOpKind() {
            return OperatorKind;
        }

        BitOpKind getBitKind() const {
            return BitKind;
        }
    };

    class ASTBoolExpr : public ASTOperatorExpr {

        const OpKind OperatorKind = OpKind::OP_BOOL;
        const BoolOpKind BoolKind;

    public:
        ASTBoolExpr(const SourceLocation Loc, const BoolOpKind &BKind) : ASTOperatorExpr(Loc), BoolKind(BKind) {}

        OpKind getOpKind() {
            return OperatorKind;
        }

        BoolOpKind getBoolKind() const {
            return BoolKind;
        }
    };

    class ASTLogicExpr : public ASTOperatorExpr {

        const OpKind OperatorKind = OpKind::OP_LOGIC;
        const LogicOpKind LogicKind;

    public:
        ASTLogicExpr(const SourceLocation Loc, const LogicOpKind &LKind) : ASTOperatorExpr(Loc), LogicKind(LKind) {}

        OpKind getOpKind() {
            return OperatorKind;
        }

        LogicOpKind getLogicKind() const {
            return LogicKind;
        }
    };

    class ASTIncDecExpr : public ASTOperatorExpr {
        const OpKind OperatorKind = OpKind::OP_INCDEC;
        const IncDecOpKind IncDecKind;

    public:
        ASTIncDecExpr(const SourceLocation Loc, const IncDecOpKind &Kind) : ASTOperatorExpr(Loc), IncDecKind(Kind) {}

        OpKind getOpKind() {
            return OperatorKind;
        }

        IncDecOpKind getIncDecKind() const {
            return IncDecKind;
        }
    };

    class ASTCondExpr : public ASTOperatorExpr {

        const OpKind OperatorKind = OpKind::OP_COND;
        const CondOpKind CondKind;

    public:
        ASTCondExpr(const SourceLocation Loc, const CondOpKind &CondKind) : ASTOperatorExpr(Loc), CondKind(CondKind) {}

        OpKind getOpKind() {
            return OperatorKind;
        }

        CondOpKind getCustKind() const {
            return CondKind;
        }
    };
}


#endif //FLY_ASTOPERATOREXPR_H
