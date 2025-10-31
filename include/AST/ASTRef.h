//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTRef.h - AST Identifier header
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

    enum class ASTRefKind {
        REF_VAR,
        REF_CALL,
        REF_TYPE,
        REF_NAMESPACE
    };

    class ASTRef : public ASTNode {

        friend class ASTBuilder;

    protected:

        ASTRefKind RefKind;

        const llvm::StringRef Name;

        std::string FullName;

        SemaResult *Sema = nullptr;

        ASTRef *Parent = nullptr;

        ASTRef *Child = nullptr;

        bool Resolved = false;

        ASTRef(const SourceLocation &Loc, llvm::StringRef Name, ASTRefKind Kind);

        ~ASTRef();

     public:

        void accept(ASTVisitor& Visitor) override;

        llvm::StringRef getName() const;

        std::string getFullName() const;

        SemaResult *getSema() const;

        bool isResolved() const;

        bool isCall() const;

        bool isVarRef() const;

        ASTRefKind getRefKind() const;

        void AddChild(ASTRef *Identifier);

        ASTRef *getParent() const;

        ASTRef *getChild() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_IDENTIFIER_H
