//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTVar.h - Var declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_AST_VARREF_H
#define FLY_AST_VARREF_H

#include "Basic/SourceLocation.h"

#include <string>

namespace fly {

    class ASTVar;

    /**
     * Reference to ASTVar definition
     * Ex.
     *  ... = a + ...
     *  b = ...
     */
    class ASTVarRef {

        friend class SemaBuilder;
        friend class SemaResolver;

        const SourceLocation Loc;
        const std::string NameSpace;
        const std::string Class;
        const std::string Name;

        ASTVar *Def = nullptr;

    public:
        ASTVarRef(const SourceLocation &Loc, const std::string Name, const std::string NameSpace);

        ASTVarRef(const SourceLocation &Loc, const std::string Name, const std::string Class, const std::string NameSpace);

        const SourceLocation &getLocation() const;

        const std::string getNameSpace() const;

        const std::string getClass() const;

        const std::string getName() const;

        ASTVar *getDef() const;

        std::string str() const;
    };
}

#endif //FLY_AST_VARREF_H
