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

        ASTRef *ResolveCall(ASTStmt *Stmt, ASTCall *Call, llvm::SmallVector<SymNameSpace *, 4> &NameSpaces);

        llvm::SmallVector<SymType *, 8> ResolveCallArgTypes(ASTStmt *Stmt, ASTCall *Call);

        ASTRef *ResolveRef(ASTStmt *Stmt, ASTRef *Ref);

        ASTRef *ResolveRef(ASTStmt *Stmt, ASTRef *Ref, llvm::SmallVector<SymNameSpace *, 4> &NameSpaces);

        ASTRef *ResolveRef(ASTStmt *Stmt, SymType *Type, ASTRef *Ref);

        ASTRef *ResolveRef(ASTStmt *Stmt, SymVar *Var, ASTRef *Ref);

        SymType *FindType(llvm::StringRef Name, llvm::SmallVector<SymNameSpace *, 4> &NameSpaces) const;
    };

} // end namespace fly

#endif