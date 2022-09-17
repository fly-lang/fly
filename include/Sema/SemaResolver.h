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
#include "AST/ASTClassField.h"


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

    private:

        bool ResolveImports(ASTNode *Node);

        bool ResolveClass(ASTNode *Node);

        bool ResolveFunctions(ASTNode *Node);

        bool ResolveBlock(ASTBlock *Block);

        bool ResolveIfBlock(ASTIfBlock *IfBlock);

        bool ResolveSwitchBlock(ASTSwitchBlock *SwitchBlock);

        bool ResolveWhileBlock(ASTWhileBlock *WhileBlock);

        bool ResolveForBlock(ASTForBlock *ForBlock);

        bool ResolveFunctionCall(ASTFunctionCall *Call);

        bool ResolveArg(ASTArg *Arg, ASTParam *Param);

        bool ResolveVarRef(ASTBlock *Block, ASTVarRef *VarRef);

        bool ResolveExpr(ASTExpr *Expr);

        ASTBlock *getBlock(ASTStmt *Stmt);

        ASTType *getType(ASTStmt *Stmt);

        ASTLocalVar *FindVarDef(ASTBlock *Block, ASTVarRef *VarRef);

        DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) const;

        DiagnosticBuilder Diag(unsigned DiagID) const;

        bool ResolveValueExpr(ASTValueExpr *pExpr);
    };

}  // end namespace fly

#endif