//===-------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaCast.h - Sema Cast
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_CAST_H
#define FLY_SEMA_CAST_H

#include "Sema/SemaExpr.h"

namespace fly {

	class ASTCast;

	class SemaCast :  public SemaExpr {

		friend class SemaBuilder;
		friend class Resolver;
		friend class SemaValidator;

		ASTCast &AST;

		explicit SemaCast(ASTCast &AST);

	public:

		~SemaCast() override = default;

		ASTCast &getAST() const;

	};

}

#endif //FLY_SEMA_CAST_H
