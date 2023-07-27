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

#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/SmallVector.h"

#include <map>

namespace llvm {
    class StringRef;
}

namespace fly {

    class Sema;
    class SemaBuilder;
    class DiagnosticsEngine;
    class DiagnosticBuilder;
    class SourceLocation;
    class ASTContext;
    class ASTNameSpace;
    class ASTNode;
    class ASTClass;
    class ASTClassFunction;
    class ASTStmt;
    class ASTBlock;
    class ASTLocalVar;
    class ASTArg;
    class ASTParam;
    class ASTCall;
    class ASTVarRef;
    class ASTExpr;
    class ASTValueExpr;
    class ASTType;
    class CodeGen;
    class ASTIfBlock;
    class ASTSwitchBlock;
    class ASTWhileBlock;
    class ASTForBlock;
    class ASTClassType;
    class ASTFunction;
    class ASTFunctionBase;
    class ASTIdentifier;
    class ASTReference;
    class ASTImport;
    class ASTIdentityType;

    class SemaResolver {

        friend class Sema;

        Sema &S;

        SemaResolver(Sema &S);

    public:

        bool Resolve();

    private:

        bool ResolveImports(ASTNode *Node);

        bool ResolveIdentities(ASTNode *Node);

        bool ResolveFunctions(ASTNode *Node);

        bool ResolveBlock(ASTBlock *Block);

        bool ResolveIfBlock(ASTIfBlock *IfBlock);

        bool ResolveSwitchBlock(ASTSwitchBlock *SwitchBlock);

        bool ResolveWhileBlock(ASTWhileBlock *WhileBlock);

        bool ResolveForBlock(ASTForBlock *ForBlock);

        bool ResolveIdentifier(ASTNode *Node, ASTIdentifier *Identifier, ASTNameSpace *&NameSpace);

        bool ResolveIdentifier(ASTNode *Node, ASTIdentifier *Identifier, ASTNameSpace *NameSpace, ASTIdentityType *&IdentityType);

        bool ResolveIdentifier(ASTBlock *Block, ASTIdentifier *Identifier);

        bool ResolveIdentityType(ASTNode *Node, ASTIdentityType * IdentityType);

        bool ResolveVarRef(ASTBlock *Block, ASTVarRef *VarRef);

        bool ResolveCall(ASTBlock *Block, ASTCall *Call);

        template <class T>
        bool ResolveCall(ASTBlock *Block, ASTCall *Call, llvm::StringMap<std::map <uint64_t,llvm::SmallVector <T *, 4>>> &Functions);

        template <class T>
        bool ResolveCall(ASTBlock *Block, ASTCall *Call, std::map <uint64_t,llvm::SmallVector <T *, 4>> &Functions);

//        bool ResolveCall(ASTBlock *Block, ASTCall *Call, llvm::StringMap<ASTImport *> Imports);

        bool ResolveArg(ASTBlock *Block, ASTArg *Arg, ASTParam *Param);

        bool ResolveExpr(ASTBlock *Block, ASTExpr *Expr);

        bool ResolveValueExpr(ASTValueExpr *pExpr);

        ASTBlock *getBlock(ASTStmt *Stmt);

        ASTType *getType(ASTStmt *Stmt);

    };

}  // end namespace fly

#endif