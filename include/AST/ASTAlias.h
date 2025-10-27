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

#include "ASTImport.h"

namespace fly {

	class ASTAlias {

		friend class ASTBuilder;

		llvm::StringRef Name;

		const SourceLocation &Loc;

		ASTAlias(const SourceLocation &Loc, llvm::StringRef Name);

	public:

		llvm::StringRef getName();

		const SourceLocation &getLocation() const;
	};

}

#endif //FLY_AST_ALIAS_H