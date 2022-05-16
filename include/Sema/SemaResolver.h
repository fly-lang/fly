//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/Sema.h - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_RESOLVER_H
#define FLY_SEMA_RESOLVER_H

#include "Sema/Sema.h"

namespace fly {

    class Sema;
    class DiagnosticsEngine;
    class DiagnosticBuilder;
    class SourceLocation;
    class ASTContext;
    class ASTNameSpace;
    class ASTNode;
    class ASTBlock;
    class ASTLocalVar;
    class ASTVarRef;
    class ASTExpr;
    class ASTType;
    class CodeGen;

    class SemaResolver {

        Sema &S;

        SemaBuilder &Builder;

    public:

        SemaResolver(Sema &S, SemaBuilder &Builder);

        bool Resolve();

        bool ResolveImports(ASTNameSpace *NameSpace);

        bool ResolveGlobalVars(ASTNode *Node);

        bool ResolveGlobalVars(ASTNameSpace *NameSpace);

        bool ResolveFunctionCalls(ASTNode *Node);

        bool ResolveFunctionCalls(ASTNameSpace *NameSpace);

        bool ResolveBodyFunctions(ASTNode *Node);

        bool ResolveBlock(ASTBlock *Block);

        bool ResolveClass(ASTNode *Node);

        ASTType *ResolveExprType(ASTExpr *Expr);

        ASTLocalVar *FindVarDecl(ASTBlock *Block, ASTVarRef *VarRef);

        bool ResolveVarRef(ASTBlock *Block, ASTVarRef *VarRef);

        bool ResolveExpr(ASTBlock *Block, const ASTExpr *Expr);

        DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) const;

        DiagnosticBuilder Diag(unsigned DiagID) const;


    };

}  // end namespace fly

#endif