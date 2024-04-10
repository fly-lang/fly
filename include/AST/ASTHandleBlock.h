//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTIfBlock.h - AST If Block Statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTHANDLEBLOCK_H
#define FLY_ASTHANDLEBLOCK_H

#include "ASTBlock.h"

namespace fly {

    class ASTVar;

    class ASTHandleBlock : public ASTBlock {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTVarRef *ErrorRef = nullptr;

        ASTHandleBlock(ASTBlock *Parent, const SourceLocation &Loc, ASTVarRef *Error);

    public:

        ASTBlock *getParent() const override;

        ASTVarRef *getErrorRef() const;

        std::string str() const;
    };
}


#endif //FLY_ASTHANDLEBLOCK_H
