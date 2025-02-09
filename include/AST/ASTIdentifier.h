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

#include "ASTBase.h"

namespace fly {

    enum class ASTRefKind {
        REF_UNDEFINED,
        REF_NAMESPACE,
        REF_TYPE,
        REF_CALL,
        REF_VAR
    };

    class ASTIdentifier : public ASTBase {

        friend class ASTBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

    protected:

        ASTRefKind RefKind;

        const llvm::StringRef Name;

        std::string FullName;

        ASTIdentifier *Parent = nullptr;

        ASTIdentifier *Child = nullptr;

        bool Resolved = false;

        ASTIdentifier(const SourceLocation &Loc, llvm::StringRef Name, ASTRefKind Kind);

        ~ASTIdentifier();

     public:

        llvm::StringRef getName() const;

        std::string getFullName() const;

        bool isResolved() const;

        bool isNameSpace() const;

        bool isType() const;

        bool isCall() const;

        bool isVarRef() const;

        ASTRefKind getRefKind() const;

        void AddChild(ASTIdentifier *Identifier);

        ASTIdentifier *getParent() const;

        ASTIdentifier *getChild() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_IDENTIFIER_H
