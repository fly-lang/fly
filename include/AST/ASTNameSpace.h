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

#include "ASTNode.h"

namespace fly {

    class SemaNameSpace;

    class ASTNameSpace : public ASTNode {

        friend class ASTBuilder;

        llvm::SmallVector<llvm::StringRef, 4> Names;

        ASTNameSpace *Parent = nullptr;

        ASTNameSpace(const SourceLocation &Loc, llvm::SmallVector<llvm::StringRef, 4> &Names);

    public:

        ~ASTNameSpace() override;

        void accept(ASTVisitor& Visitor) override;

        const llvm::SmallVector<llvm::StringRef, 4> &getNames() const;

        std::string str() const;
    };
}

#endif //FLY_AST_NAMESPACE_H
