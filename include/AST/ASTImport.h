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

#include "ASTBase.h"

namespace fly {

    class ASTNameSpace;
    class ASTModule;

    class ASTAlias : public ASTBase {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        llvm::StringRef Name;

        ASTAlias(const SourceLocation &Loc, llvm::StringRef Name);

    public:

        llvm::StringRef getName() const;

        std::string str() const override;
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

        std::string str() const override;
    };
}

#endif //FLY_AST_IMPORT_H
