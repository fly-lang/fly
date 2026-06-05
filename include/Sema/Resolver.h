//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/Resolver.h - symbol resolver
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
#include <memory>
#include "llvm/ADT/SmallPtrSet.h"

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
    class ASTTestStmt;
    class ASTCaseStmt;
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
    class ASTTypeParam;
    class ASTComment;
    class ASTImport;
    class ASTType;
    class ASTParam;
    class ASTLocalVar;
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
    class SemaBlockStmt;
    class SemaSmartAlloc;
    class SemaValidator;
    class ASTNameSpace;
    class Registry;
    struct Symbol;


    class Resolver : public ASTVisitor {

        DiagnosticsEngine &Diags;

        Registry &Reg;

    	std::unique_ptr<SemaValidator> Validator;

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

    	// Current resolved expression (set by visit(ASTXxxExpr/Value))
    	SemaExpr *CurrentExpr = nullptr;

    	// Current resolved type (set by visit(ASTXxxType))
    	SemaType *CurrentType = nullptr;

    	// Current SemaBlockStmt being populated
    	SemaBlockStmt *CurrentSemaBlock = nullptr;

    	SemaError *CurrentErrorHandler = nullptr;

    	ASTHandleStmt *CurrentHandleStmt = nullptr;

    	// Loop/switch nesting depth for break/continue validation
    	int LoopDepth = 0;
    	int SwitchDepth = 0;

    	// True while resolving the LHS of a pure '=' assignment (variable is written, not read)
    	bool InAssignLHS = false;

    	// Local vars declared in the current function that have never been read
    	llvm::SmallPtrSet<SemaLocalVar *, 16> UnusedLocalVars;

    	// Non-const params that have never been assigned to in the current function
    	llvm::SmallPtrSet<SemaParam *, 8> UnmodifiedParams;

    	// Temporary storage for resolved call arg expressions
    	SmallVector<SemaExpr *, 8> ResolvedCallArgs;

    	// Counter for generating unique synthetic out-variable names (__out_0, __out_1, ...)
    	unsigned OutVarCounter = 0;

    	// Synthetic ASTLocalVar nodes created for call-site out vars (owned, freed at end)
    	SmallVector<ASTLocalVar *, 16> SyntheticOutVars;

    	// Stable string storage for synthetic parameter names (e.g. "__out_0", "__out_1").
    	// StringRef values pointing into these strings remain valid for the Resolver's lifetime.
    	SmallVector<std::string, 16> SyntheticParamNames;

        // True when compiling in test mode (--test flag)
        bool TestMode = false;

        // True while resolving the body of an inline test {} block
        bool InTestBlock = false;

        // True while resolving a suite test-method body
        bool InSuiteTestMethod = false;

    public:

        Resolver(DiagnosticsEngine &Diags, Registry &Reg, bool TestMode = false);

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
    	void visit(ASTEnumEntry &AST) override;
        void visit(ASTLocalVar &AST) override;
        void visit(ASTParam &AST) override;
        void visit(ASTComment &AST) override;

        // Visit Types
        void visit(ASTBuiltinType &AST) override;
        void visit(ASTNamedType &AST) override;
        void visit(ASTArrayType &AST) override;
        void visit(ASTTypeParam &AST) override;

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
        void visit(ASTTestStmt &AST) override;
        void visit(ASTCaseStmt &AST) override;

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

        void CheckAbstractMethodsImplemented(SemaClassType *ClassType);

        void CheckDiamondAmbiguity(SemaClassType *ClassType);

        void CollectInterfaceDefaultMethods(SemaClassType *Interface,
                                            llvm::StringMap<llvm::SmallVector<SemaClassType *, 2>> &MethodSources);

        // bool CanInheritMethod(SemaClassMethod *Method);
        //
        // bool CanInheritAttribute(SemaClassAttribute *Attribute);

        void CreateDefaultConstructor();

    	SmallVector<SemaType *, 8> ResolveCallArgs(ASTCall *AST);

    	SmallVector<SemaType *, 8> ResolveParams(ASTFunction &AST);

    	SemaSmartAlloc *RegisterSmartAlloc(SemaExpr *Expr);

    	SemaType * PromoteNumberTypes(SemaType * Type1, SemaType * Type2);

    	void PromoteTypes(ASTBinary &AST, SemaExpr *Left, SemaExpr *Right);

    	SemaExpr * ResolveMemberSymbol(ASTMember &AST, SymbolTable *Symbols, SemaKind ExpectedKind, SemaVar *ParentVar = nullptr);

    };
} // end namespace fly

#endif

