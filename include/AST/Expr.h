//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/Expr.h - Expression into a statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_EXPR_H
#define FLY_EXPR_H

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
        EXPR_GROUP = 5
    };

    class VarRef;
    class FuncCall;
    class VarRefExpr;
    class FuncCallExpr;
    class ValueExpr;
    class GroupExpr;

    /**
     * Expression Abstract Class
     */
    class Expr {

    public:

        virtual ExprKind getKind() const = 0;
    };

    /**
     * Expression Value
     */
    class ValueExpr : public Expr {

        const SourceLocation &Loc;
        const ExprKind Kind = ExprKind::EXPR_VALUE;
        const ASTValue *Val;

    public:
        ValueExpr(const SourceLocation &Loc, const ASTValue *Val);

        const SourceLocation &getLocation() const;

        ExprKind getKind() const override;

        const ASTValue &getValue() const;
    };

    /**
     * Expression List of Expressions
     */
    class GroupExpr : public Expr {

        const ExprKind Kind = ExprKind::EXPR_GROUP;
        std::vector<Expr *> Group;

    public:

        ExprKind getKind() const override;

        const std::vector<Expr *> &getGroup() const;

        bool isEmpty() const;

        void Add(Expr * Exp);

    };

    /**
     * Var Expression Reference
     */
    class VarRefExpr : public Expr {

        const SourceLocation Loc;
        const ExprKind Kind = ExprKind::EXPR_REF_VAR;
        VarRef *Ref;

    public:
        VarRefExpr(const SourceLocation &Loc, VarRef *Ref);

        const SourceLocation &getLocation() const;

        ExprKind getKind() const override;

        VarRef *getVarRef() const;
    };

    /**
     * Var Expression Reference
     */
    class FuncCallExpr : public Expr {

        const SourceLocation &Loc;
        const ExprKind Kind = ExprKind::EXPR_REF_FUNC;
        FuncCall * Call;

    public:
        FuncCallExpr(const SourceLocation &Loc, FuncCall *Ref);

        const SourceLocation &getLocation() const;

        ExprKind getKind() const override;

        FuncCall *getCall() const;
    };
}

#endif //FLY_EXPR_H
