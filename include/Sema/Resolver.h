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

#include "SemaClassAttribute.h"
#include "SemaClassMethod.h"
#include "SemaEnumEntry.h"
#include "SemaErrorHandler.h"
#include "llvm/ADT/SmallVector.h"
#include "AST/ASTVisitor.h"

namespace fly {

    class Sema;
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
    class ASTIdentifier;
    class ASTExpr;
    class ASTValue;
    class CodeGen;
    class ASTIfStmt;
    class ASTSwitchStmt;
    class ASTLoopStmt;
    class ASTFailStmt;
    class ASTHandleStmt;
    class ASTFunction;
    class ASTIdentifier;
    class ASTArrayType;
    class ASTComment;
    class ASTImport;
    class ASTType;
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
    class ASTBoolValue;
    class LocalScope;
    class SemaResult;
    class DiagnosticsEngine;
    class DiagnosticBuilder;
    class ASTNameSpace;
    class Registry;


    class Resolver : public ASTVisitor {

        DiagnosticsEngine &Diags;

        Registry &Reg;

        SymbolTable* CurrentScope;

        SemaModule* CurrentModule;

        SemaClassType *CurrentClass;

        SemaEnumType *CurrentEnum;

        // Current Function or Method being Resolved
        SemaFunctionBase *CurrentFunction;

        // This is the NameSpace being currently resolved
        SemaNameSpace *CurrentNameSpace;

        // Current Comment associated to the next Sema Node
        SemaComment* CurrentComment;

        // Current Statement being Resolved
        ASTStmt* CurrentStmt;

    public:

        Resolver(DiagnosticsEngine &Diags, Registry &Reg);

        virtual ~Resolver();

        // Diagnostics
        DiagnosticBuilder Diag(const SourceLocation &Loc, unsigned DiagID) const;
        DiagnosticBuilder Diag(unsigned DiagID) const;

        // Visit Top Level Nodes
        void visit(ASTModule &AST) override;
        void visit(ASTNameSpace &AST) override;
        void visit(ASTImport& AST) override;
        void visit(ASTFunction &AST) override;
        void visit(ASTClass &AST) override;
        void visit(ASTEnum &AST) override;
        void visit(ASTVar &AST) override;
        void visit(ASTComment &AST) override;

        // Visit Types
        void visit(ASTBuiltinType &AST) override;
        void visit(ASTNamedType &AST) override;
        void visit(ASTArrayType &AST) override;

        // Visit Statements
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
        void visit(ASTLoopInStmt &AST) override;
        void visit(ASTVarStmt &AST) override;
        void visit(ASTBlockStmt &AST) override;

        // Visit Expressions
        void visit(ASTIdentifier &AST) override;
        void visit(ASTMember& AST) override;
        void visit(ASTCall &AST) override;
        void visit(ASTUnaryOpExpr &AST) override;
        void visit(ASTBinaryOpExpr &AST) override;
        void visit(ASTTernaryOpExpr &AST) override;
        void visit(ASTCast &AST) override;
        void visit(ASTBoolValue &AST) override;
        void visit(ASTNumberValue &AST) override;
        void visit(ASTStringValue &AST) override;
        void visit(ASTArrayValue &AST) override;
        void visit(ASTStructValue &AST) override;
        void visit(ASTNullValue &AST) override;

        void Resolve();

    private:

        // Scope Management
        void EnterScope();
        void ExitScope();

        // Semantic Resolution Phases
        void ResolveImports(SemaModule *Module);

        void ResolveFunction(SemaFunction *Func);

        void ResolveClassType(SemaClassType *ClassType);

        void ResolveBaseClasses(SemaClassType *DerivedClass);

        bool CanInheritMethod(SemaClassMethod *Method);

        bool CanInheritAttribute(SemaClassAttribute *Attribute);

        void CreateDefaultConstructor();

        void SetDefaultValueInAttributes();

        void ResolveClassAttribute(SemaClassAttribute* Attribute);

        void ResolveClassMethod(SemaClassMethod * Method);

        void ResolveEnumType(SemaEnumType *Enum);

        void ResolveEnumEntry(SemaEnumEntry* Node);

        void ResolveBody(LocalScope &Scope);

        // ------------------------

        SemaType *ResolveTypeRef(ASTType *&Type);

        void ResolveFromTopRef(ASTStmt *Stmt, ASTIdentifier *Ref, SemaNameSpace *CurrentNameSpace);

        void ResolveStaticRef(ASTStmt *Stmt, ASTIdentifier *Ref, SemaType *Type);

        void ResolveInstanceRef(ASTStmt *Stmt, ASTIdentifier *Ref, SemaResult *Parent);

        void ResoveEnumRef(ASTStmt *Stmt, ASTIdentifier *Ref, SemaEnumType *EnumType);

        ASTIdentifier* getParentRef(ASTIdentifier* Ref);

        bool ResolveRef(ASTStmt *Stmt, ASTIdentifier *VarRef);

        bool ResolveRef(ASTStmt *Stmt, ASTCall *Call);

        SemaNameSpace *ResolveNameSpace(ASTIdentifier *Ref);

        SemaType *ResolveType(llvm::StringRef Name, SemaNameSpace *CurrentNameSpace);

        void ResolveErrorHandler(ASTStmt* Stmt, SemaCall *Sema);

        SemaCall *ResolveCall(ASTStmt *Stmt, ASTCall *Call, SemaNameSpace *CurrentNameSpace);

        llvm::SmallVector<SemaType *, 8> ResolveCallArgTypes(ASTStmt *Stmt, ASTCall *Call);

        SemaVar *ResolveVar(ASTStmt *Stmt, ASTIdentifier *VarRef);

    };
} // end namespace fly

#endif