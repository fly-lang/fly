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

namespace fly {

    class ASTCall;
    class SemaVar;
	class SemaType;
    class SemaFunctionBase;
	class SemaErrorHandler;

    class SemaCall :  public SemaResult {

        friend class SemaBuilder;
        friend class Resolver;
        friend class SemaValidator;

    	ASTCall &AST;

    	SemaFunctionBase *Function = nullptr;

    	SemaErrorHandler *ErrorHandler = nullptr;

        explicit SemaCall(ASTCall &AST);

    public:

    	ASTCall &getAST() const;

    	SemaFunctionBase *getFunction() const;

    	void setFunction(SemaFunctionBase *Function);

    	SemaErrorHandler *getErrorHandler() const;

    	void setErrorHandler(SemaErrorHandler *ErrorHandler);

    	bool isNew() const;

    };

}

#endif //FLY_SEMA_CALL_H
