//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTContext.h - AST Context header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_CONTEXT_H
#define FLY_AST_CONTEXT_H

#include "llvm/ADT/StringMap.h"

namespace fly {

    class SourceLocation;
    class DiagnosticsEngine;
    class DiagnosticBuilder;
    class ASTModule;
    class ASTNameSpace;
    class ASTVarRef;
    class ASTFunction;
    class ASTImport;

    /**
     * AST Context
     */
    class ASTContext {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTNameSpace * DefaultNameSpace = nullptr;

        // All Context Namespaces
        llvm::StringMap<ASTNameSpace *> NameSpaces;

        // All Context Namespaces
        llvm::StringMap<ASTModule *> Modules;

        // All Files: <Name, ASTImport>
        llvm::StringMap<ASTImport *> ExternalImports; // TODO

        ASTContext();

    public:

        ~ASTContext();

        ASTNameSpace *getDefaultNameSpace() const;

        const llvm::StringMap<ASTNameSpace *> &getNameSpaces() const;

        const llvm::StringMap<ASTModule *> &getModules() const;
    };
}

#endif //FLY_ASTCONTEXT_H
