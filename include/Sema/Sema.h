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

#include <llvm/ADT/SmallVector.h>

#include "llvm/ADT/StringMap.h"

namespace llvm {
    class StringRef;
}

namespace fly {

    class SymBuilder;
    class ASTBuilder;
    class SemaResolver;
    class SemaValidator;
    class SemaBuilderIfStmt;
    class SemaBuilderSwitchStmt;
    class SemaBuilderLoopStmt;
    class DiagnosticsEngine;
    class DiagnosticBuilder;
    class SourceLocation;
    class SymTable;
    class SymNameSpace;
    class ASTModule;
    class ASTFunction;
    class ASTFunction;
    class ASTImport;
    class ASTVarRef;
    class ASTVar;
    class ASTBlockStmt;
    class ASTIdentifier;
    class ASTTypeRef;

    class Sema {

        friend class ASTBuilder;
        friend class SemaBuilderIfStmt;
        friend class SemaBuilderSwitchStmt;
        friend class SemaBuilderLoopStmt;
        friend class SemaResolver;
        friend class SymBuilder;

        DiagnosticsEngine &Diags;

        llvm::SmallVector<ASTModule *, 4> Modules;

        SymTable *Table = nullptr;

        ASTBuilder *Builder = nullptr;

        SymBuilder *SymBuildr = nullptr;

        SemaValidator *Validator = nullptr;

        Sema(DiagnosticsEngine &Diags);

    public:
        ~Sema();

        static Sema* CreateSema(DiagnosticsEngine &Diags);

        ASTBuilder &getASTBuilder();

        SymBuilder &getSymBuilder();

        DiagnosticsEngine &getDiags() const;

        SemaValidator &getValidator() const;

        SymTable &getSymTable() const;

        const llvm::SmallVector<ASTModule*, 4> &getModules() const;

        bool Resolve();

        DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) const;

        DiagnosticBuilder Diag(unsigned DiagID) const;
    };

}  // end namespace fly

#endif // FLY_SEMA_H