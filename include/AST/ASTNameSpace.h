//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTNameSpace.h - AST Namespace
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTNAMESPACE_H
#define FLY_ASTNAMESPACE_H

#include "AST/ASTNodeBase.h"

namespace fly {

    class ASTNode;
    class ASTContext;
    class ASTGlobalVar;
    class ASTClass;
    class ASTUnrefGlobalVar;
    class ASTUnrefCall;

    class ASTNameSpace : public ASTNodeBase {

        friend class ASTResolver;
        friend class ASTContext;
        friend class ASTNode;

        // AST by FileID
        llvm::StringMap<ASTNode *> Nodes;

    public:
        ASTNameSpace(const llvm::StringRef &Name, ASTContext *Context);

        ~ASTNameSpace();

        static const llvm::StringRef DEFAULT;

        const llvm::StringMap<ASTNode *> &getNodes() const;
    };
}

#endif //FLY_ASTNAMESPACE_H
