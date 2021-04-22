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
#include "Basic/DiagnosticParse.h"

namespace fly {

    class ASTNode;
    class ASTNameSpace;

    class ASTContext {

        friend ASTNameSpace;
        friend ASTNode;

        DiagnosticsEngine &Diags;

        // First inserted node, useful for Finalize on last
        ASTNode *FirstNode = NULL;

        // All Context Namespaces
        llvm::StringMap<ASTNameSpace *> NameSpaces;

        // All Files: <FileName, FileId>
        llvm::StringMap<ImportDecl *> Imports;

    public:
        ASTContext(DiagnosticsEngine &Diags);

        ~ASTContext();

        bool AddNode(ASTNode *Node);

        bool DelNode(ASTNode *Node);

        bool Finalize();

        const StringMap<ASTNameSpace *> &getNameSpaces() const;

    };
}

#endif //FLY_ASTCONTEXT_H
