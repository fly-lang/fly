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

#include "Basic/SourceLocation.h"

namespace fly {

    class ASTNameSpace;

    class ASTImport {

        const SourceLocation Location;

        llvm::StringRef Name;

        llvm::StringRef Alias;

        ASTNameSpace *NameSpace = nullptr;

    public:

        ASTImport(const SourceLocation &Loc, llvm::StringRef Name);

        ASTImport(const SourceLocation &Loc, llvm::StringRef Name, llvm::StringRef Alias);

        ~ASTImport();

        const SourceLocation &getLocation() const;

        const llvm::StringRef &getName() const;

        const llvm::StringRef &getAlias() const;

        ASTNameSpace *getNameSpace() const;

        void setNameSpace(ASTNameSpace *NS);
    };
}

#endif //FLY_ASTIMPORT_H
