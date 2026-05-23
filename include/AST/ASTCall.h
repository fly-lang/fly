//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTCall.h - AST function call expression header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_CALL_H
#define FLY_AST_CALL_H

#include "ASTExpr.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/SmallVector.h"

namespace fly {

    class ASTArg;
	struct Symbol;

    enum class ASTCallKind {
        CALL_DIRECT,
        CALL_NEW,
        CALL_NEW_UNIQUE,
        CALL_NEW_SHARED,
        CALL_NEW_WEAK
    };

    class ASTCall : public ASTExpr {

        friend class ASTBuilder;

    	Symbol *ResolvedSymbol = nullptr;

        const ASTCallKind CallKind;

        llvm::StringRef Name;

        llvm::SmallVector<ASTArg *, 8> Args;

        ASTCall(const SourceLocation &Loc, llvm::StringRef Name, ASTCallKind CallKind);

    public:

        void accept(ASTVisitor& Visitor) override;

        llvm::StringRef getName() const;

        llvm::SmallVector<ASTArg *, 8> getArgs() const;

        Symbol *getSymbol() const;

    	void setSymbol(Symbol *Sym);

        ASTCallKind getCallKind() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_CALL_H
