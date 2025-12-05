//===-------------------------------------------------------------------------------------------------------------===//
// include/Sym/SemaExpr.h - Sema Expr
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_RESULT_H
#define FLY_SEMA_RESULT_H

#include "Sema/SemaNode.h"

namespace fly {

	class ASTExpr;
    class ASTNode;
    class SemaType;

    class SemaExpr : public SemaNode {

    	friend class SemaBuilder;

    	SemaKind Kind;

    	SemaExpr *Child = nullptr;

    protected:

    	SemaExpr *Parent = nullptr;

    	SemaType *Type = nullptr;

        explicit SemaExpr(SemaKind Kind);

    public:
        virtual ~SemaExpr() = default;

    	SemaType *getType() const;

    	void setType(SemaType *Type);

    	virtual SemaExpr *getParent() const;

    	void setParent(SemaExpr &Result);

    	SemaExpr *getChild() const;
    };

}

#endif //FLY_SEMA_RESULT_H
