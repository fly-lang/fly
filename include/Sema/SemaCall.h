//===-------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaCall.h - Sybolic Table for ASTCall
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_CALL_H
#define FLY_SEMA_CALL_H

#include "Sema/SemaResult.h"
#include "AST/ASTCall.h"

namespace fly {

    class ASTCall;
    class SemaVar;
	class SemaType;
    class SemaFunctionBase;
	class SemaErrorHandler;

	enum class SemaCallKind {
		CALL_FUNCTION,
		CALL_METHOD,
	};

    class SemaCall :  public SemaResult {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

    	ASTCall *AST;

		SemaCallKind Kind;

    	SemaFunctionBase *Function = nullptr;

    	SemaErrorHandler *ErrorHandler = nullptr;

        explicit SemaCall(ASTCall *AST);

    public:

    	ASTCall *getAST() const;

    	SemaCallKind getKind() const;

    	SemaFunctionBase *getFunction() const;

    	SemaErrorHandler *getErrorHandler() const;

    	// virtual CodeGenCall *getCodeGen() const = 0;
	    //
    	// virtual void setCodeGen(CodeGenCall * CGCall) = 0;

    };

}

#endif //FLY_SEMA_CALL_H
