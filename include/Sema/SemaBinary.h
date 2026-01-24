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

		CodeGenExpr *CodeGen = nullptr;

		SemaType *SelectType(ASTBinary &AST);

		explicit SemaBinary(ASTBinary &AST);

	public:

    	~SemaBinary() override = default;

    	ASTBinary &getAST() const;

		CodeGenExpr *getCodeGen() const;

		void setCodeGen(CodeGenExpr *CodeGen);

    	void accept(SemaVisitor& Visitor) override;

	};

}

#endif //FLY_SEMA_CALL_H
