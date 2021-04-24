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

#include "AST/ASTNode.h"
#include "AST/GlobalVarDecl.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"

namespace fly {

    class ASTNode;

    class ASTNameSpace {

        friend ASTContext;
        friend ASTNode;

        llvm::StringRef NameSpace;

        // AST by FileID
        llvm::StringMap<ASTNode *> Nodes;

        // Public Global Vars
        llvm::StringMap<GlobalVarDecl *> Vars;

    public:
        ASTNameSpace(llvm::StringRef NS);

        ~ASTNameSpace();

        const llvm::StringRef &getNameSpace() const;

        const llvm::StringMap<ASTNode *> &getNodes() const;
    };
}

#endif //FLY_ASTNAMESPACE_H