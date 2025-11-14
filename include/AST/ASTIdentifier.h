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

#include <Sema/SemaResult.h>

#include "ASTNode.h"

namespace fly {

    enum class ASTIdentifierKind {
        VAR,
        CALL,
        TYPE,
    };

    class ASTIdentifier : public ASTNode {

        friend class ASTBuilder;

    protected:

        ASTIdentifierKind RefKind;

        const llvm::StringRef Name;

        std::string FullName;

        SemaResult *Sema = nullptr;

        ASTIdentifier *Parent = nullptr;

        ASTIdentifier *Child = nullptr;

        bool Visited = false;

        ASTIdentifier(const SourceLocation &Loc, llvm::StringRef Name, ASTIdentifierKind Kind);

        ~ASTIdentifier();

     public:

        void accept(ASTVisitor& Visitor) override;

        llvm::StringRef getName() const;

        void setSema(SemaResult *Sema);

        std::string getFullName() const;

        SemaResult *getSema() const;

        bool isVisited() const;

        void setVisited(bool Visited);

        bool isCall() const;

        bool isVarRef() const;

        ASTIdentifierKind getRefKind() const;

        void setChild(ASTIdentifier *Identifier);

        void AddChild(ASTIdentifier *Identifier);

        ASTIdentifier *getParent() const;

        ASTIdentifier *getChild() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_IDENTIFIER_H
