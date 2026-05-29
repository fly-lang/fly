//===-------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaCall.h - function call semantic analysis
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
	class SemaLocalVar;

    class SemaCall :  public SemaExpr {

        friend class SemaBuilder;
        friend class Resolver;
        friend class SemaValidator;

    	ASTCall &AST;

    	SemaFunctionBase *Function = nullptr;

    	SemaError *ErrorHandler = nullptr;

    	llvm::SmallVector<SemaExpr *, 8> Args;

    	// Synthetic local variable that receives the return value (non-null when the
    	// resolved function has a declared return type).  The callee writes to it via
    	// the hidden out parameter; the CodeGen loads it after the call to produce
    	// the expression value for assignments and chaining.
    	SemaLocalVar *OutVar = nullptr;

    	CodeGenExpr *CodeGen = nullptr;

        explicit SemaCall(ASTCall &AST, SemaType *Type);

    public:

    	~SemaCall() override = default;

    	ASTCall &getAST() const;

    	SemaFunctionBase *getFunction() const;

    	SemaError *getErrorHandler() const;

    	void setErrorHandler(SemaError *ErrorHandler);

    	bool isNew() const;

    	SemaLocalVar *getOutVar() const;

    	llvm::SmallVector<SemaExpr *, 8> &getArgs();

    	void addArg(SemaExpr *Arg);

    	CodeGenExpr *getCodeGen() const;

    	void setCodeGen(CodeGenExpr *CodeGen);

    	std::string str() const override;

    	void accept(SemaVisitor& Visitor) override;

    };

}

#endif //FLY_SEMA_CALL_H
