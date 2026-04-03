//===--------------------------------------------------------------------------------------------------------------===//
//
// Thank you to LLVM Project https://llvm.org/
// Under the Apache License v2.0 see LICENSE for details.
// Part of the Fly Project https://flylang.org
//
// include/Sema/SemaTernary.h - Sema Ternary
//===-------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_TERNARY_H
#define FLY_SEMA_TERNARY_H

#include "SemaExpr.h"

namespace fly {

	class ASTTernary;

	class SemaTernary : public SemaExpr {

		friend class SemaBuilder;

		ASTTernary &AST;

		SemaExpr *Cond;
		SemaExpr *TrueExpr;
		SemaExpr *FalseExpr;

		CodeGenExpr *CodeGen = nullptr;

		SemaType *SelectType(SemaExpr * LeftExpr, SemaExpr * RightExpr);

		explicit SemaTernary(ASTTernary &AST, SemaExpr *Cond, SemaExpr *TrueExpr, SemaExpr *FalseExpr);

	public:

		~SemaTernary() override = default;

		ASTTernary &getAST() const;

		SemaExpr *getCond() const;
		SemaExpr *getTrueExpr() const;
		SemaExpr *getFalseExpr() const;

		CodeGenExpr *getCodeGen() const;

		void setCodeGen(CodeGenExpr *CodeGen);

		void accept(SemaVisitor& Visitor) override;

	};
}
#endif //FLY_SEMA_TERNARY_H

