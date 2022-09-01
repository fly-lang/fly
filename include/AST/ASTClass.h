//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTClass.h - Class declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTCLASS_H
#define FLY_ASTCLASS_H

#include "ASTTopDef.h"

namespace fly {

    class ASTClass : public ASTTopDef {

        friend class SemaBuilder;

        std::string Name;

        bool Constant;

        ASTClass(const SourceLocation &Loc, ASTNode *Node, const std::string Name,
                 VisibilityKind Visibility, bool Constant);

    public:

        const std::string getName() const;
    };
}

#endif //FLY_ASTCLASS_H
