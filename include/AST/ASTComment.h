//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTComment.h - AST Global Var header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_COMMENT_H
#define FLY_AST_COMMENT_H

#include "ASTBase.h"

#include "llvm/ADT/StringRef.h"

namespace fly {

    class ASTModule;
    class SourceLocation;

    class ASTComment : public ASTBase {

        friend class ASTBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        llvm::StringRef Content;

        ASTComment(const SourceLocation &Loc, llvm::StringRef Content);

    public:

        ~ASTComment() = default;

        llvm::StringRef getContent() const;

        std::string str() const override;

    };
}

#endif //FLY_AST_GLOBALVAR_H
