//===-------------------------------------------------------------------------------------------------------------===//
// include/Sym/SymCall.h - Sybolic Table for ASTCall
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYM_CALL_H
#define FLY_SYM_CALL_H


namespace fly {

    class ASTCall;

	enum class SymCallKind {
		CALL_FUNCTION,
		CALL_METHOD,
	};

    class SymCall {

        friend class SymBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

    	SymCall *AST;

		SymCallKind Kind;

		Sy

        explicit SymCall(ASTCall *AST);

    public:

    	ASTCall *getAST() const;

    };

}

#endif //FLY_SYM_VAR_H
