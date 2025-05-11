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

#include "AST/ASTRef.h"

namespace fly {

    class SemaVar;

    /**
     * Reference to ASTVar definition
     * Ex.
     *  ... = a + ...
     *  b = ...
     */
    class ASTVarRef : public ASTRef {

        friend class ASTBuilder;
        friend class SemaBuilderStmt;
        friend class SemaResolver;
        friend class SemaValidator;

        SemaVar **Sym = nullptr;

        ASTVarRef(const SourceLocation &Loc, llvm::StringRef Name);

    public:

        SemaVar *getSema() const;

        std::string str() const;
    };
}

#endif //FLY_AST_VARREF_H
