//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTContext.h - AST Context
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTCONTEXT_H
#define FLY_ASTCONTEXT_H

#include <Basic/SourceLocation.h>
#include <Basic/Diagnostic.h>
#include "ASTImport.h"
#include "llvm/ADT/StringMap.h"

namespace fly {

    class ASTNode;
    class ASTNameSpace;

    class ASTContext {

        friend ASTNameSpace;
        friend ASTNode;

        DiagnosticsEngine &Diags;

        // First inserted node, useful for Finalize on last
        ASTNode *FirstNode = NULL;

        ASTNameSpace * DefaultNS;

        // All Context Namespaces
        llvm::StringMap<ASTNameSpace *> NameSpaces;

        // All Files: <FileName, FileId>
        llvm::StringMap<ASTImport *> Imports;

    public:
        ASTContext(DiagnosticsEngine &Diags);

        ~ASTContext();

        ASTNameSpace *getDefaultNameSpace() const;

        bool AddNode(ASTNode *Node);

        bool DelNode(ASTNode *Node);

        bool Finalize();

        const llvm::StringMap<ASTNameSpace *> &getNameSpaces() const;

        DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) const;

        bool hasErrors() const;
    };
}

#endif //FLY_ASTCONTEXT_H
