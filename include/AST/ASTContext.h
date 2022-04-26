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

#include "AST/ASTResolver.h"
#include "llvm/ADT/StringMap.h"

#include <vector>

namespace fly {

    class SourceLocation;
    class DiagnosticsEngine;
    class DiagnosticBuilder;
    class ASTNode;
    class ASTNameSpace;
    class ASTVarRef;
    class ASTFunc;
    class ASTImport;
    class ASTUnrefGlobalVar;
    class ASTUnrefCall;

    /**
     * AST Context
     */
    class ASTContext {

        friend class ASTResolver;
        friend class ASTNameSpace;
        friend class ASTNode;

        DiagnosticsEngine &Diags;

        ASTNameSpace * DefaultNS;

        // All Context Namespaces
        llvm::StringMap<ASTNameSpace *> NameSpaces;

        // All Files: <Name, ASTImport>
        llvm::StringMap<ASTImport *> ExternalImports; // TODO

    public:
        ASTContext(DiagnosticsEngine &Diags);

        ~ASTContext();

        ASTNameSpace *getDefaultNameSpace() const;

        const llvm::StringMap<ASTNameSpace *> &getNameSpaces() const;

        DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) const;

        DiagnosticBuilder Diag(unsigned DiagID) const;

        ASTNameSpace *AddNameSpace(std::string Name, bool ExternLib = false);

        bool AddNode(ASTNode *Node);

        bool DelNode(ASTNode *Node);

        bool Resolve();
    };
}

#endif //FLY_ASTCONTEXT_H
