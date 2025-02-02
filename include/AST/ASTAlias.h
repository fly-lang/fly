//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTAlias.h - AST Alias in Import
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_ALIAS_H
#define FLY_AST_ALIAS_H

#include "ASTBase.h"

namespace fly {

	class ASTAlias : public ASTBase {

		friend class SemaBuilder;
		friend class SemaResolver;
		friend class SemaValidator;

		llvm::StringRef Name;

		ASTAlias(const SourceLocation &Loc, llvm::StringRef Name);

	public:

		llvm::StringRef getName() const;

		std::string str() const override;
	};

}

#endif //FLY_AST_ALIAS_H