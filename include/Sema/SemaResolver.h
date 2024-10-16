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
    class SemaSpaceSymbols;
    class SemaIdentitySymbols;
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
    class ASTFailStmt;
    class ASTHandleStmt;
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
    class ASTVarStmt;

    class SemaResolver {

        friend class Sema;

        Sema &S;

        ASTModule *Module;

        SemaSpaceSymbols *MySpaceSymbols;

        llvm::StringMap<SemaSpaceSymbols *> ImportSymbols;

        SemaResolver(Sema &S, ASTModule *Module, SemaSpaceSymbols *SpaceSymbols);

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

        bool ResolveStmt(ASTStmt *Stmt);

        bool ResolveStmtBlock(ASTBlockStmt *Block);

        bool ResolveStmtIf(ASTIfStmt *IfStmt);

        bool ResolveStmtSwitch(ASTSwitchStmt *SwitchStmt);

        bool ResolveStmtLoop(ASTLoopStmt *LoopStmt);

        bool ResolveStmtLoopIn(ASTLoopInStmt *LoopInStmt);

        bool ResolveStmtVar(ASTVarStmt *VarStmt);

        bool ResolveStmtFail(ASTFailStmt *FailStmt);

        bool ResolveStmtHandle(ASTHandleStmt *HandleStmt);

        bool ResolveIdentityType(ASTIdentityType *IdentityType);

        bool ResolveIdentifier(SemaSpaceSymbols *SpaceSymbols, ASTStmt *Stmt, ASTIdentifier *Identifier);

        bool ResolveIdentifier(SemaIdentitySymbols *IdentitySymbols, ASTStmt *Stmt, ASTIdentifier *Identifier);

        bool ResolveIdentifier(ASTStmt *Stmt, ASTIdentifier *Identifier);

        bool ResolveGlobalVarRef(SemaSpaceSymbols *SpaceSymbols, ASTStmt *Stmt, ASTVarRef *VarRef);

        bool ResolveStaticVarRef(SemaIdentitySymbols *IdentitySymbols, ASTStmt *Stmt,ASTVarRef *VarRef);

        bool ResolveVarRef(ASTStmt *Stmt, ASTVarRef *VarRef);

        bool ResolveFunctionCall(SemaSpaceSymbols *SpaceSymbols, ASTStmt *Stmt, ASTCall *Call);

        bool ResolveStaticCall(SemaIdentitySymbols *IdentitySymbols, ASTStmt *Stmt, ASTCall *Call);

        bool ResolveCall(ASTStmt *Stmt, ASTCall *Call);

        bool ResolveCallArgs(ASTStmt *Stmt, ASTCall *Call);

        bool ResolveExpr(ASTStmt *Stmt, ASTExpr *Expr, ASTType *Type = nullptr);

        SemaSpaceSymbols *FindSpaceSymbols(ASTIdentifier *Identifier, ASTIdentifier *&Current) const;

        ASTGlobalVar *FindGlobalVar(llvm::StringRef Name, SemaSpaceSymbols *SpaceSymbols) const;

        SemaIdentitySymbols *FindIdentity(llvm::StringRef Name, SemaSpaceSymbols *SpaceSymbols) const;

        ASTFunction *FindFunction(ASTCall *Call, SemaSpaceSymbols *SpaceSymbols) const;

        ASTClassMethod *FindClassMethod(ASTCall *Call, SemaIdentitySymbols *IdentitySymbols) const;

        template <typename T>
        T *FindFunction(ASTCall *Call, llvm::StringMap<std::map<uint64_t, llvm::SmallVector<T *, 4>>> Functions) const;

        ASTVar *FindLocalVar(ASTStmt *Stmt, llvm::StringRef Name) const;

        SemaSpaceSymbols *AddImportSymbols(llvm::StringRef Name);
    };

}  // end namespace fly

#endif