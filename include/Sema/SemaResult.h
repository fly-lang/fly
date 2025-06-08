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


namespace fly {

    class ASTBase;
    class SemaType;

    class SemaResult {

    	friend class SemaResolver;
    	friend class SemaResolverClass;

    	bool IsCall = false;

    	SemaResult *Parent = nullptr;

    	SemaResult *Child = nullptr;

    	SemaType *Type = nullptr;

    protected:

        explicit SemaResult(bool IsCall);

    public:
        virtual ~SemaResult() = default;
    	
    	bool isCall() const;

    	SemaResult *getParent() const;

    	void setParent(SemaResult *Result);

    	SemaResult *getChild() const;

    	SemaType *getType() const;
    };

}

#endif //FLY_SEMA_RESULT_H
