//
// Created by marco on 4/14/21.
//

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
        ASTNameSpace(const llvm::StringRef &NS);

        const llvm::StringRef &getNameSpace() const;

        const llvm::StringMap<ASTNode *> &getNodes() const;
    };
}

#endif //FLY_ASTNAMESPACE_H
