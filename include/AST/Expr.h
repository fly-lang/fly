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
#include "vector"

namespace fly {

    enum ExprKind {
        EXPR_NONE = 0,
        EXPR_VALUE = 1,
        EXPR_OPERATOR = 2,
        EXPR_REF = 3,
        EXPR_GROUP = 4
    };

    class RefExpr;
    class ValueExpr;
    class GroupExpr;

    /**
     * Expression Abstract Class
     */
    class Expr {

        const SourceLocation &Loc;

    public:
        explicit Expr(const SourceLocation &Loc) : Loc(Loc) {}

        const SourceLocation &getLocation() const {
            return Loc;
        }

        virtual ExprKind getKind() { return ExprKind::EXPR_NONE; }
    };

    /**
     * Expression Reference
     */
    class RefExpr : public Expr {

        const ExprKind Kind = ExprKind::EXPR_REF;
        const Refer *Ref;

    public:
        RefExpr(const SourceLocation &Loc, const Refer *Ref) : Expr(Loc), Ref(Ref) {}

        ExprKind getKind() override {
            return Kind;
        }

        const Refer &getRef() const {
            return *Ref;
        }
    };

    /**
     * Expression Value
     */
    class ValueExpr : public Expr {

        const ExprKind Kind = ExprKind::EXPR_VALUE;
        const StringRef Str;

    public:
        ValueExpr(const SourceLocation &Loc, const StringRef Str) : Expr(Loc), Str(Str) {}

        ExprKind getKind() override {
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

        const ExprKind Kind = ExprKind::EXPR_GROUP;
        const std::vector<Expr> Group;

    public:
        GroupExpr(const SourceLocation Loc, std::vector<Expr> &Group) : Expr(Loc), Group(Group) {}

        ExprKind getKind() override {
            return Kind;
        }

        const std::vector<Expr> &getList() const {
            return Group;
        }
    };
}

#endif //FLY_EXPR_H
