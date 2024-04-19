//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTImport.h - AST Import declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_ASTIMPORT_H
#define FLY_ASTIMPORT_H

#include "ASTBase.h"

namespace fly {

    class ASTNameSpace;
    class ASTNode;

    class ASTAlias : public ASTBase {

        friend class Sema;
        friend class SemaBuilder;
        friend class SemaResolver;

        llvm::StringRef Name;

        ASTAlias(const SourceLocation &Loc, llvm::StringRef Name);

    public:

        llvm::StringRef getName() const;

        std::string str() const;
    };

    class ASTImport : public ASTBase {

        friend class Sema;
        friend class SemaBuilder;
        friend class SemaResolver;

        llvm::StringRef Name;

        ASTNameSpace *NameSpace = nullptr;

        ASTAlias *Alias = nullptr;

        ASTImport(const SourceLocation &Loc, llvm::StringRef Name);

    public:

        ~ASTImport();

        llvm::StringRef getName() const;

        const ASTAlias *getAlias() const;

        void setAlias(ASTAlias *Alias);

        ASTNameSpace *getNameSpace() const;

        void setNameSpace(ASTNameSpace *NS);

        std::string str() const;
    };
}

#endif //FLY_ASTIMPORT_H
