//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTNode.h - AST Base header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_NODE_H
#define FLY_AST_NODE_H

#include "ASTBase.h"

namespace fly {

    class ASTVisitor;

    class ASTNode : public ASTBase {

        bool Visited = false;

    protected:

        explicit ASTNode(const SourceLocation &Loc, ASTKind Kind);

    public:
        virtual ~ASTNode() = default;

        virtual void accept(ASTVisitor& Visitor) = 0;

        virtual bool isVisited() const;

        virtual void setVisited(bool Visited);

    };

}

#endif //FLY_AST_NODE_H