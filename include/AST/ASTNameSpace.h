//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTNameSpace.h - AST namespace declaration header
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

    class ASTName;

    class ASTNameSpace : public ASTNode {

        friend class ASTBuilder;

        llvm::SmallVector<ASTName *, 4> Names;

        ASTNameSpace(const SourceLocation &Loc, llvm::SmallVector<ASTName *, 4> Names);

    public:

        ~ASTNameSpace() override;

        void accept(ASTVisitor& Visitor) override;

        const llvm::SmallVector<ASTName *, 4> &getNames() const;

        std::string str() const;
    };
}

#endif //FLY_AST_NAMESPACE_H
