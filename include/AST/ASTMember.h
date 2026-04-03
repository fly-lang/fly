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
#include "ASTName.h"

namespace fly {

	class Symbol;

	class ASTMember : public ASTExpr {

		friend class ASTBuilder;

		llvm::StringRef Name;

		Symbol *ResolvedSymbol = nullptr;

	protected:

		ASTMember(const SourceLocation &Loc, llvm::StringRef Name, ASTExpr *Parent);

		~ASTMember();

	public:

		llvm::StringRef getName() const;

		void accept(ASTVisitor &Visitor) override;


		Symbol *getSymbol() const;

		void setSymbol(Symbol *Sym);

		std::string str() const override;
	};
}

#endif //FLY_AST_MEMBER_H
