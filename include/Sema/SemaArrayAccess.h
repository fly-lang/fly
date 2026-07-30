//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaArrayAccess.h - resolved array subscript
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_ARRAY_ACCESS_H
#define FLY_SEMA_ARRAY_ACCESS_H

#include "Sema/SemaExpr.h"

namespace fly {

	class ASTArrayAccess;

	/**
	 * Resolved `base[index]`. Its type is the ARRAY'S ELEMENT type, which is what
	 * makes `int x = k[1]` type-check like any other int expression. The node is
	 * direction-agnostic: CodeGen reads it or, when it is the target of an
	 * assignment, stores through it.
	 */
	class SemaArrayAccess : public SemaExpr {

		friend class SemaBuilder;
		friend class Resolver;
		friend class SemaValidator;

		ASTArrayAccess &AST;

		SemaExpr *Base;

		SemaExpr *Index;

		CodeGenExpr *CodeGen = nullptr;

		explicit SemaArrayAccess(ASTArrayAccess &AST, SemaExpr *Base, SemaExpr *Index, SemaType *Type);

	public:

		~SemaArrayAccess() override = default;

		ASTArrayAccess &getAST() const;

		SemaExpr *getBase() const;

		SemaExpr *getIndex() const;

		CodeGenExpr *getCodeGen() const;

		void setCodeGen(CodeGenExpr *CodeGen);

		std::string str() const override;

		void accept(SemaVisitor &Visitor) override;
	};

}

#endif //FLY_SEMA_ARRAY_ACCESS_H
