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

    class ASTExprStmt;

    enum OpKind {
        OP_ARITH,
        OP_COMPARISON,
        OP_LOGIC,
        OP_CONDITION
    };

    enum OpTypeKind {
        TY_UNARY,
        TY_BINARY,
        TY_TERNARY
    };

    enum UnaryOpKind {
        UNARY_PRE,
        UNARY_POST
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
        ARITH_SHIFT_R,
        ARITH_INCR,
        ARITH_DECR
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

    class ASTOperatorExpr : public ASTExpr {

        const ASTExprKind Kind = ASTExprKind::EXPR_OPERATOR;
        const OpTypeKind TypeKind;

    public:
        explicit ASTOperatorExpr(const SourceLocation &Loc, OpTypeKind TypeKind);

        ASTExprKind getKind() const override;

        OpTypeKind getTypeKind() const;

        ASTType *getType() const override;

        virtual OpKind getOpKind() = 0;

        virtual bool isUnary() = 0;

        virtual bool isBinary() = 0;

        virtual bool isTernary() = 0;

        virtual std::string str() const = 0;
    };

    class ASTUnaryExpr : public ASTOperatorExpr {

        ASTOperatorExpr *OperatorExpr;
        ASTVarRef *VarRef;
        UnaryOpKind UnaryKind;

    public:
        ASTUnaryExpr(const SourceLocation &Loc, ASTOperatorExpr *OperatorExpr, ASTVarRef *VarRef, UnaryOpKind UKind);

        OpKind getOpKind() override;

        UnaryOpKind getUnaryKind() const;

        ASTOperatorExpr *getOperatorExpr() const;

        ASTVarRef *getVarRef() const;

        bool isUnary() override{
            return true;
        }

        bool isBinary() override{
            return false;
        }

        bool isTernary() override{
            return false;
        }

        std::string str() const override;
    };

    class ASTBinaryExpr : public ASTOperatorExpr {

        const OpTypeKind TypeKind = TY_BINARY;

    public:
        explicit ASTBinaryExpr(const SourceLocation &Loc);

        bool isUnary() override {
            return false;
        }

        bool isBinary() override {
            return true;
        }

        bool isTernary() override {
            return false;
        }

    };

    class ASTArithExpr : public ASTBinaryExpr {

        const OpKind OperatorKind = OpKind::OP_ARITH;
        const ArithOpKind ArithKind;

    public:
        ASTArithExpr(const SourceLocation &Loc, const ArithOpKind &AKind);

        OpKind getOpKind() override;

        ArithOpKind getArithKind() const;

        std::string str() const override;
    };

    class ASTLogicExpr : public ASTBinaryExpr {

        const OpKind OperatorKind = OpKind::OP_LOGIC;
        const LogicOpKind LogicKind;

    public:
        ASTLogicExpr(const SourceLocation &Loc, const LogicOpKind &LKind);

        OpKind getOpKind() override;

        LogicOpKind getLogicKind() const;

        std::string str() const override;
    };

    class ASTComparisonExpr : public ASTBinaryExpr {

        const OpKind OperatorKind = OpKind::OP_COMPARISON;
        const ComparisonOpKind ComparisonKind;

    public:
        ASTComparisonExpr(const SourceLocation &Loc, const ComparisonOpKind &CKind);

        OpKind getOpKind() override;

        ComparisonOpKind getComparisonKind() const;

        virtual std::string str() const override;
    };

    class ASTTernaryExpr : public ASTOperatorExpr {

        const OpKind OperatorKind = OpKind::OP_CONDITION;
        ASTExpr *Condition;
        ASTExprStmt *Stmt1;
        ASTExprStmt *Stmt2;

    public:
        ASTTernaryExpr(const SourceLocation &Loc, ASTExpr *Condition, ASTExprStmt *Stmt1, ASTExprStmt *Stmt2);

        OpKind getOpKind() override;

        bool isUnary() override{
            return false;
        }

        bool isBinary() override{
            return false;
        }

        bool isTernary() override{
            return true;
        }

        virtual std::string str() const override;
    };
}


#endif //FLY_ASTOPERATOREXPR_H
