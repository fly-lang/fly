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

#include "ASTIdentifier.h"

namespace fly {

    class SymNameSpace;

    class ASTNameSpaceRef : public ASTIdentifier {

        friend class ASTBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        const llvm::SmallVector<llvm::StringRef, 4> Names;

        ASTNameSpaceRef(const SourceLocation &Loc, llvm::SmallVector<llvm::StringRef, 4> Namew);

    public:

        ~ASTNameSpaceRef();

        const llvm::SmallVector<llvm::StringRef, 4> &getNames() const;

        std::string str() const;
    };
}

#endif //FLY_AST_NAMESPACEREF_H
