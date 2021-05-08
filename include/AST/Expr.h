//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/Expr.h - Expression for assignment
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_EXPR_H
#define FLY_EXPR_H

#include "Refer.h"
#include "VarDecl.h"
#include "vector"

namespace fly {

    enum ExprKind {
        EXPR_VALUE = 1,
        EXPR_OPERATOR = 2,
        EXPR_REF_VAR = 3,
        EXPR_REF_FUNC = 4,
        EXPR_GROUP = 5
    };

    class VarRef;
    class FuncRef;
    class RefExpr;
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
        const StringRef Str;

    public:
        ValueExpr(const SourceLocation &Loc, const StringRef Str) : Loc(Loc), Str(Str) {}

        const SourceLocation &getLocation() const {
            return Loc;
        }

        ExprKind getKind() const override {
            return Kind;
        }

        const StringRef &getString() const {
            return Str;
        }
    };

    /**
     * Expression List of Expressions
     */
    class GroupExpr : public Expr {

        friend class Parser;

        const ExprKind Kind = ExprKind::EXPR_GROUP;
        std::vector<Expr *> Group;

    public:

        ExprKind getKind() const override {
            return Kind;
        }

        const std::vector<Expr *> &getGroup() const {
            return Group;
        }
    };

    /**
     * Var Expression Reference
     */
    class VarRefExpr : public Expr {

        const SourceLocation Loc;
        const ExprKind Kind = ExprKind::EXPR_REF_VAR;
        VarRef * Ref;

    public:
        VarRefExpr(const SourceLocation &Loc, VarRef *Ref) : Loc(Loc), Ref(Ref) {}

        const SourceLocation &getLocation() const {
            return Loc;
        }

        ExprKind getKind() const override {
            return Kind;
        }

        VarRef *getRef() const {
            return Ref;
        }
    };

    /**
     * Var Expression Reference
     */
    class FuncRefExpr : public Expr {

        const SourceLocation &Loc;
        const ExprKind Kind = ExprKind::EXPR_REF_FUNC;
        FuncRef * Ref;

    public:
        FuncRefExpr(const SourceLocation &Loc, FuncRef *Ref) : Loc(Loc), Ref(Ref) {}

        const SourceLocation &getLocation() const {
            return Loc;
        }

        ExprKind getKind() const override {
            return Kind;
        }

        FuncRef *getRef() const {
            return Ref;
        }
    };
}

#endif //FLY_EXPR_H
