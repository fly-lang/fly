//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTMember.h - AST Member header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_MEMBER_H
#define FLY_AST_MEMBER_H

#include "ASTExpr.h"

namespace fly {

	class ASTVar;
	class SemaVar;

	class ASTMember : public ASTExpr {

		friend class ASTBuilder;

	protected:

		const llvm::StringRef Name;

		ASTVar *Var;

		SemaVar *Sema;

		ASTMember(const SourceLocation &Loc, llvm::StringRef Name, ASTExpr *Parent);

		~ASTMember();

	public:

		void accept(ASTVisitor &Visitor) override;

		llvm::StringRef getName() const;

		ASTVar *getVar();

		void setSema(SemaVar *Sema);

		SemaVar *getSema() const;

		std::string str() const override;
	};
}

#endif //FLY_AST_MEMBER_H
