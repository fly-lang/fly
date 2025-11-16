//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTName.h - AST Name header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_NAME_H
#define FLY_AST_NAME_H

#include "ASTBase.h"

namespace fly {

	class ASTName : public ASTBase {

		friend class ASTBuilder;

		llvm::StringRef Name;

		explicit ASTName(const SourceLocation &Loc, llvm::StringRef Name);

	public:

		llvm::StringRef getName() const;

		std::string str() const override;
	};

}
#endif //FLY_AST_NAME_H