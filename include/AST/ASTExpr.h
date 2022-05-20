//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTExpr.h - Expression into a statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTEXPR_H
#define FLY_ASTEXPR_H

#include "Basic/SourceLocation.h"
#include <vector>
#include <utility>

namespace fly {

    enum ASTExprKind {
        EXPR_VALUE,
        EXPR_REF_VAR,
        EXPR_REF_FUNC,
        EXPR_GROUP,
        EXPR_EMPTY
    };

    enum ASTExprGroupKind {
        GROUP_UNARY,
        GROUP_BINARY,
        GROUP_TERNARY
    };

    enum UnaryOptionKind {
        UNARY_PRE,
        UNARY_POST
    };

    enum UnaryOpKind {
        ARITH_INCR,
        ARITH_DECR,
        LOGIC_NOT
    };

    enum BinaryOptionKind {
        BINARY_ARITH,
        BINARY_LOGIC,
        BINARY_COMPARISON
    };

    enum BinaryOpKind {

        // Arithmetic
        ARITH_ADD = 101,
        ARITH_SUB = 102,
        ARITH_MUL = 103,
        ARITH_DIV = 104,
        ARITH_MOD = 105,
        ARITH_AND = 106,
        ARITH_OR = 107,
        ARITH_XOR = 108,
        ARITH_SHIFT_L = 109,
        ARITH_SHIFT_R = 110,

        // Logic
        LOGIC_AND = 201,
        LOGIC_OR = 202,

        // Comparison
        COMP_EQ = 301,
        COMP_NE = 302,
        COMP_GT = 303,
        COMP_GTE = 304,
        COMP_LT = 305,
        COMP_LTE = 306
    };

    enum TernaryOpKind {
        CONDITION
    };

    class ASTType;
    class ASTValue;
    class ASTVarRef;
    class ASTFunctionCall;
    class ASTVarRefExpr;
    class ASTFuncCallExpr;
    class ASTValueExpr;
    class ASTStmt;

    /**
     * Expression Abstract Class
     */
    class ASTExpr {

        friend class SemaResolver;

        const SourceLocation &Loc;

        ASTStmt *Stmt = nullptr;

    protected:

        ASTType *Type;

    public:

        ASTExpr(const SourceLocation &Loc);

        const SourceLocation &getLocation() const;

        virtual ASTType *getType() const;

        virtual ASTExprKind getExprKind() const = 0;

        virtual std::string str() const = 0;
    };

    class ASTEmptyExpr : public ASTExpr {

        const ASTExprKind Kind = ASTExprKind::EXPR_EMPTY;

    public:

        ASTEmptyExpr(const SourceLocation &Loc);

        ASTExprKind getExprKind() const override;

        virtual std::string str() const override;
    };

    /**
     * Value Expression
     */
    class ASTValueExpr : public ASTExpr {

        const ASTExprKind Kind = ASTExprKind::EXPR_VALUE;
        ASTValue *Val;

    public:
        explicit ASTValueExpr(ASTValue *Val);

        ASTExprKind getExprKind() const override;

        ASTValue &getValue() const;

        std::string str() const override;
    };

    /**
     * Var Reference Expression
     */
    class ASTVarRefExpr : public ASTExpr {

        const ASTExprKind Kind = ASTExprKind::EXPR_REF_VAR;
        ASTVarRef *Ref;

    public:
        ASTVarRefExpr(ASTVarRef *Ref);

        ASTExprKind getExprKind() const override;

        ASTVarRef *getVarRef() const;

        ASTType *getType() const override;

        std::string str() const override;
    };

    /**
     * Function Call Expression
     */
    class ASTFuncCallExpr : public ASTExpr {

        const ASTExprKind Kind = ASTExprKind::EXPR_REF_FUNC;
        ASTFunctionCall * Call;

    public:
        ASTFuncCallExpr(ASTFunctionCall *Ref);

        ASTExprKind getExprKind() const override;

        ASTFunctionCall *getCall() const;

        ASTType *getType() const override;

        std::string str() const override;
    };

    /**
     * Group Expression
     */
    class ASTGroupExpr : public ASTExpr {

        friend class SemaResolver;

        const ASTExprKind Kind = ASTExprKind::EXPR_GROUP;

        const ASTExprGroupKind GroupKind;

    public:

        ASTGroupExpr(const SourceLocation &Loc, ASTExprGroupKind GroupKind);

        ASTExprKind getExprKind() const override;

        virtual ASTExprGroupKind getGroupKind();

        virtual ASTType *getType() const override = 0;

        virtual std::string str() const override = 0;
    };

    /**
     * Unary Group Expression
     */
    class ASTUnaryGroupExpr : public ASTGroupExpr {

        friend class SemaResolver;

        const UnaryOpKind OperatorKind;

        const UnaryOptionKind OptionKind;

        const ASTVarRefExpr *First;

    public:

        ASTUnaryGroupExpr(const SourceLocation &Loc, UnaryOpKind Operator, UnaryOptionKind Option, ASTVarRefExpr *First);

        UnaryOpKind getOperatorKind() const;

        UnaryOptionKind getOptionKind() const;

        const ASTVarRefExpr *getFirst() const;

        ASTType *getType() const override;

        std::string str() const override;
    };

    /**
     * Binary Group Expression
     */
    class ASTBinaryGroupExpr : public ASTGroupExpr {

        friend class SemaResolver;

        const BinaryOpKind OperatorKind;

        const BinaryOptionKind OptionKind;

        ASTExpr *First;

        ASTExpr *Second;

    public:

        ASTBinaryGroupExpr(const SourceLocation &Loc, BinaryOpKind Operator, ASTExpr *First, ASTExpr *Second);

        BinaryOpKind getOperatorKind() const;

        BinaryOptionKind getOptionKind() const;

        const ASTExpr *getFirst() const;

        const ASTExpr *getSecond() const;

        ASTType *getType() const override;

        std::string str() const override;
    };

    /**
     * Ternary Group Expression
     */
    class ASTTernaryGroupExpr : public ASTGroupExpr {

        friend class SemaResolver;

        // Only Ternary Condition (if ? than : else)
        const TernaryOpKind OperatorKind = CONDITION;
        ASTExpr *First;
        ASTExpr *Second;
        ASTExpr *Third;

    public:

        ASTTernaryGroupExpr(const SourceLocation &Loc, ASTExpr *First, ASTExpr *Second, ASTExpr *Third);

        TernaryOpKind getOperatorKind() const;

        ASTType *getType() const override;

        const ASTExpr *getFirst() const;

        const ASTExpr *getSecond() const;

        const ASTExpr *getThird() const;

        std::string str() const override;
    };
}

#endif //FLY_ASTEXPR_H
