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

#include "SemaErrorHandler.h"
#include "llvm/ADT/SmallVector.h"

namespace fly {

    class Sema;
    class ASTBuilder;
    class DiagnosticsEngine;
    class DiagnosticBuilder;
    class SourceLocation;
    class SymTable;
    class SemaNameSpace;
    class SemaModule;
    class SemaComment;
    class SemaClassType;
    class SemaEnumType;
    class ASTModule;
    class ASTBase;
    class ASTClass;
    class ASTStmt;
    class ASTBlockStmt;
    class ASTArg;
    class ASTVar;
    class ASTCall;
    class ASTRef;
    class ASTExpr;
    class ASTValueExpr;
    class CodeGen;
    class ASTIfStmt;
    class ASTSwitchStmt;
    class ASTLoopStmt;
    class ASTFailStmt;
    class ASTHandleStmt;
    class ASTFunction;
    class ASTRef;
    class ASTComment;
    class ASTImport;
    class ASTTypeRef;
    class ASTVar;
    class ASTLoopInStmt;
    class ASTEnum;
    class ASTVarStmt;
    class SemaGlobalVar;
    class SemaFunction;
    class SemaVar;
    class SemaCall;
    class SemaLocalVar;
    class SemaFunctionBase;
    class SemaType;
    class ASTScope;
    class ASTValue;
    class SemaResult;

    class SemaResolver {

        friend class Sema;
        friend class SemaResolverClass;

        Sema &S;

        // This is the Module NameSpace
        SemaNameSpace *NameSpace;

        SemaModule *Module;

        bool isDefaultNameSpace;

        llvm::SmallVector<ASTBlockStmt *, 8> Bodies;

        SemaResolver(Sema &S, ASTModule *Module);

    public:

        static bool Resolve(Sema &S);

    private:

        void AddSymbols();

        void ResolveImports();

        void ResolveComment(SemaComment *Comment, ASTBase* AST);

        // TODO: remove GlobalVar
        // void ResolveGlobalVars();

        void ResolveFunctions();

        void ResolveTypes();

        void ResolveBodies();

        void ResolveEnumType(SemaEnumType *Sema);

        bool ResolveStmt(ASTStmt *Stmt);

        bool ResolveStmtBlock(ASTBlockStmt *Block);

        bool ResolveStmtIf(ASTIfStmt *IfStmt);

        bool ResolveStmtSwitch(ASTSwitchStmt *SwitchStmt);

        bool ResolveStmtLoop(ASTLoopStmt *LoopStmt);

        bool ResolveStmtLoopIn(ASTLoopInStmt *LoopInStmt);

        bool ResolveStmtVar(ASTVarStmt *VarStmt);

        bool ResolveStmtFail(ASTFailStmt *FailStmt);

        bool ResolveStmtHandle(ASTHandleStmt *HandleStmt);

        bool ResolveValue(ASTValue *AST);

        bool ResolveExpr(ASTStmt *Stmt, ASTExpr *Expr);

        bool ResolveTypeRef(ASTTypeRef *&Type);

        void ResolveFromTopRef(ASTStmt *Stmt, ASTRef *Ref, SemaNameSpace *CurrentNameSpace);

        void ResolveRef(ASTStmt *Stmt, ASTRef *Ref, SemaType *Type, SemaResult *Parent);

        ASTRef* getParentRef(fly::ASTRef* Ref);

        bool ResolveRef(ASTStmt *Stmt, ASTRef *VarRef);

        bool ResolveRef(ASTStmt *Stmt, ASTCall *Call);

        SemaNameSpace *ResolveNameSpace(ASTRef *Ref);

        SemaType *ResolveType(llvm::StringRef Name, SemaNameSpace *CurrentNameSpace);

        void ResolveErrorHandler(ASTStmt* Stmt, SemaCall *Sema);

        SemaCall *ResolveCall(ASTStmt *Stmt, ASTCall *Call, SemaNameSpace *CurrentNameSpace);

        SemaVar *ResolveVar(ASTStmt *Stmt, ASTRef *VarRef);

        llvm::SmallVector<SemaType *, 8> ResolveCallArgTypes(ASTStmt *Stmt, ASTCall *Call);


    };

} // end namespace fly

#endif