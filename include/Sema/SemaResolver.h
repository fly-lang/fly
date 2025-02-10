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

#include "SymBuilder.h"

namespace llvm {
    class StringRef;
}

namespace fly {

    class Sema;
    class ASTBuilder;
    class DiagnosticsEngine;
    class DiagnosticBuilder;
    class SourceLocation;
    class SymTable;
    class SymNameSpace;
    class SymModule;
    class SymComment;
    class SymClass;
    class SymEnum;
    class ASTModule;
    class ASTBase;
    class ASTClass;
    class ASTStmt;
    class ASTBlockStmt;
    class ASTArg;
    class ASTVar;
    class ASTCall;
    class ASTVarRef;
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
    class SymGlobalVar;
    class SymFunction;
    class SymVar;
    class SymLocalVar;
    class SymFunctionBase;
    class ASTScope;

    class SemaResolver {
        friend class Sema;

        Sema &S;

        // This is the Default NameSpace
        SymNameSpace *Default;

        // This is the Module NameSpace
        SymNameSpace *NameSpace;

        SymModule *Module;

        SemaResolver(Sema &S, ASTModule *Module);

    public:

        static bool Resolve(Sema &S);

    private:

        void AddSymbols();

        void AddImport(ASTImport *AST);

        void AddGlobalVar(ASTVar *AST, SymComment *Comment);

        void AddFunction(ASTFunction *AST, SymComment *Comment);

        void AddClass(ASTClass *AST, SymComment *Comment);

        void AddEnum(ASTEnum *AST, SymComment *Comment);

        void ResolveImports();

        void ResolveComment(SymComment *Comment, ASTBase* AST);

        void ResolveGlobalVars();

        void ResolveFunctions();

        void ResolveClasses();

        void ResolveEnums();

        bool ResolveTypeRef(ASTTypeRef *&Type);

        bool ResolveStmt(ASTStmt *Stmt);

        bool ResolveStmtBlock(ASTBlockStmt *Block);

        bool ResolveStmtIf(ASTIfStmt *IfStmt);

        bool ResolveStmtSwitch(ASTSwitchStmt *SwitchStmt);

        bool ResolveStmtLoop(ASTLoopStmt *LoopStmt);

        bool ResolveStmtLoopIn(ASTLoopInStmt *LoopInStmt);

        bool ResolveStmtVar(ASTVarStmt *VarStmt);

        bool ResolveStmtFail(ASTFailStmt *FailStmt);

        bool ResolveStmtHandle(ASTHandleStmt *HandleStmt);

        bool ResolveExpr(ASTStmt *Stmt, ASTExpr *Expr, SymType *Type = nullptr);

        // bool ResolveRef(SymNameSpace *NameSpace, ASTStmt *Stmt, ASTRef *Ref);
        //
        // bool ResolveRef(ASTIdentity *Identity, ASTStmt *Stmt, ASTRef *Identifier);

        // bool ResolveRef(ASTStmt *Stmt, ASTRef *Ref);

        // bool ResolveGlobalVarRef(SymNameSpace *NameSpace, ASTStmt *Stmt, ASTVarRef *VarRef);

        // bool ResolveVarRef(ASTStmt *Stmt, ASTVarRef *VarRef);
        //
        // bool ResolveStaticVarRef(SymIdentity *Identity, ASTStmt *Stmt, ASTVarRef *VarRef);
        //
        // bool ResolveFunctionCall(SymNameSpace *NameSpace, ASTStmt *Stmt, ASTCall *Call);
        //
        // bool ResolveStaticCall(ASTIdentity *Identity, ASTStmt *Stmt, ASTCall *Call);

        ASTRef *ResolveCall(ASTStmt *Stmt, ASTCall *Call, SymNameSpace *NameSpaces...);

        ASTRef *ResolveRef(ASTStmt *Stmt, ASTRef *Ref);

        ASTRef *ResolveRef(ASTStmt *Stmt, ASTRef *Ref, SymNameSpace *NameSpaces...);

        ASTRef *ResolveRef(SymType *Type, ASTRef *Ref, SymNameSpace *NameSpaces...);

        ASTRef *ResolveRef(SymVar *Var, ASTRef *Ref, SymNameSpace *NameSpaces...);

        SymType *FindType(llvm::StringRef Name, SymNameSpace *NameSpaces...);

        SymVar *FindVar(ASTStmt *Stmt, ASTRef *Ref, SymNameSpace *NameSpaces...) const;

        SymLocalVar *FindLocalVar(ASTStmt *Stmt, ASTRef *Ref) const;
    };

} // end namespace fly

#endif