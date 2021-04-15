//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTContext.h - AST Context
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
/// \file
/// Defines the fly::ASTContext interface.
///
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_ASTCONTEXT_H
#define FLY_ASTCONTEXT_H

#include "ASTNode.h"
#include "GlobalVarDecl.h"
#include "Frontend/CompilerInstance.h"

namespace fly {

    class ASTNode;

    class ASTContext {

        friend ASTNameSpace;
        friend ASTNode;

        // AST by FileID
        llvm::DenseMap<FileID, ASTNode*> Nodes;

        // All Context Namespaces
        llvm::StringMap<ASTNameSpace*> NameSpaces;

        // All Imports
        llvm::StringMap<ImportDecl*> Imports;

    public:

        const DenseMap<FileID, ASTNode *> &getNodes() const;

        bool AddNode(ASTNode &AST);

        bool DelNode(ASTNode &AST);
    };
}

#endif //FLY_ASTCONTEXT_H
