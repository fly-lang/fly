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

    enum class ASTIdentifierKind {
        VAR,
        TYPE,
    };

    class ASTIdentifier : public ASTExpr {

        friend class ASTBuilder;

    protected:

        ASTIdentifierKind RefKind;

        const llvm::StringRef Name;

        ASTIdentifier *Parent = nullptr;

        ASTIdentifier *Child = nullptr;

        ASTIdentifier(const SourceLocation &Loc, llvm::StringRef Name, ASTIdentifierKind Kind);

        ASTIdentifier(const SourceLocation &Loc, llvm::StringRef Name);

        ~ASTIdentifier();

     public:

        void accept(ASTVisitor& Visitor) override;

        llvm::StringRef getName() const;

        ASTIdentifierKind getRefKind() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_IDENTIFIER_H
