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
    class SemaSymbols;
    class DiagnosticsEngine;
    class DiagnosticBuilder;
    class SourceLocation;
    class ASTContext;
    class ASTNameSpace;
    class ASTModule;
    class ASTTopDef;
    class ASTClass;
    class ASTClassMethod;
    class ASTStmt;
    class ASTBlockStmt;
    class ASTLocalVar;
    class ASTArg;
    class ASTParam;
    class ASTCall;
    class ASTVarRef;
    class ASTExpr;
    class ASTValueExpr;
    class ASTType;
    class CodeGen;
    class ASTIfStmt;
    class ASTSwitchStmt;
    class ASTLoopStmt;
    class ASTClassType;
    class ASTFunction;
    class ASTFunctionBase;
    class ASTIdentifier;
    class ASTImport;
    class ASTIdentityType;
    class ASTVar;
    class ASTIdentity;
    class ASTLoopInStmt;
    class ASTGlobalVar;
    class ASTEnum;

    class SemaResolver {

        friend class Sema;

        Sema &S;

        ASTModule *Module;

        SemaSymbols *NameSpace;

        llvm::StringMap<SemaSymbols *> Imports;

        SemaResolver(Sema &S, ASTModule *Module, SemaSymbols *Symbols);

    public:

        static bool Resolve(Sema &S);

    private:

        void ResolveGlobalVarDeclarations();

        void ResolveFunctionDeclarations();

        void ResolveIdentityDeclarations();

        void ResolveImportDefinitions();

        void ResolveGlobalVarDefinitions();

        void ResolveIdentityDefinitions();

        void ResolveFunctionDefinitions();

        void ResolveType(ASTType *Type);

        void ResolveIdentityType(ASTIdentityType *IdentityType);

        bool ResolveStmt(ASTStmt *Stmt);

        bool ResolveStmtBlock(ASTBlockStmt *Block);

        bool ResolveStmtIf(ASTIfStmt *IfStmt);

        bool ResolveStmtSwitch(ASTSwitchStmt *SwitchStmt);

        bool ResolveStmtLoop(ASTLoopStmt *LoopStmt);

        bool ResolveStmtLoopIn(ASTLoopInStmt *LoopInStmt);

        bool ResolveParentIdentifier(ASTIdentifier *Identifier, ASTStmt *Stmt = nullptr);

        bool ResolveIdentifier(SemaSymbols *Parent, ASTIdentifier *Identifier);

        bool ResolveIdentifier(ASTStmt *Stmt, ASTIdentifier *Identifier);

        bool ResolveIdentifier(ASTType *Parent, ASTIdentifier *Identifier);

        bool ResolveVarRef(ASTStmt *Stmt, ASTVarRef *VarRef);

        ASTVar *ResolveVarRef(llvm::StringRef Name, ASTIdentityType *IdentityType);

        bool ResolveCall(ASTStmt *Stmt, ASTCall *Call);

        bool ResolveCall(ASTStmt *Stmt, ASTCall *Call, ASTIdentityType *IdentityType);

        bool ResolveArg(ASTStmt *Stmt, ASTArg *Arg, ASTParam *Param);

        bool ResolveExpr(ASTStmt *Stmt, ASTExpr *Expr, ASTType *Type = nullptr);

        SemaSymbols *FindNameSpace(ASTIdentifier *Identifier) const;

        ASTGlobalVar *FindGlobalVar(llvm::StringRef Name, SemaSymbols *Symbols = nullptr) const;

        ASTIdentity *FindIdentity(llvm::StringRef Name, SemaSymbols *Symbols = nullptr) const;

        ASTFunction *FindFunction(ASTCall *Call, SemaSymbols *Symbols = nullptr) const;

        template <typename T>
        T *FindFunction(ASTCall *Call, llvm::StringMap<std::map<uint64_t, llvm::SmallVector<T *, 4>>> Functions) const;

        ASTVar *FindLocalVar(ASTStmt *Stmt, llvm::StringRef Name) const;

        SemaSymbols *AddImportSymbols(llvm::StringRef Name);

        ASTClassMethod * FindMethod(ASTCall *Call, ASTClass *Class);
    };

}  // end namespace fly

#endif