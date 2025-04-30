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
    class SymVar;
	class SymType;
    class SymFunctionBase;
	class SymErrorHandler;

	enum class SymCallKind {
		CALL_FUNCTION,
		CALL_METHOD,
	};

    class SymCall {

        friend class SymBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

    	ASTCall *AST;

		SymCallKind Kind;

    	SymVar *Parent = nullptr;

    	SymFunctionBase *Function = nullptr;

    	SymErrorHandler *ErrorHandler = nullptr;

        explicit SymCall(ASTCall *AST);

    public:

    	ASTCall *getAST() const;

    	SymCallKind getKind() const;

    	SymVar *getParent() const;

    	SymFunctionBase *getFunction() const;

    	SymErrorHandler *getErrorHandler() const;

    	// virtual CodeGenCall *getCodeGen() const = 0;
	    //
    	// virtual void setCodeGen(CodeGenCall * CGCall) = 0;

    };

}

#endif //FLY_SYM_VAR_H
