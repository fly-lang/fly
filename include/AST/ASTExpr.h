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
#include "llvm/ADT/StringRef.h"
#include "ASTValue.h"
#include <vector>
#include <utility>

namespace fly {

    enum ExprKind {
        EXPR_VALUE = 1,
        EXPR_OPERATOR = 2,
        EXPR_REF_VAR = 3,
        EXPR_REF_FUNC = 4,
        EXPR_GROUP = 5,
        EXPR_VIRTUAL = 10
    };

    class ASTVarRef;
    class ASTFuncCall;
    class ASTVarRefExpr;
    class ASTFuncCallExpr;
    class ASTValueExpr;
    class ASTGroupExpr;

    /**
     * Expression Abstract Class
     */
    class ASTExpr {

        const SourceLocation &Loc;

    public:

        ASTExpr(const SourceLocation &Loc);

        const SourceLocation &getLocation() const;

        virtual ExprKind getKind() const = 0;

        virtual ASTType *getType() const = 0;
    };

    /**
     * Expression Value
     */
    class ASTValueExpr : public ASTExpr {

        const ExprKind Kind = ExprKind::EXPR_VALUE;
        const ASTValue *Val;

    public:
        ASTValueExpr(const SourceLocation &Loc, const ASTValue *Val);

        ExprKind getKind() const override;

        const ASTValue &getValue() const;

        ASTType *getType() const;
    };

    /**
     * Var Expression Reference
     */
    class ASTVarRefExpr : public ASTExpr {

        const ExprKind Kind = ExprKind::EXPR_REF_VAR;
        ASTVarRef *Ref;

    public:
        ASTVarRefExpr(const SourceLocation &Loc, ASTVarRef *Ref);

        ExprKind getKind() const override;

        ASTVarRef *getVarRef() const;

        ASTType *getType() const;
    };

    /**
     * Var Expression Reference
     */
    class ASTFuncCallExpr : public ASTExpr {

        const ExprKind Kind = ExprKind::EXPR_REF_FUNC;
        ASTFuncCall * Call;

    public:
        ASTFuncCallExpr(const SourceLocation &Loc, ASTFuncCall *Ref);

        ExprKind getKind() const override;

        ASTFuncCall *getCall() const;

        ASTType *getType() const;
    };

    /**
     * Expression List of Expressions
     */
    class ASTGroupExpr : public ASTExpr {

        const ExprKind Kind = ExprKind::EXPR_GROUP;
        std::vector<ASTExpr *> Group;

    public:

        ASTGroupExpr(const SourceLocation &Loc);

        ExprKind getKind() const override;

        const std::vector<ASTExpr *> &getGroup() const;

        bool isEmpty() const;

        void Add(ASTExpr * Exp);

        ASTType *getType() const;

    };
}

#endif //FLY_ASTEXPR_H
