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
    class ASTClassMethod;
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
    class ASTLoopBlock;
    class ASTClassType;
    class ASTFunction;
    class ASTFunctionBase;
    class ASTIdentifier;
    class ASTImport;
    class ASTIdentityType;
    class ASTVar;
    class ASTIdentity;

    class SemaResolver {

        friend class Sema;

        Sema &S;

        SemaResolver(Sema &S);

    public:

        bool Resolve();

    private:

        bool ResolveNameSpace(ASTNode *Node, ASTIdentifier *&Identifier);

        bool ResolveImports(ASTNode *Node);

        bool ResolveGlobalVars(ASTNode *Node);

        bool ResolveIdentities(ASTNode *Node);

        bool ResolveFunctions(ASTNode *Node);

        bool ResolveStmt(ASTStmt *Stmt);

        bool ResolveBlock(ASTBlock *Block);

        bool ResolveIfBlock(ASTIfBlock *IfBlock);

        bool ResolveSwitchBlock(ASTSwitchBlock *SwitchBlock);

        bool ResolveLoopBlock(ASTLoopBlock *LoopBlock);

        bool ResolveParentIdentifier(ASTStmt *Parent, ASTIdentifier *&Identifier);

        bool ResolveIdentityType(ASTNode *Node, ASTIdentityType *IdentityType);

        bool ResolveVarRef(ASTStmt *Parent, ASTVarRef *VarRef);

        ASTVar *ResolveVarRefNoParent(ASTStmt *Parent, llvm::StringRef Name);

        ASTVar *ResolveVarRef(llvm::StringRef Name, ASTIdentityType *IdentityType);

        bool ResolveVarRefWithParent(ASTVarRef *VarRef);

        bool ResolveCall(ASTStmt *Parent, ASTCall *Call);

        bool ResolveCallNoParent(ASTStmt *Parent, ASTCall *Call);

        bool ResolveCall(ASTStmt *Parent, ASTCall *Call, ASTIdentityType *IdentityType);

        bool ResolveCall(ASTStmt *Parent, ASTCall *Call, ASTNameSpace *NameSpace);

        bool ResolveCallWithParent(ASTStmt *Parent, ASTCall *Call);

        template <class T>
        bool ResolveCall(ASTStmt *Parent, ASTCall *Call, llvm::StringMap<std::map <uint64_t,llvm::SmallVector <T *, 4>>> &Functions);

        template <class T>
        bool ResolveCall(ASTStmt *Parent, ASTCall *Call, std::map <uint64_t,llvm::SmallVector <T *, 4>> &Functions);

        bool ResolveArg(ASTStmt *Parent, ASTArg *Arg, ASTParam *Param);

        bool ResolveExpr(ASTStmt *Parent, ASTExpr *Expr);

        bool ResolveValueExpr(ASTValueExpr *pExpr);

        ASTNameSpace *FindNameSpace(llvm::StringRef Name) const;

        ASTNode *FindNode(ASTFunctionBase *FunctionBase) const;

        ASTNode *FindNode(llvm::StringRef Name, ASTNameSpace *NameSpace) const;

        ASTIdentity *FindIdentity(llvm::StringRef Name, ASTNameSpace *NameSpace) const;

        ASTIdentityType *FindIdentityType(llvm::StringRef Name, ASTNameSpace *NameSpace) const;

        ASTVar *FindLocalVar(ASTStmt *Parent, llvm::StringRef Name) const;

        ASTImport *FindImport(ASTNode *Node, llvm::StringRef Name);

    };

}  // end namespace fly

#endif