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
    class ASTFuncCall;
    class ASTImport;

    class ASTContext {

        friend ASTNameSpace;
        friend ASTNode;

        DiagnosticsEngine &Diags;

        ASTNameSpace * DefaultNS;

        // All Context Namespaces
        llvm::StringMap<ASTNameSpace *> NameSpaces;

        // All Files: <Name, ASTImport>
        llvm::StringMap<ASTImport *> Imports;

        // Contains all unresolved VarRef with GlobalVar
        std::vector<ASTVarRef *> UnRefGlobalVars;

        // Contains all unresolved Calls with Function
        std::vector<ASTFuncCall *> UnRefCalls;

    public:
        ASTContext(DiagnosticsEngine &Diags);

        ~ASTContext();

        ASTNameSpace *getDefaultNameSpace() const;

        bool AddNode(ASTNode *Node);

        bool DelNode(ASTNode *Node);

        bool Resolve();

        void addUnRefCall(ASTFuncCall *Call);

        void addUnRefGlobalVar(ASTVarRef *Var);

        const llvm::StringMap<ASTNameSpace *> &getNameSpaces() const;

        DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) const;

    };
}

#endif //FLY_ASTCONTEXT_H
