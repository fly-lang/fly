//===-------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaUnary.h - unary expression semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_UNARY_H
#define FLY_SEMA_UNARY_H

#include "Sema/SemaExpr.h"

namespace fly {

	class ASTUnary;

	class SemaUnary :  public SemaExpr {

		friend class SemaBuilder;
		friend class Resolver;
		friend class SemaValidator;

		ASTUnary &AST;

		SemaExpr *Expr;

		CodeGenExpr *CodeGen = nullptr;

		explicit SemaUnary(ASTUnary &AST, SemaExpr *Expr);

	public:

    	~SemaUnary() override = default;

    	ASTUnary &getAST() const;

		SemaExpr *getExpr() const;

		CodeGenExpr *getCodeGen() const;

		void setCodeGen(CodeGenExpr *CodeGen);

    	void accept(SemaVisitor& Visitor) override;

	};

}

#endif //FLY_SEMA_CALL_H
