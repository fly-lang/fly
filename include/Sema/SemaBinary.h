//===-------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaBinary.h - binary expression semantic analysis
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

		// Set by the Resolver when this is a reassignment to a heap-owned string
		// variable (`s = …`, not the initial `string s = …` declaration). Tells
		// codegen to free the destination's previous buffer AFTER storing the new
		// value (the RHS may read the same variable — `s = concat(s, …)`).
		bool FreeLHSOnAssign = false;

		CodeGenExpr *CodeGen = nullptr;

		SemaType *SelectType(ASTBinary &AST, SemaExpr *Left, SemaExpr *Right);

		explicit SemaBinary(ASTBinary &AST, SemaExpr *Left, SemaExpr *Right);

	public:

    	~SemaBinary() override = default;

    	ASTBinary &getAST() const;

		SemaExpr *getLeft() const;
		SemaExpr *getRight() const;

		bool isFreeLHSOnAssign() const;

		CodeGenExpr *getCodeGen() const;

		void setCodeGen(CodeGenExpr *CodeGen);

		std::string str() const override;

    	void accept(SemaVisitor& Visitor) override;

	};

}

#endif //FLY_SEMA_CALL_H
