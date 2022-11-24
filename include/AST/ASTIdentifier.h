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

#include "llvm/ADT/SmallVector.h"

namespace fly {

    class ASTIdentifier : public Debuggable {

        friend class SemaBuilder;
        friend class SemaResolver;

    protected:

        const SourceLocation Loc;
        const llvm::StringRef ClassName;
        const llvm::StringRef Name;
        llvm::StringRef NameSpace;

    public:

        ASTIdentifier(const SourceLocation &Loc, llvm::StringRef Name);

        ASTIdentifier(const SourceLocation &Loc, llvm::StringRef ClassName, llvm::StringRef Name);

        const SourceLocation &getLocation() const;

        llvm::StringRef getNameSpace() const;

        void setNameSpace(llvm::StringRef NameSpace);

        llvm::StringRef getClassName() const;

        llvm::StringRef getName() const;

        std::string str() const;
    };
}

#endif //FLY_AST_IDENTIFIER_H
