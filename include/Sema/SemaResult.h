//===-------------------------------------------------------------------------------------------------------------===//
// include/Sym/SemaResult.h - Sybolic Table for ASTRef
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

    class ASTNode;
    class SemaType;

    class SemaResult : public SemaNode {

    	friend class SemaBuilder;

    	SemaKind Kind;

    	SemaResult *Child = nullptr;

    protected:

    	SemaResult *Parent = nullptr;

    	SemaType *Type = nullptr;

        explicit SemaResult(SemaKind Kind);

    public:
        virtual ~SemaResult() = default;

    	void setType(SemaType *Type);
    	
    	bool isCall() const;

    	virtual SemaResult *getParent() const;

    	void setParent(SemaResult &Result);

    	SemaResult *getChild() const;

    	SemaType *getType() const;
    };

}

#endif //FLY_SEMA_RESULT_H
