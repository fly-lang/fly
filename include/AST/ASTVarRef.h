//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTVarRef.h - AST Var Ref header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_VARREF_H
#define FLY_AST_VARREF_H

#include "AST/ASTIdentifier.h"

namespace fly {

    class ASTVar;

    /**
     * Reference to ASTVar definition
     * Ex.
     *  ... = a + ...
     *  b = ...
     */
    class ASTVarRef : public ASTIdentifier {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTVar *Def = nullptr;

        ASTVarRef(const SourceLocation &Loc, llvm::StringRef Name);

    public:

        ASTVar *getDef() const;

        std::string print() const;

        std::string str() const;
    };
}

#endif //FLY_AST_VARREF_H
