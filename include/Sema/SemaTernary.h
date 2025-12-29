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

	class ASTTernaryOp;

	class SemaTernary : public SemaExpr {

		friend class SemaBuilder;

		ASTTernaryOp &AST;

		SemaType *SelectType(SemaExpr * LeftExpr, SemaExpr * RightExpr);

		explicit SemaTernary(ASTTernaryOp &AST);

	public:

		~SemaTernary() override = default;

		ASTTernaryOp &getAST() const;

	};
}
#endif //FLY_SEMA_TERNARY_H