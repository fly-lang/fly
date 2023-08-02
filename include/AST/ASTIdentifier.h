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
    class ASTReference;

    class ASTIdentifier : public Debuggable {

        friend class SemaBuilder;
        friend class SemaResolver;

    protected:

        const SourceLocation Loc;

        const llvm::StringRef Name;

        std::string PrintName;

        uint32_t Index = 0;

        ASTIdentifier *Root = nullptr;

        ASTIdentifier *Parent = nullptr;

        ASTIdentifier *Child = nullptr;

        ASTReference *Reference = nullptr;

        bool RefIsCall = false;

        ASTIdentifier(const SourceLocation &Loc, llvm::StringRef Name);

        ~ASTIdentifier();

    public:

        const SourceLocation &getLocation() const;

        llvm::StringRef getName() const;

        bool isCall() const;

        ASTVarRef *getVarRef() const;

        ASTCall *getCall() const;

        void setCall(ASTCall *Call);

        ASTIdentifier * AddChild(const SourceLocation &Loc, const StringRef Name);

        uint32_t getIndex() const;

        ASTIdentifier *getRoot() const;

        ASTIdentifier *getParent() const;

        ASTIdentifier *getChild() const;

        std::string print() const;

        std::string str() const;
    };
}

#endif //FLY_AST_IDENTIFIER_H
