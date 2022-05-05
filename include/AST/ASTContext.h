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

#include "llvm/ADT/StringMap.h"

#include <vector>

namespace fly {

    class SourceLocation;
    class DiagnosticsEngine;
    class DiagnosticBuilder;
    class ASTNode;
    class ASTNameSpace;
    class ASTVarRef;
    class ASTFunction;
    class ASTImport;
    class ASTUnrefGlobalVar;
    class ASTUnrefCall;

    /**
     * AST Context
     */
    class ASTContext {

        friend class Sema;
        friend class SemaResolver;
        friend class SemaBuilder;

        ASTNameSpace * DefaultNS;

        // All Context Namespaces
        llvm::StringMap<ASTNameSpace *> NameSpaces;

        // All Context Namespaces
        llvm::StringMap<ASTNode *> Nodes;

        // All Files: <Name, ASTImport>
        llvm::StringMap<ASTImport *> ExternalImports; // TODO

    public:
        ASTContext();

        ~ASTContext();

        ASTNameSpace *getDefaultNameSpace() const;

        const llvm::StringMap<ASTNameSpace *> &getNameSpaces() const;

        ASTNameSpace *AddNameSpace(std::string Name, bool ExternLib = false);

        const llvm::StringMap<ASTNode *> &getNodes() const;

        bool AddNode(ASTNode *Node);
    };
}

#endif //FLY_ASTCONTEXT_H
