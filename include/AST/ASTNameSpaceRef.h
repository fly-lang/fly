//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTNameSpaceRef.h - AST Namespace Ref header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_NAMESPACEREF_H
#define FLY_AST_NAMESPACEREF_H

#include "ASTRef.h"

namespace fly {

    class SemaNameSpace;

    class ASTNameSpaceRef : public ASTRef {

        friend class ASTBuilder;
        friend class Resolver;
        friend class SemaValidator;

        const llvm::SmallVector<llvm::StringRef, 4> Names;

        ASTNameSpaceRef(const SourceLocation &Loc, llvm::SmallVector<llvm::StringRef, 4> Names);

    public:

        ~ASTNameSpaceRef();

        const llvm::SmallVector<llvm::StringRef, 4> &getNames() const;

        std::string str() const;
    };
}

#endif //FLY_AST_NAMESPACEREF_H
