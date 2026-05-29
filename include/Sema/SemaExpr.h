//===-------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaExpr.h - expression semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_RESULT_H
#define FLY_SEMA_RESULT_H

#include "CodeGen/CodeGenExpr.h"
#include "Sema/SemaNode.h"

namespace fly {

	class ASTExpr;
    class ASTNode;
    class SemaType;

    class SemaExpr : public SemaNode {

    	friend class SemaBuilder;

    	SemaExpr *Child = nullptr;

    protected:

    	SemaExpr *Parent = nullptr;

    	SemaType *Type;

    	CodeGenExpr *CodeGen = nullptr;

        explicit SemaExpr(SemaKind Kind, SemaType *Type);

    public:
        virtual ~SemaExpr();

        std::string str() const override;

    	SemaType *getType() const;

    	void setType(SemaType *Type);

    	virtual SemaExpr *getParent() const;

    	void setParent(SemaExpr &Result);

    	SemaExpr *getChild() const;

    	virtual CodeGenExpr *getCodeGen() const;

    	void setCodeGen(CodeGenExpr *CG);
    };

}

#endif //FLY_SEMA_RESULT_H
