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

	enum class SemaResultKind {
		VAR,
		CALL
	};

    class SemaResult {

    	friend class SemaResolver;

    	SemaResultKind Kind;

    	SemaResult *Parent = nullptr;

    	SemaResult *Child = nullptr;

    protected:

        explicit SemaResult(SemaResultKind Kind);

    public:
        virtual ~SemaResult() = default;
    	
    	SemaResultKind getKind() const;

    	SemaResult *getParent() const;

    	void setParent(SemaResult *Result);

    	SemaResult *getChild() const;
    };

}

#endif //FLY_SEMA_RESULT_H
