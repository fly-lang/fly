//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTNameSpace.h - AST Namespace Ref header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_NAMESPACE_H
#define FLY_AST_NAMESPACE_H

#include "ASTIdentifier.h"

namespace fly {

    class SymNameSpace;

    class ASTNameSpace : public ASTBase {

        friend class ASTBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        const llvm::StringRef Name;

        ASTNameSpace *Parent = nullptr;

        ASTNameSpace(const SourceLocation &Loc, llvm::StringRef Name);

    public:

        ~ASTNameSpace();

        llvm::StringRef getName() const;

        ASTNameSpace *getParent() const;

        std::string str() const;
    };
}

#endif //FLY_AST_NAMESPACE_H
