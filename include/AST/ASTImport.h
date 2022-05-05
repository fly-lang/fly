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
#include "AST/ASTUnref.h"

#include <vector>

namespace fly {

    class ASTNameSpace;
    class ASTNode;

    class ASTImport {

        friend class Sema;
        friend class SemaBuilder;
        friend class SemaResolver;

        const SourceLocation NameLocation;

        const SourceLocation AliasLocation;

        std::string Name;

        std::string Alias;

        ASTNameSpace *NameSpace = nullptr;

        // Contains all unresolved VarRef to a GlobalVar
        std::vector<ASTUnrefGlobalVar *>  UnrefGlobalVars;

        // Contains all unresolved Function Calls
        std::vector<ASTUnrefCall *> UnrefFunctionCalls;

    public:

        ASTImport(const SourceLocation &NameLoc, const std::string Name);

        ASTImport(const SourceLocation &NameLoc, const std::string Name,
                  const SourceLocation &AliasLoc, const std::string Alias);

        ~ASTImport();

        const SourceLocation &getNameLocation() const;

        const SourceLocation &getAliasLocation() const;

        const std::string &getName() const;

        const std::string &getAlias() const;

        ASTNameSpace *getNameSpace() const;

        void setNameSpace(ASTNameSpace *NS);

        std::string str() const;
    };
}

#endif //FLY_ASTIMPORT_H
