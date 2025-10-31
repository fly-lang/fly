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

#include <unordered_map>

#include "SemaClassMethod.h"
#include "SemaErrorHandler.h"
#include "llvm/ADT/SmallVector.h"
#include "AST/ASTVisitor.h"

namespace fly {

    class Sema;
    class SemaResolverClass;
    class SemaBuilder;
    class ASTBuilder;
    class DiagnosticsEngine;
    class DiagnosticBuilder;
    class SourceLocation;
    class SymbolTable;
    class SemaNameSpace;
    class SemaModule;
    class SemaComment;
    class SemaClassType;
    class SemaEnumType;
    class ASTModule;
    class ASTNode;
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
    class ASTModifier;
    class ASTValue;
    class SemaResult;
    class DiagnosticsEngine;
    class DiagnosticBuilder;
    class ASTNameSpace;
    class Registry;


    class Resolver : public ASTVisitor {

        DiagnosticsEngine &Diags;

        Registry &Reg;

        SymbolTable *BuiltinScope;

        SymbolTable* CurrentScope;

        SemaModule* CurrentModule;

        // This is the Module NameSpace
        SemaNameSpace *CurrentNameSpace;

        SemaComment* CurrentComment;

    public:

        Resolver(DiagnosticsEngine &Diags, Registry &Reg);

        virtual ~Resolver();

        SymbolTable * CreateBuiltinScope();

        DiagnosticBuilder Diag(const SourceLocation &Loc, unsigned DiagID) const;

        DiagnosticBuilder Diag(unsigned DiagID) const;

        void visit(ASTModule &AST) override;
        void visit(ASTNameSpace &AST) override;
        void visit(ASTImport& AST) override;
        void visit(ASTFunction &AST) override;
        void visit(ASTClass &AST) override;
        void visit(ASTEnum &AST) override;
        void visit(ASTComment &AST) override;
        void visit(ASTValue &AST) override;
        void visit(ASTVar &AST) override;

        void visit(ASTRef &AST) override;
        void visit(ASTCall &AST) override;
        void visit(ASTNameSpaceRef &AST) override;
        void visit(ASTTypeRef &AST) override;

        void visit(ASTBreakStmt &AST) override;
        void visit(ASTContinueStmt &AST) override;
        void visit(ASTDeleteStmt &AST) override;
        void visit(ASTExprStmt &AST) override;
        void visit(ASTFailStmt &AST) override;
        void visit(ASTHandleStmt &AST) override;
        void visit(ASTReturnStmt &AST) override;
        void visit(ASTRuleStmt &AST) override;
        void visit(ASTIfStmt &AST) override;
        void visit(ASTSwitchStmt &AST) override;
        void visit(ASTLoopStmt &AST) override;
        void visit(ASTVarStmt &AST) override;
        void visit(ASTBlockStmt &AST) override;

        void visit(ASTValueExpr &AST) override;
        void visit(ASTVarRefExpr &AST) override;
        void visit(ASTCallExpr &AST) override;
        void visit(ASTOpExpr &AST) override;
        void visit(ASTCastExpr &AST) override;

        void Resolve();

    private:

        // Scope Management
        void EnterScope();
        void ExitScope();

        // Semantic Resolution Phases
        void ResolveImports(SemaModule *Module);

        void ResolveFunctions(SemaModule *Module);

        void ResolveTypes(SemaModule *Module);

        void ResolveClassTypes();

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

        void ResolveStaticRef(ASTStmt *Stmt, ASTRef *Ref, SemaType *Type);

        void ResolveInstanceRef(ASTStmt *Stmt, ASTRef *Ref, SemaResult *Parent);

        void ResoveEnumRef(ASTStmt *Stmt, ASTRef *Ref, SemaEnumType *EnumType);

        ASTRef* getParentRef(ASTRef* Ref);

        bool ResolveRef(ASTStmt *Stmt, ASTRef *VarRef);

        bool ResolveRef(ASTStmt *Stmt, ASTCall *Call);

        SemaNameSpace *ResolveNameSpace(ASTRef *Ref);

        SemaType *ResolveType(llvm::StringRef Name, SemaNameSpace *CurrentNameSpace);

        void ResolveErrorHandler(ASTStmt* Stmt, SemaCall *Sema);

        SemaCall *ResolveCall(ASTStmt *Stmt, ASTCall *Call, SemaNameSpace *CurrentNameSpace);

        llvm::SmallVector<SemaType *, 8> ResolveCallArgTypes(ASTStmt *Stmt, ASTCall *Call);

        SemaVar *ResolveVar(ASTStmt *Stmt, ASTRef *VarRef);

    };
} // end namespace fly

#endif