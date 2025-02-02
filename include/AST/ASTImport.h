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

    class ASTComment;
    class SymNameSpace;
    class ASTModule;
    class ASTAlias;

    class ASTImport : public ASTBase {

        friend class Sema;
        friend class ASTBuilder;
        friend class SemaResolver;

        ASTModule *Module;

        llvm::StringRef Name;

        ASTAlias *Alias = nullptr;

        ASTImport(const SourceLocation &Loc, llvm::StringRef Name);

    public:

        ~ASTImport();

        ASTModule* getModule() const;

        ASTAlias *getAlias() const;

        void setAlias(ASTAlias *Alias);

        std::string str() const override;
    };

}

#endif //FLY_AST_IMPORT_H
