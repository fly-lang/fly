//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ClassDecl.h - Class declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CLASSDECL_H
#define FLY_CLASSDECL_H

#include "TopDecl.h"

namespace fly {

    class ClassDecl : public TopDecl {

        friend class ASTNode;
        friend class Parser;
        friend class ClassParser;
    public:
        VisibilityKind Visibility;
        bool Constant;
        llvm::StringRef Name;
        SourceLocation Location;




        const StringRef &getName() const;

    };

    class ClassRef {

        const llvm::StringRef Name;
        const ClassDecl *D;

    public:
        ClassRef(const llvm::StringRef &Name);

    };
}

#endif //FLY_CLASSDECL_H
