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

#include "AST/ASTVisitor.h"
#include "SemaClassAttribute.h"
#include "SemaClassMethod.h"
#include "SemaEnumEntry.h"

#include <AST/ASTMember.h>

namespace fly {

    class SemaContext;
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
    class ASTArrayType;
    class ASTComment;
    class ASTImport;
    class ASTType;
    class ASTParam;
    class ASTLoopInStmt;
    class ASTEnum;
    class SemaFunction;
    class SemaVar;
    class SemaCall;
    class SemaLocalVar;
    class SemaFunctionBase;
    class SemaType;
    class ASTModifier;
    class ASTBoolValue;
    class LocalScope;
    class SemaExpr;
    class SemaValidator;
    class ASTNameSpace;
    class Registry;
    class Symbol;


    class Resolver : public ASTVisitor {

        DiagnosticsEngine &Diags;

        Registry &Reg;

    	SemaValidator *Validator;

        SymbolTable* CurrentScope;

        SemaModule* CurrentModule;

        SemaClassType *CurrentClass = nullptr;

        SemaEnumType *CurrentEnum =	nullptr;

        // Current Function or Method being Resolved
        SemaFunctionBase *CurrentFunction = nullptr;

        // This is the NameSpace being currently resolved
        SemaNameSpace *CurrentNameSpace = nullptr;

        // Current Comment associated to the next Sema Node
        SemaComment* CurrentComment = nullptr;

        // Current Statement being Resolved
        ASTStmt* CurrentStmt = nullptr;

    	SemaExpr *CurrentExpr = nullptr;

    	SemaError *CurrentErrorHandler = nullptr;

    	ASTHandleStmt *CurrentHandleStmt = nullptr;

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
        void visit(ASTAttribute &AST) override;
        void visit(ASTMethod &AST) override;
        void visit(ASTEnum &AST) override;
        void visit(ASTLocalVar &AST) override;
        void visit(ASTParam &AST) override;
        void visit(ASTComment &AST) override;

        // Visit Types
        void visit(ASTBuiltinType &AST) override;
        void visit(ASTNamedType &AST) override;
        void visit(ASTArrayType &AST) override;

        // Visit Statements
        void visit(ASTExprStmt &AST) override;
        void visit(ASTDeclStmt &AST) override;
        void visit(ASTFailStmt &AST) override;
        void visit(ASTHandleStmt &AST) override;
        void visit(ASTReturnStmt &AST) override;
    	void visit(ASTDeleteStmt &AST) override;
    	void visit(ASTBreakStmt &AST) override;
    	void visit(ASTContinueStmt &AST) override;
    	void visit(ASTBlockStmt &AST) override;
        void visit(ASTRuleStmt &AST) override;
        void visit(ASTIfStmt &AST) override;
        void visit(ASTSwitchStmt &AST) override;
        void visit(ASTLoopStmt &AST) override;
        void visit(ASTLoopInStmt &AST) override;

        // Visit Expressions
        void visit(ASTIdentifier &AST) override;
        void visit(ASTMember& AST) override;
        void visit(ASTCall &AST) override;
        void visit(ASTUnary &AST) override;
		void visit(ASTBinary &AST) override;
        void visit(ASTTernary &AST) override;
        void visit(ASTCast &AST) override;
        void visit(ASTBoolValue &AST) override;
        void visit(ASTNumberValue &AST) override;
        void visit(ASTStringValue &AST) override;
        void visit(ASTArrayValue &AST) override;
        void visit(ASTStructValue &AST) override;
        void visit(ASTNullValue &AST) override;
        void visit(ASTUnsetValue &AST) override;
    	void visit(ASTEnumEntry &AST) override;

        // Main Resolve Function
        void Resolve();

    private:

        // Scope Management
        void EnterScope();
        void ExitScope();
        void ResetCurrents();
        void addSymbol(Symbol *Sym);

        // Semantic Resolution Phases
        void ResolveImports(SemaModule *Module);

        void ResolveFunction(SemaFunction *Sema);

        void ResolveClassType(SemaClassType *ClassType);

        void ResolveBaseClasses(SemaClassType *DerivedClass);

        bool CanInheritMethod(SemaClassMethod *Method);

        bool CanInheritAttribute(SemaClassAttribute *Attribute);

        void CreateDefaultConstructor();

        void ResolveEnumType(SemaEnumType *Enum);

    	SmallVector<SemaType *, 8> ResolveCallArgs(ASTCall *AST);

    	SmallVector<SemaType *, 8> ResolveParams(ASTFunction &AST);

    	SemaType * PromoteNumberTypes(SemaType * Type1, SemaType * Type2);

    	void PromoteTypes(ASTBinary &AST);

    	SemaExpr * ResolveMemberSymbol(ASTMember &AST, SymbolTable *Symbols, SemaKind ExpectedKind, SemaVar *ParentVar = nullptr);

    };
} // end namespace fly

#endif

