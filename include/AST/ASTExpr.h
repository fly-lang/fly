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
        EXPR_EMPTY,
        EXPR_VALUE,
        EXPR_REF_VAR,
        EXPR_REF_FUNC,
        EXPR_GROUP
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
    class ASTFunctionCallExpr;
    class ASTValueExpr;
    class ASTStmt;

    /**
     * Expression Abstract Class
     */
    class ASTExpr {

        friend class SemaBuilder;
        friend class SemaResolver;

        const SourceLocation &Loc;

        const ASTExprKind Kind;

        ASTStmt *Stmt = nullptr;

    protected:

        ASTType *Type = nullptr;

        ASTExpr(const SourceLocation &Loc, ASTExprKind Kind);

    public:

        const SourceLocation &getLocation() const;

        virtual ASTType *getType() const;

        ASTStmt *getStmt();

        ASTExprKind getExprKind() const;

        virtual std::string str() const = 0;
    };

    class ASTEmptyExpr : public ASTExpr {

        friend class SemaBuilder;

    public:

        ASTEmptyExpr(const SourceLocation &Loc);

        virtual std::string str() const override;
    };

    /**
     * Value Expression
     */
    class ASTValueExpr : public ASTExpr {

        friend class SemaBuilder;

        ASTValue *Val = nullptr;

        explicit ASTValueExpr(ASTValue *Val);

    public:

        ASTValue &getValue() const;

        std::string str() const override;
    };

    /**
     * Var Reference Expression
     */
    class ASTVarRefExpr : public ASTExpr {

        friend class SemaBuilder;

        ASTVarRef *Ref = nullptr;

        ASTVarRefExpr(ASTVarRef *Ref);

    public:

        ASTVarRef *getVarRef() const;

        ASTType *getType() const override;

        std::string str() const override;
    };

    /**
     * Function Call Expression
     */
    class ASTFunctionCallExpr : public ASTExpr {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTFunctionCall *Call = nullptr;

        ASTFunctionCallExpr(ASTFunctionCall *Call);

    public:

        ASTFunctionCall *getCall() const;

        ASTType *getType() const override;

        std::string str() const override;
    };

    /**
     * Group Expression
     */
    class ASTGroupExpr : public ASTExpr {

        friend class SemaBuilder;
        friend class SemaResolver;

        const ASTExprGroupKind GroupKind;

    protected:

        ASTGroupExpr(const SourceLocation &Loc, ASTExprGroupKind GroupKind);

    public:

        virtual ASTExprGroupKind getGroupKind();

        virtual ASTType *getType() const override = 0;

        virtual std::string str() const override = 0;
    };

    /**
     * Unary Group Expression
     */
    class ASTUnaryGroupExpr : public ASTGroupExpr {

        friend class SemaBuilder;
        friend class SemaResolver;

        const UnaryOpKind OperatorKind;

        const UnaryOptionKind OptionKind;

        const ASTVarRefExpr *First = nullptr;

        ASTUnaryGroupExpr(const SourceLocation &Loc, UnaryOpKind Operator, UnaryOptionKind Option, ASTVarRefExpr *First);

    public:

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

        friend class SemaBuilder;
        friend class SemaResolver;

        const BinaryOpKind OperatorKind;

        const BinaryOptionKind OptionKind;

        ASTExpr *First = nullptr;

        ASTExpr *Second = nullptr;

        ASTBinaryGroupExpr(const SourceLocation &Loc, BinaryOpKind Operator, ASTExpr *First, ASTExpr *Second);

    public:

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

        friend class SemaBuilder;
        friend class SemaResolver;

        // Only Ternary Condition (if ? than : else)
        const TernaryOpKind OperatorKind = CONDITION;

        ASTExpr *First = nullptr;

        ASTExpr *Second = nullptr;

        ASTExpr *Third = nullptr;

        ASTTernaryGroupExpr(const SourceLocation &Loc, ASTExpr *First, ASTExpr *Second, ASTExpr *Third);

    public:

        TernaryOpKind getOperatorKind() const;

        ASTType *getType() const override;

        const ASTExpr *getFirst() const;

        const ASTExpr *getSecond() const;

        const ASTExpr *getThird() const;

        std::string str() const override;
    };
}

#endif //FLY_ASTEXPR_H
