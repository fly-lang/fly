//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTArrayAccess.h - AST array subscript expression header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_ARRAY_ACCESS_H
#define FLY_AST_ARRAY_ACCESS_H

#include "ASTExpr.h"

namespace fly {

    /**
     * Array subscript: `base[index]`.
     *
     * Both positions are expressions: the base evaluates to an array (its %array
     * fat pointer carries data + size) and the index to an integer. The same node
     * serves reads and writes — an assignment simply puts it on the left of `=`,
     * which is why it holds the two operands and nothing about direction.
     */
    class ASTArrayAccess : public ASTExpr {

        friend class ASTBuilder;

        ASTExpr *Base;

        ASTExpr *Index;

        ASTArrayAccess(const SourceLocation &Loc, ASTExpr *Base, ASTExpr *Index);

    public:

        void accept(ASTVisitor &Visitor) override;

        ASTExpr *getBase() const;

        ASTExpr *getIndex() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_ARRAY_ACCESS_H
