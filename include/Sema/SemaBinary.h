//===-------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaBinary.h - Sema Binary
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_BINARY_H
#define FLY_SEMA_BINARY_H

#include "Sema/SemaExpr.h"

namespace fly {

	class ASTBinary;

	class SemaBinary :  public SemaExpr {

		friend class SemaBuilder;
		friend class Resolver;
		friend class SemaValidator;

		ASTBinary &AST;

		SemaExpr *Left;
		SemaExpr *Right;

		CodeGenExpr *CodeGen = nullptr;

		SemaType *SelectType(ASTBinary &AST, SemaExpr *Left, SemaExpr *Right);

		explicit SemaBinary(ASTBinary &AST, SemaExpr *Left, SemaExpr *Right);

	public:

    	~SemaBinary() override = default;

    	ASTBinary &getAST() const;

		SemaExpr *getLeft() const;
		SemaExpr *getRight() const;

		CodeGenExpr *getCodeGen() const;

		void setCodeGen(CodeGenExpr *CodeGen);

    	void accept(SemaVisitor& Visitor) override;

	};

}

#endif //FLY_SEMA_CALL_H
