//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTIdentifier.h - Identifier declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_AST_IDENTIFIER_H
#define FLY_AST_IDENTIFIER_H

#include "ASTBase.h"

namespace fly {

    enum class ASTIdentifierKind {
        REF_UNDEF,
        REF_NAMESPACE,
        REF_TYPE,
        REF_CALL,
        REF_VAR
    };

    class ASTIdentifier : public ASTBase {

        friend class SemaBuilder;
        friend class SemaResolver;

    protected:

        const llvm::StringRef Name;

        bool Resolved = false;

        std::string FullName;

        ASTIdentifier *Parent = nullptr;

        ASTIdentifier *Child = nullptr;

        ASTIdentifierKind Kind = ASTIdentifierKind::REF_UNDEF;

        ASTIdentifier(const SourceLocation &Loc, llvm::StringRef Name);

        ASTIdentifier(const SourceLocation &Loc, llvm::StringRef Name, ASTIdentifierKind Kind);

        ~ASTIdentifier();

    public:

        llvm::StringRef getName() const;

        std::string getFullName() const;

        bool isUndef() const;

        bool isNameSpace() const;

        bool isType() const;

        bool isCall() const;

        bool isVarRef() const;

        ASTIdentifierKind getIdKind() const;

        ASTIdentifier * AddChild(ASTIdentifier *Identifier);

        ASTIdentifier *getParent() const;

        ASTIdentifier *getChild() const;

        std::string print() const;

        std::string str() const;
    };
}

#endif //FLY_AST_IDENTIFIER_H
