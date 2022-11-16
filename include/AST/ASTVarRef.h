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

#include "AST/ASTIdentifier.h"

#include <string>

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

    public:
        ASTVarRef(const SourceLocation &Loc, llvm::StringRef NameSpace, llvm::StringRef Name);

        ASTVarRef(const SourceLocation &Loc, llvm::StringRef NameSpace, llvm::StringRef ClassName, llvm::StringRef Name);

        ASTVar *getDef() const;

        std::string str() const;
    };
}

#endif //FLY_AST_VARREF_H
