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
    class ASTNode;

    class ASTImport {

        const SourceLocation Location;

        std::string Name;

        std::string Alias;

        ASTNameSpace *NameSpace = nullptr;

    public:

        ASTImport(const SourceLocation &Loc, const std::string Name);

        ASTImport(const SourceLocation &Loc, const std::string Name, const std::string Alias);

        ~ASTImport();

        const SourceLocation &getLocation() const;

        const std::string &getName() const;

        const std::string &getAlias() const;

        ASTNameSpace *getNameSpace() const;

        void setNameSpace(ASTNameSpace *NS);

        std::string str() const;
    };
}

#endif //FLY_ASTIMPORT_H
