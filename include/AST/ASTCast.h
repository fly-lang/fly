//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTCallExpr.h - AST Var Expression header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_CAST_H
#define FLY_AST_CAST_H

#include "ASTExpr.h"

namespace fly {

	class ASTType;

    /**
     * Value Expression
     */
    class ASTCast : public ASTExpr {

        friend class ASTBuilder;

        ASTExpr *Expr = nullptr;

        ASTType *Type = nullptr;

        explicit ASTCast(ASTExpr *From, ASTType *Type);

    public:

    	void accept(ASTVisitor& Visitor) override;

        ASTExpr *getExpr() const;

        ASTType *getType() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_CAST_H
