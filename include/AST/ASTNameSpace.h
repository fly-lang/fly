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

    class ASTIdentifier;

    class ASTNameSpace : public ASTNode {

        friend class ASTBuilder;

        ASTIdentifier *Identifier;

        ASTNameSpace(const SourceLocation &Loc, ASTIdentifier *Identifier);

    public:

        ~ASTNameSpace() override;

        void accept(ASTVisitor& Visitor) override;

        ASTIdentifier *getIdentifier() const;

        std::string str() const;
    };
}

#endif //FLY_AST_NAMESPACE_H
