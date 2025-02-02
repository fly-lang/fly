//===-------------------------------------------------------------------------------------------------------------===//
// include/Sym/SymRef.h - Sybolic Table for ASTRef
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYM_REF_H
#define FLY_SYM_REF_H


namespace fly {

    class ASTIdentifier;

	enum class SymRefKind {
		REF_NAMESPACE,
		REF_TYPE,
		REF_CALL,
		REF_VAR
	};

    class SymRef {

    	friend class SymBuilder;
    	friend class SemaResolver;
    	friend class SemaValidator;

    	ASTIdentifier *AST;

		SymRefKind Kind;

        explicit SymRef(ASTIdentifier *AST);

    public:

    	ASTIdentifier *getAST() const;

		SymRefKind getKind() const;

    };

}

#endif //FLY_SYM_VAR_H
