//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTTernary.h - AST ternary expression header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_TERNARY_H
#define FLY_AST_TERNARY_H

#include "ASTExpr.h"

namespace fly {

    /**
     * Ternary Operator Expression
     */
    class ASTTernary : public ASTExpr {

        friend class ASTBuilder;

        ASTExpr *ConditionExpr;

        SourceLocation TrueOpLocation;

        ASTExpr *TrueExpr;

        SourceLocation FalseOpLocation;

        ASTExpr *FalseExpr;

        ASTTernary(ASTExpr *ConditionExpr, const SourceLocation &TrueOpLocation,
                         ASTExpr *TrueExpr, const SourceLocation &FalseOpLocation, ASTExpr *FalseExpr);

    public:

        void accept(ASTVisitor& Visitor) override;

        ASTExpr *getConditionExpr() const;

        SourceLocation &getTrueOpLocation();

        ASTExpr *getTrueExpr() const;

        SourceLocation &getFalseOpLocation();

        ASTExpr *getFalseExpr() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_TERNARY_H

