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

#include "ASTTopDecl.h"

namespace fly {

    class ASTClass : public ASTTopDecl {

        friend class SemaBuilder;

        std::string Name;

        bool Constant;

    public:
        ASTClass(const SourceLocation &Loc, ASTNode *Node, const std::string &Name,
                 VisibilityKind Visibility, bool Constant);

        const std::string &getName() const;
    };

    class ClassRef {

        const std::string Name;
        const ASTClass *D;

    public:
        ClassRef(const std::string &Name);

    };
}

#endif //FLY_ASTCLASS_H
