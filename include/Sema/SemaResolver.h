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

    class SemaResolver {

        friend class Sema;

        Sema &S;

        SemaResolver(Sema &S);

    public:

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

        bool ResolveType(ASTType * Type);

        bool FindFunction(ASTBlock *Block, ASTCall *Call, llvm::StringMapIterator<std::map<uint64_t, llvm::SmallVector<ASTFunction *, 4>>> StrMapIt);

        bool ResolveCall(ASTBlock *Block, ASTCall *Call);

        bool ResolveArg(ASTBlock *Block, ASTArg *Arg, ASTParam *Param);

        bool ResolveVarRef(ASTBlock *Block, ASTVarRef *VarRef);

        bool ResolveExpr(ASTBlock *Block, ASTExpr *Expr);

        bool ResolveValueExpr(ASTValueExpr *pExpr);

        ASTBlock *getBlock(ASTStmt *Stmt);

        ASTType *getType(ASTStmt *Stmt);

    };

}  // end namespace fly

#endif