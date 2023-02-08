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

#include "Basic/Debuggable.h"
#include "Basic/SourceLocation.h"

#include <string>

namespace fly {

    class ASTType;
    class ASTValue;
    class ASTVarRef;
    class ASTCall;
    class ASTVarRefExpr;
    class ASTCallExpr;
    class ASTValueExpr;
    class ASTStmt;

    enum class ASTExprKind : char {
        EXPR_EMPTY,
        EXPR_VALUE,
        EXPR_VAR_REF,
        EXPR_CALL,
        EXPR_GROUP
    };

    enum class ASTExprGroupKind : char {
        GROUP_UNARY,
        GROUP_BINARY,
        GROUP_TERNARY
    };

    enum class ASTUnaryOptionKind {
        UNARY_PRE,
        UNARY_POST
    };

    enum class ASTUnaryOperatorKind {
        ARITH_INCR,
        ARITH_DECR,
        LOGIC_NOT
    };

    enum class ASTBinaryOptionKind {
        BINARY_ARITH,
        BINARY_LOGIC,
        BINARY_COMPARISON
    };

    enum class ASTBinaryOperatorKind : int {

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

    enum ASTTernaryOperatorKind {
        CONDITION
    };

    /**
     * Expression Abstract Class
     */
    class ASTExpr : public Debuggable {

        friend class SemaBuilder;
        friend class SemaResolver;

        const SourceLocation &Loc;

        ASTStmt *Stmt = nullptr;

        ASTExpr *Parent = nullptr;

    protected:

        const ASTExprKind Kind;

        ASTType *Type = nullptr;

        ASTExpr(const SourceLocation &Loc, ASTExprKind Kind);

    public:

        const SourceLocation &getLocation() const;

        virtual ASTType *getType() const;

        ASTStmt *getStmt();

        ASTExprKind getExprKind() const;

        std::string str() const;
    };

    class ASTEmptyExpr : public ASTExpr {

        friend class SemaBuilder;
        friend class SemaResolver;

    public:

        ASTEmptyExpr(const SourceLocation &Loc);

        virtual std::string str() const override;
    };

    /**
     * Value Expression
     */
    class ASTValueExpr : public ASTExpr {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTValue *Value = nullptr;

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
        friend class SemaResolver;

        ASTVarRef *VarRef = nullptr;

        ASTVarRefExpr(ASTVarRef *VarRef);

    public:

        ASTVarRef *getVarRef() const;

        ASTType *getType() const override;

        std::string str() const override;
    };

    /**
     * Function Call Expression
     */
    class ASTCallExpr : public ASTExpr {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTCall *Call = nullptr;

        ASTCallExpr(ASTCall *Call);

    public:

        ASTCall *getCall() const;

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

        std::string str() const;
    };

    /**
     * Unary Group Expression
     */
    class ASTUnaryGroupExpr : public ASTGroupExpr {

        friend class SemaBuilder;
        friend class SemaResolver;

        const ASTUnaryOperatorKind OperatorKind;

        const ASTUnaryOptionKind OptionKind;

        const ASTVarRefExpr *First = nullptr;

        ASTUnaryGroupExpr(const SourceLocation &Loc, ASTUnaryOperatorKind Operator, ASTUnaryOptionKind Option, ASTVarRefExpr *First);

    public:

        ASTUnaryOperatorKind getOperatorKind() const;

        ASTUnaryOptionKind getOptionKind() const;

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

        const SourceLocation OpLoc;

        const ASTBinaryOperatorKind OperatorKind;

        const ASTBinaryOptionKind OptionKind;

        ASTExpr *First = nullptr;

        ASTExpr *Second = nullptr;

        ASTBinaryGroupExpr(const SourceLocation &OpLoc, ASTBinaryOperatorKind Operator, ASTExpr *First, ASTExpr *Second);

    public:

        ASTBinaryOperatorKind getOperatorKind() const;

        ASTBinaryOptionKind getOptionKind() const;

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
        const ASTTernaryOperatorKind OperatorKind = CONDITION;

        const SourceLocation IfLoc;

        const SourceLocation ElseLoc;

        ASTExpr *First = nullptr;

        ASTExpr *Second = nullptr;

        ASTExpr *Third = nullptr;

        ASTTernaryGroupExpr(ASTExpr *First, const SourceLocation &IfLoc, ASTExpr *Second,
                            const SourceLocation &ElseLoc, ASTExpr *Third);

    public:

        ASTTernaryOperatorKind getOperatorKind() const;

        ASTType *getType() const override;

        const ASTExpr *getFirst() const;

        const ASTExpr *getSecond() const;

        const ASTExpr *getThird() const;

        std::string str() const override;
    };
}

#endif //FLY_ASTEXPR_H
