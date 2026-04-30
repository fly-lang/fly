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

#include "Sema/SemaExpr.h"

#include <llvm/ADT/SmallVector.h>

namespace fly {

    class ASTCall;
    class SemaVar;
	class SemaType;
    class SemaFunctionBase;
	class SemaError;

    class SemaCall :  public SemaExpr {

        friend class SemaBuilder;
        friend class Resolver;
        friend class SemaValidator;

    	ASTCall &AST;

    	SemaFunctionBase *Function = nullptr;

    	SemaError *ErrorHandler = nullptr;

    	llvm::SmallVector<SemaExpr *, 8> Args;

    	CodeGenExpr *CodeGen = nullptr;

        explicit SemaCall(ASTCall &AST, SemaType *Type);

    public:

    	~SemaCall() override = default;

    	ASTCall &getAST() const;

    	SemaFunctionBase *getFunction() const;

    	SemaError *getErrorHandler() const;

    	void setErrorHandler(SemaError *ErrorHandler);

    	bool isNew() const;

    	llvm::SmallVector<SemaExpr *, 8> &getArgs();

    	void addArg(SemaExpr *Arg);

    	CodeGenExpr *getCodeGen() const;

    	void setCodeGen(CodeGenExpr *CodeGen);

    	void accept(SemaVisitor& Visitor) override;

    };

}

#endif //FLY_SEMA_CALL_H
