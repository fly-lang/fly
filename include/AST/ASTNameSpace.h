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

#include "ASTBase.h"

namespace fly {

    class SymNameSpace;

    class ASTNameSpace : public ASTBase {

        friend class ASTBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        llvm::SmallVector<llvm::StringRef, 4> Names;

        ASTNameSpace *Parent = nullptr;

        ASTNameSpace(const SourceLocation &Loc, llvm::SmallVector<llvm::StringRef, 4> &Names);

    public:

        ~ASTNameSpace();

        const llvm::SmallVector<llvm::StringRef, 4> &getNames() const;

        std::string str() const;
    };
}

#endif //FLY_AST_NAMESPACE_H
