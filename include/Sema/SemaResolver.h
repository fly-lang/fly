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
    class ASTLocalVar;
    class ASTArg;
    class ASTVar;
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
    class ASTFunction;
    class ASTIdentifier;
    class ASTComment;
    class ASTImport;
    class ASTRefType;
    class ASTVar;
    class ASTIdentity;
    class ASTLoopInStmt;
    class ASTEnum;
    class ASTAssignmentStmt;
    class SymGlobalVar;
    class SymFunction;
    class SymIdentity;
class ASTScope;

    class SemaResolver {

        friend class Sema;

        Sema &S;

        // This is the Module NameSpace
        SymNameSpace *NameSpace;

        SymModule *Module;

        SemaResolver(Sema &S, ASTModule *Module);

    public:

        static bool Resolve(Sema &S);

    private:

        void AddSymbols();

        void AddNameSpace();

        void AddImport(ASTImport *AST);

        void AddGlobalVar(ASTVar *AST, SymComment *Comment);

        void AddFunction(ASTFunction *AST, SymComment *Comment);

        void AddClass(ASTClass *AST, SymComment *Comment);

        void AddEnum(ASTEnum *Enum, SymComment *Comment);

        void ResolveImports();

        void ResolveComment(SymComment *Comment, ASTBase* AST);

        void ResolveGlobalVars();

        void ResolveFunctions();

        void ResolveClasses();

        void ResolveEnums();

        bool ResolveType(ASTType *Type);

        bool ResolveStmt(ASTStmt *Stmt);

        bool ResolveStmtBlock(ASTBlockStmt *Block);

        bool ResolveStmtIf(ASTIfStmt *IfStmt);

        bool ResolveStmtSwitch(ASTSwitchStmt *SwitchStmt);

        bool ResolveStmtLoop(ASTLoopStmt *LoopStmt);

        bool ResolveStmtLoopIn(ASTLoopInStmt *LoopInStmt);

        bool ResolveStmtVar(ASTAssignmentStmt *VarStmt);

        bool ResolveStmtFail(ASTFailStmt *FailStmt);

        bool ResolveStmtHandle(ASTHandleStmt *HandleStmt);

        bool ResolveIdentityType(ASTRefType *IdentityType);

        bool ResolveIdentifier(SymNameSpace *NameSpace, ASTStmt *Stmt, ASTIdentifier *Identifier);

        bool ResolveIdentifier(ASTIdentity *Identity, ASTStmt *Stmt, ASTIdentifier *Identifier);

        bool ResolveIdentifier(ASTStmt *Stmt, ASTIdentifier *Identifier);

        bool ResolveGlobalVarRef(SymNameSpace *NameSpace, ASTStmt *Stmt, ASTVarRef *VarRef);

        bool ResolveStaticVarRef(ASTIdentity *Identity, ASTStmt *Stmt,ASTVarRef *VarRef);

        bool ResolveVarRef(ASTStmt *Stmt, ASTVarRef *VarRef);

        bool ResolveFunctionCall(SymNameSpace *NameSpace, ASTStmt *Stmt, ASTCall *Call);

        bool ResolveStaticCall(ASTIdentity *Identity, ASTStmt *Stmt, ASTCall *Call);

        bool ResolveCall(ASTStmt *Stmt, ASTCall *Call);

        bool ResolveCallArgs(ASTStmt *Stmt, ASTCall *Call);

        bool ResolveExpr(ASTStmt *Stmt, ASTExpr *Expr, ASTType *Type = nullptr);

        SymNameSpace *FindNameSpace(ASTIdentifier *Identifier, ASTIdentifier *&Current) const;

        SymGlobalVar *FindGlobalVar(llvm::StringRef Name, SymNameSpace *NameSpace) const;

        SymIdentity *FindIdentity(llvm::StringRef Name, SymNameSpace *NameSpace) const;

        SymFunction *FindFunction(ASTCall *Call, SymNameSpace *NameSpace) const;

        SymClassMethod *FindClassMethod(ASTCall *Call, ASTClass *Class) const;

        template <typename T>
        T *FindFunction(ASTCall *Call, llvm::SmallVector<T *, 8> Functions) const;

        template <typename T>
        T *FindFunction(ASTCall *Call, llvm::StringMap<std::map<uint64_t, llvm::SmallVector<T *, 4>>> Functions) const;

        ASTVar *FindLocalVar(ASTStmt *Stmt, llvm::StringRef Name) const;
    };

}  // end namespace fly

#endif