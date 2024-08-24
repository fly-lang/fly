//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/Sema.h - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_H
#define FLY_SEMA_H

#include "llvm/ADT/StringMap.h"

namespace llvm {
    class StringRef;
}

namespace fly {

    class SemaBuilder;
    class SemaResolver;
    class SemaValidator;
    class SemaBuilderIfStmt;
    class SemaBuilderSwitchStmt;
    class SemaBuilderLoopStmt;
    class SemaSpaceSymbols;
    class DiagnosticsEngine;
    class DiagnosticBuilder;
    class SourceLocation;
    class ASTContext;
    class ASTNameSpace;
    class ASTModule;
    class ASTIdentity;
    class ASTClassAttribute;
    class ASTClassMethod;
    class ASTFunctionBase;
    class ASTFunction;
    class ASTImport;
    class ASTLocalVar;
    class ASTVarRef;
    class ASTVar;
    class ASTBlockStmt;
    class ASTIdentifier;
    class ASTIdentityType;

    class Sema {

        friend class SemaBuilder;
        friend class SemaBuilderIfStmt;
        friend class SemaBuilderSwitchStmt;
        friend class SemaBuilderLoopStmt;
        friend class SemaResolver;

        DiagnosticsEngine &Diags;

        ASTContext *Context = nullptr;

        SemaBuilder *Builder = nullptr;

        SemaValidator *Validator = nullptr;

        // Default Symbols
        SemaSpaceSymbols *DefaultSymbols;

        // Symbols organized by NameSpace
        llvm::StringMap<SemaSpaceSymbols *> MapSymbols;

        Sema(DiagnosticsEngine &Diags);

    public:

        static Sema* CreateSema(DiagnosticsEngine &Diags);

        SemaBuilder &getBuilder();

        DiagnosticsEngine &getDiags() const;

        SemaValidator &getValidator() const;

        ASTContext &getContext() const;

        bool Resolve();

        DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) const;

        DiagnosticBuilder Diag(unsigned DiagID) const;
    };

}  // end namespace fly

#endif // FLY_SEMA_H