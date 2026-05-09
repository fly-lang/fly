//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTIdentifier.h - AST Identifier header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_IDENTIFIER_H
#define FLY_AST_IDENTIFIER_H

#include "ASTExpr.h"

namespace fly {

    class ASTVar;
    struct Symbol;

    class ASTIdentifier : public ASTExpr {

        friend class ASTBuilder;

    	Symbol *ResolvedSymbol = nullptr;

    protected:

        const llvm::StringRef Name;

        ASTVar *Var = nullptr;

        ASTIdentifier(const SourceLocation &Loc, llvm::StringRef Name);

        ~ASTIdentifier();

    public:

        void accept(ASTVisitor& Visitor) override;

        llvm::StringRef getName() const;

        ASTVar *getVar();

        Symbol *getSymbol() const;

    	void setSymbol(Symbol *Sym);

        std::string str() const override;
    };
}

#endif //FLY_AST_IDENTIFIER_H
