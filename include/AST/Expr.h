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
    class FuncRefExpr;
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
        const llvm::StringRef Str;

    public:
        ValueExpr(const SourceLocation &Loc, const llvm::StringRef Str);

        const SourceLocation &getLocation() const;

        ExprKind getKind() const override;

        const llvm::StringRef &getString() const;
    };

    /**
     * Expression List of Expressions
     */
    class GroupExpr : public Expr {

        friend class Parser;
        friend class GlobalVarParser;

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
        VarRef * Ref;

    public:
        VarRefExpr(const SourceLocation &Loc, VarRef *Ref);

        const SourceLocation &getLocation() const;

        ExprKind getKind() const override;

        VarRef *getRef() const;
    };

    /**
     * Var Expression Reference
     */
    class FuncRefExpr : public Expr {

        const SourceLocation &Loc;
        const ExprKind Kind = ExprKind::EXPR_REF_FUNC;
        FuncCall * Ref;

    public:
        FuncRefExpr(const SourceLocation &Loc, FuncCall *Ref);

        const SourceLocation &getLocation() const;

        ExprKind getKind() const override;

        FuncCall *getRef() const;
    };
}

#endif //FLY_EXPR_H
