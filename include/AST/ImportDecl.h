//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ImportDecl.h - AST Import declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_IMPORTDECL_H
#define FLY_IMPORTDECL_H

#include "TopDecl.h"
#include "ASTNameSpace.h"

namespace fly {

    class ASTNameSpace;

    class ImportDecl : public TopDecl {

        llvm::StringRef Name;

        llvm::StringRef Alias;

        ASTNameSpace *NameSpace = nullptr;

    public:

        ImportDecl(const SourceLocation &Loc, llvm::StringRef Name) : TopDecl(Loc), Name(Name), Alias(Name) {}

        ImportDecl(const SourceLocation &Loc, llvm::StringRef Name, llvm::StringRef Alias) : TopDecl(Loc), Name(Name),
                                                                                 Alias(Alias) {}

        ~ImportDecl() {
            NameSpace = nullptr;
        }

        TopDeclKind getKind() const override {
            return TopDeclKind::DECL_IMPORT;
        }

        const llvm::StringRef &getName() const {
            return Name;
        }

        const llvm::StringRef &getAlias() const {
            return Alias;
        }

        ASTNameSpace *getNameSpace() const {
            return NameSpace;
        }

        void setNameSpace(ASTNameSpace *NS) {
            NameSpace = NS;
        }
    };
}

#endif //FLY_IMPORTDECL_H
