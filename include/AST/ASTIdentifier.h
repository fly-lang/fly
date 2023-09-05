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

#include "Basic/Debuggable.h"
#include "Basic/SourceLocation.h"

namespace fly {

    class ASTStmt;
    class ASTCall;
    class ASTVarRef;

    enum class ASTIdentifierKind {
        REF_UNDEF,
        REF_NAMESPACE,
        REF_TYPE,
        REF_CALL,
        REF_VAR
    };

    class ASTIdentifier : public Debuggable {

        friend class SemaBuilder;
        friend class SemaResolver;

    protected:

        const SourceLocation Loc;

        const llvm::StringRef Name;

        std::string FullName;

        ASTIdentifier *Parent = nullptr;

        ASTIdentifier *Child = nullptr;

        ASTIdentifierKind Kind = ASTIdentifierKind::REF_UNDEF;

        ASTIdentifier(const SourceLocation &Loc, llvm::StringRef Name);

        ASTIdentifier(const SourceLocation &Loc, llvm::StringRef Name, ASTIdentifierKind Kind);

        ~ASTIdentifier();

    public:

        const SourceLocation &getLocation() const;

        llvm::StringRef getName() const;

        std::string getFullName() const;

        bool isUndef() const;

        bool isNameSpace() const;

        bool isType() const;

        bool isCall() const;

        bool isVarRef() const;

        ASTIdentifierKind getKind() const;

        ASTIdentifier * AddChild(ASTIdentifier *Identifier);

        ASTIdentifier *getParent() const;

        ASTIdentifier *getChild() const;

        std::string print() const;

        std::string str() const;
    };
}

#endif //FLY_AST_IDENTIFIER_H
