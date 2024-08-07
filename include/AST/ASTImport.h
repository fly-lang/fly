//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTImport.h - AST Import header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_IMPORT_H
#define FLY_AST_IMPORT_H

#include "ASTIdentifier.h"

namespace fly {

    class ASTNameSpace;
    class ASTModule;
    class ASTAlias;

    class ASTImport : public ASTIdentifier {

        friend class Sema;
        friend class SemaBuilder;
        friend class SemaResolver;

        llvm::StringRef Name;

        ASTAlias *Alias = nullptr;

        ASTImport(const SourceLocation &Loc, llvm::StringRef Name);

    public:

        ~ASTImport();

        const ASTAlias *getAlias() const;

        void setAlias(ASTAlias *Alias);

        std::string str() const override;
    };

    class ASTAlias : public ASTIdentifier {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        llvm::StringRef Name;

        ASTImport *Import;

        ASTAlias(ASTImport *Import, const SourceLocation &Loc, llvm::StringRef Name);

    public:

        ASTImport *getImport() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_IMPORT_H
