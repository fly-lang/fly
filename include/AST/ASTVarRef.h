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

#include "AST/ASTReference.h"

#include <string>

namespace fly {

    class ASTVar;
    class ASTIdentifier;

    /**
     * Reference to ASTVar definition
     * Ex.
     *  ... = a + ...
     *  b = ...
     */
    class ASTVarRef : public ASTReference {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTVar *Def = nullptr;

        ASTVarRef(ASTIdentifier *Identifier);

        ASTVarRef(ASTVar *Var);

    public:

        ASTVar *getDef() const;

        bool isLocalVar();

        std::string print() const;

        std::string str() const;
    };
}

#endif //FLY_AST_VARREF_H
