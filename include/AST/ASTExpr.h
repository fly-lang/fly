//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTExpr.h - AST Expression header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_EXPR_H
#define FLY_AST_EXPR_H

#include "ASTBase.h"

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
        EXPR_VALUE,
        EXPR_VAR_REF,
        EXPR_CALL,
        EXPR_NEW,
        EXPR_OP
    };

    /**
     * Expression Abstract Class
     */
    class ASTExpr : public ASTBase {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

    protected:

        const ASTExprKind Kind;

        ASTType *Type = nullptr;

        ASTExpr(const SourceLocation &Loc, ASTExprKind Kind);

    public:

        ASTType *getType() const;

        ASTExprKind getExprKind() const;

        std::string str() const override;
    };

    /**
     * Value Expression
     */
    class ASTValueExpr : public ASTExpr {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTValue *Value = nullptr;

        explicit ASTValueExpr(ASTValue *Val);

    public:

        ASTValue *getValue() const;

        std::string str() const override;
    };

    /**
     * Var Reference Expression
     */
    class ASTVarRefExpr : public ASTExpr {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTVarRef *VarRef = nullptr;

        explicit ASTVarRefExpr(ASTVarRef *VarRef);

    public:

        ASTVarRef *getVarRef() const;

        std::string str() const override;
    };

    /**
     * Function Call Expression
     */
    class ASTCallExpr : public ASTExpr {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTCall *Call = nullptr;

        explicit ASTCallExpr(ASTCall *Call);

    public:

        ASTCall *getCall() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_EXPR_H
