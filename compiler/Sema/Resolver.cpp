//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/Resolver.cpp - symbol resolver
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/Resolver.h"

#include "AST/ASTArg.h"
#include "AST/ASTAttribute.h"
#include "AST/ASTBinary.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTBreakStmt.h"
#include "AST/ASTBuilder.h"
#include "AST/ASTCall.h"
#include "AST/ASTClass.h"
#include "AST/ASTContinueStmt.h"
#include "AST/ASTDeleteStmt.h"
#include "AST/ASTEnum.h"
#include "AST/ASTEnumEntry.h"
#include "AST/ASTExpr.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTFailStmt.h"
#include "AST/ASTFunction.h"
#include "AST/ASTHandleStmt.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTIfStmt.h"
#include "AST/ASTImport.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTLoopInStmt.h"
#include "AST/ASTLoopStmt.h"
#include "AST/ASTMethod.h"
#include "AST/ASTModule.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTReturnStmt.h"
#include "AST/ASTSwitchStmt.h"
#include "AST/ASTTernary.h"
#include "AST/ASTType.h"
#include "AST/ASTUnary.h"
#include "AST/ASTValue.h"
#include "Basic/Debug.h"
#include "Basic/Diagnostic.h"
#include "Sema/SemaBuilder.h"
#include "Sema/SemaCast.h"
#include "Sema/SemaBuiltin.h"
#include "Sema/SemaClassAttribute.h"
#include "Sema/SemaClassInstance.h"
#include "Sema/SemaClassMethod.h"
#include "Sema/SemaClassType.h"
#include "Sema/SemaEnumType.h"
#include "Sema/SemaImport.h"
#include "Sema/SemaMember.h"
#include "Sema/SemaModule.h"
#include "Sema/SemaNameSpace.h"
#include "Sema/SemaBinary.h"
#include "Sema/SemaBlockStmt.h"
#include "Sema/SemaCall.h"
#include "Sema/SemaSmartAlloc.h"
#include "Sema/SemaStringAlloc.h"
#include "Sema/SemaValidator.h"
#include "Sema/SymbolTable.h"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Support/Signals.h"

#include <functional>

#include <AST/ASTCast.h>
#include <AST/ASTDeclStmt.h>
#include <AST/ASTParam.h>
#include <Sema/Helper.h>
#include <Sema/Registry.h>
#include <Sema/SemaCall.h>
#include <Sema/SemaEnumEntry.h>
#include <Sema/SemaEnumList.h>
#include <Sema/SemaFunction.h>
#include <Sema/SemaLocalVar.h>
#include <Sema/SemaParam.h>
#include <Sema/SemaBinary.h>
#include <Sema/SemaUnary.h>
#include <Sema/SemaTernary.h>
#include <Sema/SemaValue.h>
#include <Sema/SemaBlockStmt.h>
#include <Sema/SemaDeclStmt.h>
#include <Sema/SemaExprStmt.h>
#include <Sema/SemaReturnStmt.h>
#include <Sema/SemaIfStmt.h>
#include <Sema/SemaSwitchStmt.h>
#include <Sema/SemaLoopStmt.h>
#include <Sema/SemaLoopInStmt.h>
#include <Sema/SemaDeleteStmt.h>
#include <Sema/SemaBreakStmt.h>
#include <Sema/SemaContinueStmt.h>
#include <Sema/SemaFailStmt.h>
#include <Sema/SemaHandleStmt.h>
#include <llvm/Transforms/IPO/FunctionImport.h>

using namespace fly;

Resolver::Resolver(DiagnosticsEngine &Diags, Registry &Reg) : Diags(Diags),
	CurrentModule(nullptr),
	Reg(Reg),
	CurrentNameSpace(Reg.getDefaultNameSpace()),
    CurrentScope(Reg.getDefaultNameSpace()->getSymbols()),
	Validator(new SemaValidator(Diags)){
}

/**
 * Write Diagnostics
 * @param Loc
 * @param DiagID
 * @return
 */
DiagnosticBuilder Resolver::Diag(const SourceLocation &Loc, unsigned DiagID) const {
	return Diags.Report(Loc, DiagID);
}

DiagnosticBuilder Resolver::Diag(unsigned DiagID) const {
	if (DebugLog && DiagID == diag::err_invalid_behavior)
		llvm::sys::PrintStackTrace(llvm::errs());
	return Diags.Report(DiagID);
}

void Resolver::visit(ASTModule &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTModule)");
	if (!AST.isVisited()) {
		AST.setVisited(true);

		EnterScope();

		CurrentModule = new SemaModule(AST, CurrentScope);
		Reg.addModule(CurrentModule);

		// Process namespace declaration first; ASTModule stores it separately from Nodes
		if (AST.getNameSpace() != nullptr) {
			AST.getNameSpace()->accept(*this);
		} else {
			CurrentNameSpace = Reg.getDefaultNameSpace();
		}

		for (auto Node : AST.getNodes()) {
			Node->accept(*this);
		}

		ExitScope();
	}
}

void Resolver::visit(ASTNameSpace &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTNameSpace)");
	if (!AST.isVisited()) {
		AST.setVisited(true);

		// Build the CurrentNameSpace
		SemaNameSpace *NameSpace = Reg.getOrCreateNameSpace(AST.getNames());

		// Set Symbol Table
		CurrentModule->setNameSpace(NameSpace);
		CurrentNameSpace = NameSpace;
	}
}

void Resolver::visit(ASTImport &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTImport)");
	if (!AST.isVisited()) {
		AST.setVisited(true);

		Validator->CheckImport(AST);

		// Add import in Module
		SemaBuilder::CreateImport(*CurrentModule, AST);
	}
}

void Resolver::visit(ASTFunction &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTFunction)");
	ResetCurrents();

	// Enter Function Scope
	EnterScope();

	// Create Sema Function
	CurrentFunction = SemaBuilder::CreateFunction(*CurrentModule, CurrentScope, AST);

	// 'main' is the entry point: it takes no parameters
	if (AST.getName() == "main") {
		const auto &MainParams = AST.getParams();
		if (!MainParams.empty()) {
			Diag(AST.getLocation(), diag::err_sema_main_with_params);
		}
	}

	// Add to Symbol Table of Parent Scope (Module or Class)
	Symbol *Sym = new Symbol(AST.getName(), SymbolKind::FUNCTION, CurrentFunction);

	// Exit Function Scope
	ExitScope();

	// Build flattened namespace name (e.g. "fly_string" for fly.string)
	ASTNameSpace *NS = CurrentModule->getAST().getNameSpace();
	if (NS && !NS->getNames().empty()) {
		std::string NSName;
		for (auto *N : NS->getNames()) {
			if (!NSName.empty()) NSName += "_";
			NSName += N->getName();
		}
		CurrentFunction->setNamespaceName(std::move(NSName));
	}

	// For external functions (no body), resolve params and return type immediately
	// so that callers can inspect them without waiting for the body resolution pass.
	if (AST.getBody() == nullptr) {
		SymbolTable *SavedScope = CurrentScope;
		CurrentScope = CurrentFunction->getSymbols();
		EnterScope();
		for (auto Param : AST.getParams()) {
			Param->accept(*this);
			SemaParam *ResolvedParam = Param->getSymbol()->getRefAs<SemaParam>();
			CurrentFunction->addParam(ResolvedParam);
		}
		// Resolve explicit return type if provided in the header
		if (AST.getReturnType() != nullptr) {
			AST.getReturnType()->accept(*this);
			CurrentFunction->setReturnType(CurrentType);
		}
		ExitScope();
		CurrentScope = SavedScope;
	}

	// Add Symbol to the module scope (for local / unqualified calls)
	addSymbol(Sym);

	// Also register in the namespace scope so qualified calls (fly.strings.len())
	// can find the function when CurrentScope is switched to the namespace table.
	if (CurrentNameSpace && CurrentNameSpace != Reg.getDefaultNameSpace()) {
		CurrentNameSpace->getSymbols()->insert(Sym);
	}
}

void Resolver::visit(ASTClass &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTClass)");
	if (!AST.isVisited()) {
		AST.setVisited(true);

		// Reset
		ResetCurrents();

		// Enter Class Scope
		EnterScope();

		// Create Sema Class
		CurrentClass = SemaBuilder::CreateClass(*CurrentModule, CurrentScope, AST);

		// Create Symbol
		Symbol *Sym = new Symbol(AST.getName(), SymbolKind::CLASS, CurrentClass);

		// Note: Nodes will be added in the next Resolve Steps

		// Exit Class Scope
		ExitScope();

		// Add Class Symbol to the current scope
		addSymbol(Sym);

		// Also register in the namespace scope so qualified type lookups
		// (e.g. fly.os.fly_file, fly.os.Reader) can find the type when
		// CurrentScope is switched to the namespace symbol table.
		if (CurrentNameSpace && CurrentNameSpace != Reg.getDefaultNameSpace()) {
			CurrentNameSpace->getSymbols()->insert(Sym);
		}
	}
}

void Resolver::visit(ASTAttribute &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTAttribute)");

	// Forbid redeclaration of inherited fields — two slots with the same name
	// are never intentional and always cause confusion.
	SemaClassAttribute *ExistingAttr = CurrentClass->LookupAttribute(AST.getName());
	if (ExistingAttr) {
		if (&ExistingAttr->getClass() != CurrentClass) {
			// Inherited field: error only if visibility > private.
			// A private field is invisible to subclasses so redeclaring it is allowed;
			// base class methods can still reference the original private field via
			// their own class's symbol table, while the subclass gets an independent slot.
			if (ExistingAttr->getVisibility() != SemaVisibilityKind::PRIVATE) {
				Diag(AST.getLocation(), diag::err_sema_field_hides_inherited)
					<< AST.getName() << CurrentClass->getName() << ExistingAttr->getClass().getName();
				return;
			}
			// Private inherited field: allow the subclass to declare its own slot.
		} else {
			// Same class: plain redefinition.
			Diag(AST.getLocation(), diag::err_sema_var_redefinition) << AST.getName();
			return;
		}
	}

	// Resolve Type
	AST.getType()->accept(*this);
	SemaType * Type = CurrentType;

	// Create Class Attribute
	SemaClassAttribute *Sema = SemaBuilder::CreateClassAttribute(*CurrentClass, AST, Type);
	CurrentClass->addAttribute(Sema); // Function Local var to be allocated

	// Set Expr or Default Value
	if (AST.getExpr()) {
		AST.getExpr()->accept(*this);
		// Promote the init expr's numeric type to the declared attribute type so that
		// e.g. "int a = 3" generates i32 3 and not i16 3 (which getBitsNeeded infers).
		if (CurrentExpr && Type->isNumber() &&
		    CurrentExpr->getType() && CurrentExpr->getType()->isNumber()) {
			CurrentExpr->setType(PromoteNumberTypes(Type, CurrentExpr->getType()));
		}
		Sema->InitExpr = CurrentExpr;
	}

	// Create the Symbol and add to Symbol Table
	Symbol *Sym = new Symbol(AST.getName(), SymbolKind::ATTRIBUTE, Sema);
	AST.setSymbol(Sym);
	addSymbol(Sym);
}

void Resolver::visit(ASTMethod &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTMethod)");

	// Enter Method Scope
	EnterScope();

	// Methods are implicitly void - no return type to resolve
	SmallVector<SemaType *, 8> Types = ResolveParams(AST);

	// Find Method duplication (strict: exact type match, no numeric promotion)
	if (Reg.LookupFunctionExact(AST.getName(), Types, CurrentClass->getSymbols())) {
		Diag(AST.getLocation(), diag::err_sema_var_redefinition) << AST.getName();
		return;
	}

	// Abstract modifier + body is a conflict
	for (auto *Mod : AST.getModifiers()) {
		if (Mod->getModifierKind() == ASTModifierKind::MOD_ABSTRACT && AST.getBody() != nullptr) {
			Diag(AST.getLocation(), diag::err_sema_abstract_method_has_body) << AST.getName();
			ExitScope();
			return;
		}
	}

	// Create Class Method
	SemaClassMethod *Sema = SemaBuilder::CreateClassMethod(CurrentClass, AST, CurrentScope);

	// A method without a body is abstract; the class must be abstract or an interface
	if (Sema->isAbstract()) {
		if (!CurrentClass->isAbstract() && CurrentClass->getClassKind() != SemaClassKind::INTERFACE) {
			Diag(AST.getLocation(), diag::err_sema_abstract_method_requires_abstract_class)
				<< CurrentClass->getName() << AST.getName();
			return;
		}
	}

	// Check that this method does not override a final method in a base class
	if (!Sema->isConstructor()) {
		for (auto *Base : CurrentClass->getBaseClasses()) {
			SemaClassMethod *BaseMethod = Base->LookupMethod(AST.getName());
			if (BaseMethod && BaseMethod->isFinal()) {
				Diag(AST.getLocation(), diag::err_sema_final_method_overridden)
					<< AST.getName() << Base->getName();
				return;
			}
			// Track the overridden method
			if (BaseMethod) {
				Sema->Overridden = BaseMethod;
			}
		}
	}

	CurrentClass->addMethod(Sema); // Function Local var to be allocated

	// Check if default constructor
	if (AST.getName() == CurrentClass->getName() && AST.getParams().empty()) {
		CurrentClass->setDefaultConstructor(Sema);
	}

	// Enter Parameters Scope
	EnterScope();

	// Resolve Parameters Types
	for (auto Param : AST.getParams()) {

		// resolve parameter type
		Param->accept(*this);
		SemaParam *SemaP = Param->getSymbol()->getRefAs<SemaParam>();
		Sema->addParam(SemaP);
		addSymbol(Param->getSymbol());
	}

	// Exit Parameters Scope
	ExitScope();

	// Add to Body list for resolve in the next step (abstract methods have no body to resolve)
	if (!Sema->isAbstract()) {
		Reg.addBody(Sema);
	}

	// Create the Symbol and add to Symbol Table of the parent scop
	Symbol *Sym = new Symbol(AST.getName(), SymbolKind::FUNCTION, Sema);

	// Exit Method Scope
	ExitScope();

	// Add Symbol to the current scope
	addSymbol(Sym);
}

void Resolver::visit(ASTEnum &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTEnum)");
	ResetCurrents();

	// Enums cannot extend classes, structs, or interfaces
	if (!AST.getBases().empty()) {
		Diag(AST.getLocation(), diag::err_sema_enum_extends_not_allowed) << AST.getName();
	}

	// Enter Enum Scope
	EnterScope();

	// Create Sema Enum
	CurrentEnum = SemaBuilder::CreateEnum(*CurrentModule, CurrentScope, AST);

	// Create Symbol
	Symbol *Sym = new Symbol(AST.getName(), SymbolKind::ENUM, CurrentEnum);

	// Add enum entries
	for (auto Def : AST.getNodes()) {
		Def->accept(*this);
	}

	// Exit Class Scope
	ExitScope();

	// Add Symbol to the current scope
	addSymbol(Sym);
}

void Resolver::visit(ASTEnumEntry &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTEnumEntry)");

	// Find Var duplication in the current scope
	SemaEnumEntry *ExistingEnum = CurrentEnum->LookupEntry(AST.getName());
	if (ExistingEnum) {
		Diag(AST.getLocation(), diag::err_sema_var_redefinition) << AST.getName();
	}

	SemaEnumEntry *Sema = SemaBuilder::CreateEnumEntry(CurrentEnum, AST);
	CurrentEnum->addEntry(Sema);

	// Create Symbol
	Symbol *Sym = new Symbol(AST.getName(), SymbolKind::ENUM, Sema);

	// Add Symbol to the current scope
	addSymbol(Sym);
}

void Resolver::visit(ASTLocalVar &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTLocalVar)");
	if (!AST.isVisited()) {
		AST.setVisited(true);

		// Resolve Type
		AST.getType()->accept(*this);
		SemaType *Type = CurrentType;

		// Create Sema Local Var
		SemaLocalVar *Sema = SemaBuilder::CreateLocalVar(AST, Type);

		// Track as potentially unused until a read is observed
		UnusedLocalVars.insert(Sema);

		// Add LocalVar to the Function Base LocalVars
		CurrentFunction->addLocalVar(Sema);

		// Find Var duplication in the current scope
		SmallVector<Symbol *, 8> *Symbols = CurrentScope->lookup(AST.getName());
		if (Symbols) {
			Diag(AST.getLocation(), diag::err_sema_var_redefinition) << AST.getName();
		}

		// Add to Symbol Table
		Symbol *Sym = new Symbol(AST.getName(), SymbolKind::LOCAL_VAR, Sema);
		AST.setSymbol(Sym);

		// Add Symbol to the current scope
		addSymbol(Sym);
	}
}

void Resolver::visit(ASTParam &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTParam)");
	if (!AST.isVisited()) {
		AST.setVisited(true);
		// Resolve Type
		AST.getType()->accept(*this);
		SemaType *Type = CurrentType;

		// Create Sema Param
		SemaParam * Sema = SemaBuilder::CreateParam(AST, Type);

		// Create Symbol and set on AST
		Symbol *Sym = new Symbol(AST.getName(), SymbolKind::PARAM, Sema);
		AST.setSymbol(Sym);
	}
}

void Resolver::visit(ASTComment &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTComment)");
	CurrentComment = SemaBuilder::CreateComment(AST);
}

void Resolver::visit(ASTBuiltinType &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTBuiltinType)");
	if (!AST.isVisited()) {
		AST.setVisited(true);
	}

	SemaType *Sema = nullptr;
	switch (AST.getBuiltinKind()) {

		case ASTBuiltinTypeKind::TYPE_VOID:
			Sema = SemaBuiltin::getVoidType();
			break;
		case ASTBuiltinTypeKind::TYPE_BOOL:
			Sema = SemaBuiltin::getBoolType();
			break;
		case ASTBuiltinTypeKind::TYPE_BYTE:
			Sema = SemaBuiltin::getByteType();
			break;
		case ASTBuiltinTypeKind::TYPE_SHORT:
			Sema = SemaBuiltin::getShortType();
			break;
		case ASTBuiltinTypeKind::TYPE_INT:
			Sema = SemaBuiltin::getIntType();
			break;
		case ASTBuiltinTypeKind::TYPE_LONG:
			Sema = SemaBuiltin::getLongType();
			break;
		case ASTBuiltinTypeKind::TYPE_USHORT:
			Sema = SemaBuiltin::getUShortType();
			break;
		case ASTBuiltinTypeKind::TYPE_UINT:
			Sema = SemaBuiltin::getUIntType();
			break;
		case ASTBuiltinTypeKind::TYPE_ULONG:
			Sema = SemaBuiltin::getULongType();
			break;
		case ASTBuiltinTypeKind::TYPE_FLOAT:
			Sema = SemaBuiltin::getFloatType();
			break;
		case ASTBuiltinTypeKind::TYPE_DOUBLE:
			Sema = SemaBuiltin::getDoubleType();
			break;
		case ASTBuiltinTypeKind::TYPE_COMPLEX:
			Sema = SemaBuiltin::getComplexType();
			break;
		case ASTBuiltinTypeKind::TYPE_STRING:
			Sema = SemaBuiltin::getStringType();
			break;
		case ASTBuiltinTypeKind::TYPE_ERROR:
			Sema = SemaBuiltin::getErrorType();
			break;
	}
	CurrentType = Sema;
}

void Resolver::visit(ASTNamedType &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTNamedType)");
	if (!AST.isVisited()) {
		AST.setVisited(true);
	}

	SymbolTable *Scope = CurrentScope;
	Symbol *Sym = Reg.LookupNamedType(AST, Scope);
	if (!Sym) {
		return;
	}
	SemaType *Sema = static_cast<SemaType *>(Sym->getRef());

	CurrentType = Sema;
}

void Resolver::visit(ASTArrayType &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTArrayType)");
	if (!AST.isVisited()) {
		AST.setVisited(true);

		// Resolve Element Type
		ASTType * ElementType = AST.getElementType();
		ElementType->accept(*this);
		SemaType *ElementSemaType = CurrentType;

		// Resolve Size Expression
		SemaExpr *SizeExpr = nullptr;
		if (AST.getSizeExpr()) {
			AST.getSizeExpr()->accept(*this);
			SizeExpr = CurrentExpr;

			// Validate Size Expression Type
			if (SizeExpr->getType()->isInteger() == false) {
				Diag(AST.getSizeExpr()->getLocation(), diag::err_sema_array_size_not_integer);
				return;
			}
		}

		// Create Sema Array Type
		SemaArrayType *Sema = SemaBuiltin::CreateArrayType(ElementSemaType, SizeExpr);
		CurrentType = Sema;
	}
}

// Returns true if Expr produces a heap-allocated string at runtime.

SemaSmartAlloc *Resolver::RegisterSmartAlloc(SemaExpr *Expr) {
	if (!CurrentSemaBlock || !Expr)
		return nullptr;

	SemaCall *SmartCall = nullptr;
	auto CheckCall = [&](SemaExpr *E) {
		if (E->getKind() == SemaKind::CALL) {
			SemaCall *C = static_cast<SemaCall *>(E);
			ASTCallKind CK = C->getAST().getCallKind();
			if (CK == ASTCallKind::CALL_NEW_UNIQUE ||
			    CK == ASTCallKind::CALL_NEW_SHARED ||
			    CK == ASTCallKind::CALL_NEW_WEAK)
				SmartCall = C;
		}
	};

	if (Expr->getKind() == SemaKind::BINARY)
		CheckCall(static_cast<SemaBinary *>(Expr)->getRight());
	else
		CheckCall(Expr);

	if (SmartCall) {
		SemaSmartAlloc *Alloc = new SemaSmartAlloc(SmartCall);
		CurrentSemaBlock->addAlloc(Alloc);
		return Alloc;
	}
	return nullptr;
}

void Resolver::visit(ASTExprStmt &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTExprStmt)");
	CurrentStmt = &AST;
	ASTExpr *Expr = AST.getExpr();

	Expr->accept(*this);
	SemaExpr *ResolvedExpr = CurrentExpr;

	// Create SemaExprStmt and add to current block
	if (CurrentSemaBlock && ResolvedExpr) {
		// Detect string reassignment: any assign to a heap-owned string variable
		// requires freeing the old buffer before storing the new one.
		// Insert a SemaDeleteStmt for the LHS variable immediately before the assign.
		if (ResolvedExpr->getKind() == SemaKind::BINARY) {
			SemaBinary *Bin = static_cast<SemaBinary *>(ResolvedExpr);
			if (Bin->getAST().isAssign()) {
				SemaExpr *LHS = Bin->getLeft();
				SemaKind LK = LHS ? LHS->getKind() : SemaKind::VALUE;
				SemaVar *LHSVar = nullptr;
				if (LK == SemaKind::LOCAL_VAR || LK == SemaKind::PARAM_VAR ||
				    LK == SemaKind::ERROR_VAR || LK == SemaKind::ATTRIBUTE ||
				    LK == SemaKind::INSTANCE_VAR)
					LHSVar = static_cast<SemaVar *>(LHS);
				if (LHSVar && LHSVar->getStringAlloc())
					CurrentSemaBlock->addContent(SemaBuilder::CreateDeleteStmt(&AST, LHS));
			}
		}

		SemaExprStmt *ExprStmt = SemaBuilder::CreateExprStmt(&AST, ResolvedExpr);
		CurrentSemaBlock->addContent(ExprStmt);

		// Register smart-pointer alloc: handles direct call and binary assignment (a = new unique T())
		RegisterSmartAlloc(ResolvedExpr);
	}
}

void Resolver::visit(ASTDeclStmt &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTDeclStmt)");
	CurrentStmt = &AST;
	ASTLocalVar *LV = AST.getLocalVar();

	// Resolve LocalVar Type
	LV->accept(*this);
	SemaLocalVar *LocalVar = LV->getSymbol() ?
		LV->getSymbol()->getRefAs<SemaLocalVar>() : nullptr;

	// Resolve Initialization Expression
	SemaExpr *DeclExpr = nullptr;
	if (AST.getExpr()) {
		AST.getExpr()->accept(*this);
		DeclExpr = CurrentExpr;
	}

	// Type check: initializer must be compatible with declared type
	if (LocalVar && DeclExpr && LocalVar->getType()) {
		Validator->CheckAssignment(LV->getLocation(), LocalVar->getType(), DeclExpr);
	}

	// Check for array without size expression or initialization expression
	if (LocalVar && LocalVar->getType() && LocalVar->getType()->isArray()) {
		SemaArrayType *ArrayType = static_cast<SemaArrayType *>(LocalVar->getType());
		if (ArrayType->getSizeExpr() == nullptr && AST.getExpr() == nullptr) {
			Diag(LV->getLocation(), diag::err_sema_array_size_missing);
		}
	}

	// Register a SemaSmartAlloc for any CALL_NEW_* in the init expression,
	// and bind it to the local variable so copies can be detected later.
	if (SemaSmartAlloc *NewAlloc = RegisterSmartAlloc(DeclExpr))
		if (LocalVar)
			LocalVar->setAlloc(NewAlloc);

	// Propagate SmartAlloc when the init expression is itself a SemaVar (var-to-var assignment).
	// Covers: var b = a  (direct) or  var b = someExpr  where the binary RHS is a SemaVar.
	if (LocalVar && DeclExpr) {
		SemaVar *SrcVar = nullptr;
		SemaKind IK = DeclExpr->getKind();
		if (IK == SemaKind::LOCAL_VAR || IK == SemaKind::PARAM_VAR ||
		    IK == SemaKind::ERROR_VAR  || IK == SemaKind::ATTRIBUTE  ||
		    IK == SemaKind::INSTANCE_VAR) {
			SrcVar = static_cast<SemaVar *>(DeclExpr);
		} else if (IK == SemaKind::BINARY) {
			SemaExpr *Right = static_cast<SemaBinary *>(DeclExpr)->getRight();
			SemaKind RK = Right->getKind();
			if (RK == SemaKind::LOCAL_VAR || RK == SemaKind::PARAM_VAR ||
			    RK == SemaKind::ERROR_VAR  || RK == SemaKind::ATTRIBUTE  ||
			    RK == SemaKind::INSTANCE_VAR) {
				SrcVar = static_cast<SemaVar *>(Right);
			}
		}
		if (SrcVar && SrcVar->getSmartAlloc()) {
			if (SrcVar->getSmartAlloc()->isUnique()) {
				Diag(LV->getLocation(), diag::err_sema_unique_copy) << SrcVar->getAST()->getName();
			} else if (SrcVar->getSmartAlloc()->isShared()) {
				// Each copy gets its own SemaSmartAlloc entry wrapping the same Call.
				// Call->getCodeGen()->getValue() is always the right pointer for cleanup
				// because every allocation has its own SA — no variable tracking needed.
				SemaSmartAlloc *CopyAlloc = new SemaSmartAlloc(
					SrcVar->getSmartAlloc()->getCall());
				LocalVar->setAlloc(CopyAlloc);
				CurrentSemaBlock->addAlloc(CopyAlloc);
				SrcVar->getSmartAlloc()->incrReferenceCounter();
			} else if (SrcVar->getSmartAlloc()->isWeak()) {
				// Weak copy: each copy owns a SA entry and calls free() at its own
				// scope exit. The first copy to exit scope frees the object; all
				// remaining copies become dangling — programmer's responsibility.
				// Refcount is NOT incremented: weak never retains.
				SemaSmartAlloc *CopyAlloc = new SemaSmartAlloc(
					SrcVar->getSmartAlloc()->getCall());
				LocalVar->setAlloc(CopyAlloc);
				CurrentSemaBlock->addAlloc(CopyAlloc);
			}
		}
	}

	// Non-const string variables with an initializer always own a heap buffer:
	// register scope-exit free. Const strings and uninitialized strings (zero value)
	// use a global/null pointer and never need freeing.
	if (LocalVar && DeclExpr && LocalVar->getType() && LocalVar->getType()->isString() && !LocalVar->isConstant()) {
		if (CurrentSemaBlock) {
			SemaStringAlloc *SA = new SemaStringAlloc(LocalVar);
			CurrentSemaBlock->addAlloc(SA);
			LocalVar->setAlloc(SA);
		}
	}

	// Create SemaDeclStmt and add to current block
	if (CurrentSemaBlock) {
		SemaDeclStmt *SemaStmt = SemaBuilder::CreateDeclStmt(&AST, LocalVar, DeclExpr);
		CurrentSemaBlock->addContent(SemaStmt);
	}
}

void Resolver::visit(ASTFailStmt &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTFailStmt)");
	CurrentStmt = &AST;

	if (CurrentFunction == nullptr) {
		Diag(AST.getLocation(), diag::err_sema_fail_outside_function);
		return;
	}

	if (CurrentHandleStmt == nullptr) {
		CurrentFunction->setFallible(true);
	}

	SemaFailStmt *SemaStmt = SemaBuilder::CreateFailStmt(&AST);

	// Resolve the optional first expression if it exists
	ASTExpr *FirstExpr = AST.getFirstExpr();
	if (FirstExpr != nullptr) {
		FirstExpr->accept(*this);
		SemaStmt->setFirst(CurrentExpr);
	}

	// Resolve the optional second expression if it exists
	ASTExpr *SecondExpr = AST.getSecondExpr();
	if (SecondExpr != nullptr) {
		SecondExpr->accept(*this);
		SemaStmt->setSecond(CurrentExpr);
	}

	// Create SemaFailStmt and add to current block
	if (CurrentSemaBlock) {
		CurrentSemaBlock->addContent(SemaStmt);
	}
}

void Resolver::visit(ASTHandleStmt &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTHandleStmt)");
	CurrentStmt = &AST;

	ASTHandleStmt *ParentHandle = CurrentHandleStmt;
	CurrentHandleStmt = &AST;

	CurrentErrorHandler = SemaBuilder::CreateErrorHandler();

	// Create SemaHandleStmt and set error handler on it
	SemaHandleStmt *SemaStmt = SemaBuilder::CreateHandleStmt(&AST);
	SemaStmt->setErrorHandler(CurrentErrorHandler);

	// Capture the handle body using a capture block
	SemaBlockStmt *SavedBlock = CurrentSemaBlock;
	SemaBlockStmt *HandleCapture = SemaBuilder::CreateBlockStmt(nullptr);
	CurrentSemaBlock = HandleCapture;
	EnterScope();
	for (ASTStmt *Stmt : AST.getHandle()->getContent()) {
		Stmt->accept(*this);
	}
	ExitScope();
	CurrentSemaBlock = SavedBlock;

	// Set the captured handle body on the SemaHandleStmt
	SemaStmt->setHandle(HandleCapture);

	// Add to current block
	if (CurrentSemaBlock) {
		CurrentSemaBlock->addContent(SemaStmt);
	}

	CurrentHandleStmt = ParentHandle;
}

void Resolver::visit(ASTReturnStmt &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTReturnStmt)");
	CurrentStmt = &AST;

	if (CurrentSemaBlock) {
		SemaReturnStmt *SemaStmt = SemaBuilder::CreateReturnStmt(&AST);
		CurrentSemaBlock->addContent(SemaStmt);
	}
}

void Resolver::visit(ASTDeleteStmt &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTDeleteStmt)");
	CurrentStmt = &AST;
	ASTExpr * Expr = AST.getExpr();

	Expr->accept(*this);
	SemaExpr *ResolvedExpr = CurrentExpr;

	// Create SemaDeleteStmt and add to current block
	if (CurrentSemaBlock && ResolvedExpr) {
		SemaDeleteStmt *SemaStmt = SemaBuilder::CreateDeleteStmt(&AST, ResolvedExpr);
		CurrentSemaBlock->addContent(SemaStmt);
	}
}


void Resolver::visit(ASTBreakStmt &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTBreakStmt)");
	CurrentStmt = &AST;

	if (LoopDepth == 0 && SwitchDepth == 0) {
		Diag(AST.getLocation(), diag::err_sema_break_outside_loop);
		return;
	}

	// Create SemaBreakStmt and add to current block
	if (CurrentSemaBlock) {
		SemaBreakStmt *SemaStmt = SemaBuilder::CreateBreakStmt(&AST);
		CurrentSemaBlock->addContent(SemaStmt);
	}
}

void Resolver::visit(ASTContinueStmt &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTContinueStmt)");
	CurrentStmt = &AST;

	if (LoopDepth == 0) {
		Diag(AST.getLocation(), diag::err_sema_continue_outside_loop);
		return;
	}

	// Create SemaContinueStmt and add to current block
	if (CurrentSemaBlock) {
		SemaContinueStmt *SemaStmt = SemaBuilder::CreateContinueStmt(&AST);
		CurrentSemaBlock->addContent(SemaStmt);
	}
}

void Resolver::visit(ASTBlockStmt &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTBlockStmt)");
	CurrentStmt = &AST;

	// Save parent SemaBlock
	SemaBlockStmt *ParentSemaBlock = CurrentSemaBlock;

	// Create new SemaBlockStmt for this block
	SemaBlockStmt *NewBlock = SemaBuilder::CreateBlockStmt(&AST);
	CurrentSemaBlock = NewBlock;

	// Only enter a new scope for nested blocks, not for function body
	// (function body scope was already entered by Resolve())
	bool IsNestedBlock = (ParentSemaBlock != nullptr);
	if (IsNestedBlock) {
		EnterScope();
	}

	// Resolve Statements; detect unreachable code after unconditional jumps
	bool Terminated = false;
	for (ASTStmt *Stmt : AST.getContent()) {
		if (Terminated) {
			Diag(Stmt->getLocation(), diag::warn_sema_unreachable_code);
			break;
		}
		Stmt->accept(*this);
		ASTStmtKind SK = Stmt->getStmtKind();
		if (SK == ASTStmtKind::STMT_RETURN || SK == ASTStmtKind::STMT_FAIL ||
		    SK == ASTStmtKind::STMT_BREAK  || SK == ASTStmtKind::STMT_CONTINUE) {
			Terminated = true;
		}
	}

	// Exit Block Scope for nested blocks
	if (IsNestedBlock) {
		ExitScope();
	}

	// Add new block as content of parent, or set as function body
	if (ParentSemaBlock) {
		ParentSemaBlock->addContent(NewBlock);
	} else if (CurrentFunction && !CurrentFunction->getBody()) {
		CurrentFunction->setBody(NewBlock);
	}

	// Restore parent SemaBlock
	CurrentSemaBlock = ParentSemaBlock;
}

void Resolver::visit(ASTRuleStmt &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTRuleStmt)");
	CurrentStmt = &AST;
	AST.getExpr()->accept(*this);
	AST.getStmt()->accept(*this);
}

void Resolver::visit(ASTIfStmt &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTIfStmt)");
	CurrentStmt = &AST;

	// Resolve condition
	AST.getExpr()->accept(*this);
	SemaExpr *CondExpr = CurrentExpr;

	// Resolve then block — use a temporary capture block
	SemaBlockStmt *SavedBlock = CurrentSemaBlock;
	SemaBlockStmt *ThenCapture = SemaBuilder::CreateBlockStmt(nullptr);
	CurrentSemaBlock = ThenCapture;
	EnterScope();
	for (ASTStmt *Stmt : static_cast<ASTBlockStmt *>(AST.getStmt())->getContent()) {
		Stmt->accept(*this);
	}
	ExitScope();
	CurrentSemaBlock = SavedBlock;

	// Create SemaIfStmt with the captured then block
	SemaIfStmt *SemaIf = SemaBuilder::CreateIfStmt(&AST, CondExpr, ThenCapture);

	// Elsif Blocks
	for (ASTRuleStmt *Elsif : AST.getElsif()) {
		Elsif->getExpr()->accept(*this);
		SemaExpr *ElsifExpr = CurrentExpr;

		// Capture elsif body
		SemaBlockStmt *ElsifCapture = SemaBuilder::CreateBlockStmt(nullptr);
		CurrentSemaBlock = ElsifCapture;
		EnterScope();
		for (ASTStmt *Stmt : static_cast<ASTBlockStmt *>(Elsif->getStmt())->getContent()) {
			Stmt->accept(*this);
		}
		ExitScope();
		CurrentSemaBlock = SavedBlock;

		SemaIf->addElsif(ElsifExpr, ElsifCapture);
	}

	// Else Block
	if (AST.getElse()) {
		SemaBlockStmt *ElseCapture = SemaBuilder::CreateBlockStmt(nullptr);
		CurrentSemaBlock = ElseCapture;
		EnterScope();
		for (ASTStmt *Stmt : static_cast<ASTBlockStmt *>(AST.getElse())->getContent()) {
			Stmt->accept(*this);
		}
		ExitScope();
		CurrentSemaBlock = SavedBlock;
		SemaIf->setElse(ElseCapture);
	}

	// Add to current block
	if (CurrentSemaBlock) {
		CurrentSemaBlock->addContent(SemaIf);
	}
}

void Resolver::visit(ASTSwitchStmt &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTSwitchStmt)");
	CurrentStmt = &AST;

	// Switch Variable
	AST.getExpr()->accept(*this);
	SemaExpr *SwitchExpr = CurrentExpr;
	SemaType * CaseType = SwitchExpr ? SwitchExpr->getType() : nullptr;

	// Create SemaSwitchStmt
	SemaSwitchStmt *SemaSwitch = SemaBuilder::CreateSwitchStmt(&AST, SwitchExpr);

	++SwitchDepth;

	SemaBlockStmt *SavedBlock = CurrentSemaBlock;

	// Case Blocks
	for (ASTRuleStmt *Case : AST.getCases()) {
		Case->getExpr()->accept(*this);
		SemaExpr *CaseExpr = CurrentExpr;
		if (CaseExpr && CaseType) CaseExpr->setType(CaseType);

		// Capture case body
		SemaBlockStmt *CaseCapture = SemaBuilder::CreateBlockStmt(nullptr);
		CurrentSemaBlock = CaseCapture;
		EnterScope();
		for (ASTStmt *Stmt : static_cast<ASTBlockStmt *>(Case->getStmt())->getContent()) {
			Stmt->accept(*this);
		}
		ExitScope();
		CurrentSemaBlock = SavedBlock;

		// Warn if non-empty case doesn't end with a terminator (fallthrough)
		const auto &CaseContent = CaseCapture->getContent();
		if (!CaseContent.empty()) {
			SemaKind LastKind = CaseContent.back()->getKind();
			if (LastKind != SemaKind::STMT_BREAK && LastKind != SemaKind::STMT_RETURN &&
			    LastKind != SemaKind::STMT_FAIL) {
				Diag(Case->getExpr()->getLocation(), diag::warn_sema_switch_fallthrough);
			}
		}

		SemaSwitch->addCase(CaseExpr, CaseCapture);
	}

	// Default Block
	if (AST.getDefault()) {
		SemaBlockStmt *DefaultCapture = SemaBuilder::CreateBlockStmt(nullptr);
		CurrentSemaBlock = DefaultCapture;
		EnterScope();
		for (ASTStmt *Stmt : static_cast<ASTBlockStmt *>(AST.getDefault())->getContent()) {
			Stmt->accept(*this);
		}
		ExitScope();
		CurrentSemaBlock = SavedBlock;
		SemaSwitch->setDefault(DefaultCapture);
    }

	--SwitchDepth;

	// Warn if switching on an enum without a default and not all variants are covered
	if (CaseType && CaseType->getKind() == SemaKind::TYPE_ENUM && AST.getDefault() == nullptr) {
		SemaEnumType *EnumT = static_cast<SemaEnumType *>(CaseType);
		llvm::SmallPtrSet<SemaEnumEntry *, 8> Covered;
		for (const auto &Rule : SemaSwitch->getCases()) {
			if (Rule.Expr && Rule.Expr->getKind() == SemaKind::ENUM_ENTRY) {
				Covered.insert(static_cast<SemaEnumEntry *>(Rule.Expr));
			}
		}
		for (const auto &KV : EnumT->getEntries()) {
			if (!Covered.count(KV.second)) {
				Diag(AST.getLocation(), diag::warn_sema_switch_enum_not_exhaustive)
				    << EnumT->getName();
				break;
			}
		}
	}

	// Add to current block
	if (CurrentSemaBlock) {
		CurrentSemaBlock->addContent(SemaSwitch);
	}
}

void Resolver::visit(ASTLoopStmt &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTLoopStmt)");
	CurrentStmt = &AST;

	// Create SemaLoopStmt
	SemaLoopStmt *SemaLoop = SemaBuilder::CreateLoopStmt(&AST);

	++LoopDepth;

	// Enter Loop Scope to keep init variables visible in condition, body, and post
	EnterScope();

	SemaBlockStmt *SavedBlock = CurrentSemaBlock;

	// Capture Init statements
	SemaBlockStmt *InitCapture = SemaBuilder::CreateBlockStmt(nullptr);
	CurrentSemaBlock = InitCapture;
	for (ASTStmt *S : AST.getInit()) {
		S->accept(*this);
	}
	CurrentSemaBlock = SavedBlock;
	for (SemaStmt *S : InitCapture->getContent()) {
		SemaLoop->addInit(S);
	}

	// Resolve condition
	SemaExpr *CondExpr = nullptr;
	if (AST.getExpr()) {
		AST.getExpr()->accept(*this);
		CondExpr = CurrentExpr;
	}
	SemaLoop->setCond(CondExpr);

	// Capture Loop body
	SemaBlockStmt *BodyCapture = SemaBuilder::CreateBlockStmt(nullptr);
	CurrentSemaBlock = BodyCapture;
	EnterScope();
	for (ASTStmt *Stmt : static_cast<ASTBlockStmt *>(AST.getLoop())->getContent()) {
		Stmt->accept(*this);
	}
	ExitScope();
	CurrentSemaBlock = SavedBlock;
	SemaLoop->setBody(BodyCapture);

	// Warn on `while(true)` without any break or fail in the body
	if (AST.getExpr() && AST.getExpr()->getExprKind() == ASTExprKind::EXPR_VALUE) {
		ASTValue *CondVal = static_cast<ASTValue *>(AST.getExpr());
		if (CondVal->isBool() && static_cast<ASTBoolValue *>(CondVal)->getValue()) {
			bool HasEscape = false;
			for (SemaStmt *S : BodyCapture->getContent()) {
				SemaKind K = S->getKind();
				if (K == SemaKind::STMT_BREAK || K == SemaKind::STMT_FAIL) {
					HasEscape = true;
					break;
				}
			}
			if (!HasEscape) {
				Diag(AST.getLocation(), diag::warn_sema_infinite_loop);
			}
		}
	}

	// Capture Post statements
	SemaBlockStmt *PostCapture = SemaBuilder::CreateBlockStmt(nullptr);
	CurrentSemaBlock = PostCapture;
	for (ASTStmt *S : AST.getPost()) {
		S->accept(*this);
	}
	CurrentSemaBlock = SavedBlock;
	for (SemaStmt *S : PostCapture->getContent()) {
		SemaLoop->addPost(S);
	}

	// Exit Loop Scope
	ExitScope();

	--LoopDepth;

	// Add to current block
	if (CurrentSemaBlock) {
		CurrentSemaBlock->addContent(SemaLoop);
	}
}

void Resolver::visit(ASTLoopInStmt &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTLoopInStmt)");
	CurrentStmt = &AST;

	AST.getItem()->accept(*this);
	SemaExpr *ItemExpr = CurrentExpr;
    AST.getList()->accept(*this);
	SemaExpr *ListExpr = CurrentExpr;

	++LoopDepth;

	// Loop Statement
	AST.getStmt()->accept(*this);

	--LoopDepth;

	// Create SemaLoopInStmt and add to block
	SemaLoopInStmt *SemaLoopIn = SemaBuilder::CreateLoopInStmt(&AST, ItemExpr, ListExpr, nullptr);
	if (CurrentSemaBlock) {
		CurrentSemaBlock->addContent(SemaLoopIn);
	}
}

void Resolver::visit(ASTIdentifier &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTIdentifier)");

	// Invalid if it has Parent
	if (AST.getParent() != nullptr) {
		Diag(diag::err_invalid_behavior);
		return;
	}

	if (!AST.isVisited()) {
		AST.setVisited(true);

		// DEBUG: trace identifier lookup
		// llvm::errs() << "DEBUG Identifier: name=" << AST.getName();

		// ---------------------------------------
		// Try local and parent scopes (class, module, namespace, global scope)
		// ---------------------------------------
		SymbolTable * Scope = CurrentScope;
		Symbol *CurrentSymbol = nullptr;
		while (!CurrentSymbol && Scope) {
			llvm::SmallVector<Symbol *, 8> *Symbols;
			if ((Symbols = Scope->lookupInParents(AST.getName()))) {
				if (Symbols) {
					CurrentSymbol = (*Symbols)[0];
					break;
				}
			}
			Scope = Scope->getParent();
		}

		// ---------------------------------------
		// Not Found → Error
		// ---------------------------------------
		if (!CurrentSymbol) {
			Diag(AST.getLocation(), diag::err_sema_unresolved_identifier) << AST.getName();
			return;
		}

		// Always store the resolved Symbol on the AST
		AST.setSymbol(CurrentSymbol);

		// Sym Found as Variable - set CurrentExpr
		if (CurrentSymbol->isVarKind()) {
			SemaVar *Sema = static_cast<SemaVar *>(CurrentSymbol->getRef());
			CurrentExpr = Sema;

			// Mark local var as read (unless this is the write-only LHS of a plain '=')
			if (!InAssignLHS && CurrentSymbol->getKind() == SymbolKind::LOCAL_VAR) {
				UnusedLocalVars.erase(static_cast<SemaLocalVar *>(Sema));
			}
		}
	}
}

void Resolver::visit(ASTMember &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTMember)");

	// Invalid if it has no Parent
	if (AST.getParent() == nullptr) {
		Diag(diag::err_invalid_behavior);
		return;
	}

	// After visiting parent
	// CurrentSymbol should be set
	if (!AST.isVisited()) {

		// Visit parent first and resolve Parent Symbol
		AST.getParent()->accept(*this);

		// Read the resolved Symbol from the parent
		Symbol *ParentSymbol = nullptr;
		if (AST.getParent()->getExprKind() == ASTExprKind::EXPR_IDENTIFIER) {
			ParentSymbol = static_cast<ASTIdentifier *>(AST.getParent())->getSymbol();
		} else if (AST.getParent()->getExprKind() == ASTExprKind::EXPR_MEMBER) {
			ParentSymbol = static_cast<ASTMember *>(AST.getParent())->getSymbol();
		} else if (AST.getParent()->getExprKind() == ASTExprKind::EXPR_CALL) {
			ParentSymbol = static_cast<ASTCall *>(AST.getParent())->getSymbol();
		}


		if (!ParentSymbol) {
			return; // already reported error in visiting parent
		}

		// ---------------------------------------
		// Try in current scopes (namespace, class, enum, current scopes)
		// ---------------------------------------
		AST.setVisited(true);
		SemaExpr *Sema = nullptr;

		// check namespace: member may be a sub-namespace (e.g. "string" in "fly.string")
		if (ParentSymbol->getKind() == SymbolKind::NAMESPACE) {
			SemaNameSpace *ParentNS = static_cast<SemaNameSpace *>(ParentSymbol->getRef());
			llvm::SmallVector<Symbol *, 8> *SubSyms = ParentNS->getSymbols()->lookup(AST.getName());
			if (SubSyms && !SubSyms->empty() && (*SubSyms)[0]->getKind() == SymbolKind::NAMESPACE) {
				AST.setSymbol((*SubSyms)[0]);
				CurrentExpr = nullptr; // not a value expression
				return;
			}
			Diag(diag::err_invalid_behavior); // Member access on namespace not resolved
			return;
		}

		if (ParentSymbol->getKind() == SymbolKind::CLASS) {
			SemaClassType *ParentSema = static_cast<SemaClassType *>(ParentSymbol->getRef());

			// Feature 5b: BaseClass.field inside a derived class instance method →
			// resolve as an instance access through 'this', navigating to the embedded base.
			if (CurrentClass && CurrentClass->isDerived(ParentSema) &&
			    CurrentFunction && CurrentFunction->getKind() == SemaKind::METHOD &&
			    !static_cast<SemaClassMethod *>(CurrentFunction)->isStatic()) {
				SemaClassInstance *ThisVar = static_cast<SemaClassMethod *>(CurrentFunction)->getThis();
				Sema = ResolveMemberSymbol(AST, ParentSema->getSymbols(), SemaKind::ATTRIBUTE, ThisVar);
			}

			if (!Sema) {
				// Static class attribute access
				Sema = ResolveMemberSymbol(AST, ParentSema->getSymbols(), SemaKind::ATTRIBUTE);
			}
			if (!Sema) return;
		} else if (ParentSymbol->getKind() == SymbolKind::ENUM) {
			SemaEnumType *ParentSema = static_cast<SemaEnumType *>(ParentSymbol->getRef());
			// Handle built-in enum members
			if (AST.getName() == "list") {
				Sema = SemaBuilder::CreateEnumList(ParentSema);
			} else {
				Sema = ResolveMemberSymbol(AST, ParentSema->getSymbols(), SemaKind::ENUM_ENTRY);
				if (!Sema) return;
			}
		} else if (ParentSymbol->isVarKind()) {
			SemaVar *ParentVar = static_cast<SemaVar *>(ParentSymbol->getRef());

			if (ParentVar->getType()->isClass()) {
				SemaClassType * ClassType = static_cast<SemaClassType *>(ParentVar->getType());
				// Use LookupAttribute to traverse the inheritance hierarchy so that
				// fields inherited from base classes are found even though they live
				// in the base class's symbol table rather than the derived class's.
				SemaClassAttribute *Attr = ClassType->LookupAttribute(AST.getName());
				if (!Attr) {
					Diag(AST.getLocation(), diag::err_sema_unresolved_identifier) << AST.getName();
					return;
				}
				Sema = ResolveMemberSymbol(AST, Attr->getClass().getSymbols(), SemaKind::ATTRIBUTE, ParentVar);
				if (!Sema) return;
			} else if (ParentVar->getType()->isEnum()) {
				SemaEnumType * EnumType = static_cast<SemaEnumType *>(ParentVar->getType());
				Sema = ResolveMemberSymbol(AST, EnumType->getSymbols(), SemaKind::ENUM_ENTRY, ParentVar);
				if (!Sema) return;
			} else {
				// Parent is not an object type
				Diag(diag::err_invalid_behavior);
				return;
			}
		//}  else if (CurrentSymbol->getKind() == SymbolKind::FUNCTION) { // return is always void
		//	ParentType = static_cast<SemaFunctionBase *>(CurrentSymbol->getRef())->getReturnType();
		} else if (ParentSymbol->getKind() == SymbolKind::VALUE) {
			// Cannot exists Value after a Member access
			Diag(diag::err_invalid_behavior);
			return;
		}

		// Configure CurrentExpr
		CurrentExpr = Sema;
	}
}

// Build a human-readable signature string like "foo(int, float)" for diagnostics.
static std::string BuildCandidateSignature(SemaFunctionBase *F) {
	std::string Sig = F->getName().str() + "(";
	bool First = true;
	for (SemaParam *P : F->getParams()) {
		if (!First) Sig += ", ";
		Sig += P->getType()->getName();
		First = false;
	}
	Sig += ")";
	return Sig;
}

void Resolver::visit(ASTCall &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTCall)");

	if (!AST.isVisited()) {
		AST.setVisited(true);

		// Save the current scope
		SymbolTable *SavedScope = CurrentScope;

		// ---------------------------------------
		// Resolve parent Symbol if present
		// ---------------------------------------
		Symbol *ParentSymbol = nullptr;
		SemaExpr *ResolvedParentExpr = nullptr;
		if (AST.getParent()) {
			AST.getParent()->accept(*this);
			ResolvedParentExpr = CurrentExpr; // Save the parent expression before ResolveCallArgs overwrites it
			if (AST.getParent()->getExprKind() == ASTExprKind::EXPR_IDENTIFIER) {
				ParentSymbol = static_cast<ASTIdentifier *>(AST.getParent())->getSymbol();
			} else if (AST.getParent()->getExprKind() == ASTExprKind::EXPR_MEMBER) {
				ParentSymbol = static_cast<ASTMember *>(AST.getParent())->getSymbol();
			} else if (AST.getParent()->getExprKind() == ASTExprKind::EXPR_CALL) {
				ParentSymbol = static_cast<ASTCall *>(AST.getParent())->getSymbol();
			}
		}

		// This is the Sema Call to be resolved
		SemaCall *Sema = nullptr;

		// Resolve arguments in the caller's scope (before switching to class/enum scope)
		SmallVector<SemaType *, 8> ArgTypes = ResolveCallArgs(&AST);

		// ---------------------------------------
		// Try in current scopes (namespace, class, enum, current scopes)
		// ---------------------------------------
		if (ParentSymbol) {
			if (ParentSymbol->getKind() == SymbolKind::NAMESPACE) {
				CurrentScope = static_cast<SemaNameSpace *>(ParentSymbol->getRef())->getSymbols();
			} else if (ParentSymbol->getKind() == SymbolKind::CLASS) {
				SemaClassType *ClassType = static_cast<SemaClassType *>(ParentSymbol->getRef());
				CurrentScope = ClassType->getSymbols();
			} else if (ParentSymbol->isVarKind()) {
				SemaType *ParentType = static_cast<SemaVar *>(ParentSymbol->getRef())->getType();
				if (ParentType->isClass()) {
					CurrentScope = static_cast<SemaClassType *>(ParentType)->getSymbols();
				} else if (ParentType->isEnum()) {
					CurrentScope = static_cast<SemaEnumType *>(ParentType)->getSymbols();
				}
			}
			// else if (ParentSymbol->getKind() == SymbolKind::FUNCTION) {
			//	ParentType = static_cast<SemaFunctionBase *>(ParentSymbol->getRef())->getReturnType();
			//}
			else if (ParentSymbol->getKind() == SymbolKind::VALUE) {
				// Cannot exists Value after a Member access
				Diag(diag::err_invalid_behavior);
				CurrentScope = SavedScope;
				return;
			}
		}

		// Create a new instance using a Constructor
		if (AST.getCallKind() == ASTCallKind::CALL_NEW ||
			AST.getCallKind() == ASTCallKind::CALL_NEW_UNIQUE ||
			AST.getCallKind() == ASTCallKind::CALL_NEW_SHARED ||
			AST.getCallKind() == ASTCallKind::CALL_NEW_WEAK) {

			// Constructor cannot have a parent
			if (ParentSymbol) {
				Diag(AST.getLocation(), diag::err_sema_ctor_with_parent) << AST.getName();
				CurrentScope = SavedScope;
				return;
			}

			// Lookup current call name into Names
			Symbol *Sym = Reg.LookupNamedType(AST.getName(), CurrentScope);
			if (!Sym) {
				Diag(AST.getLocation(), diag::err_sema_unknown_type) << AST.getName();
				CurrentScope = SavedScope;
				return;
			}

			// Check if type is a Class
			if (Sym->getRef()->getKind() != SemaKind::TYPE_CLASS) {
				Diag(AST.getLocation(), diag::err_sema_new_not_class) << AST.getName();
				CurrentScope = SavedScope;
				return;
			}

			// Lookup Constructor method into Class
			SemaClassType *ClassType = static_cast<SemaClassType *>(Sym->getRef());

			// Cannot instantiate an abstract class
			if (ClassType->isAbstract()) {
				Diag(AST.getLocation(), diag::err_sema_abstract_class_instantiation) << AST.getName();
				CurrentScope = SavedScope;
				return;
			}

			Symbol * CurrentSymbol = Reg.LookupFunction(AST.getName(), ArgTypes, ClassType->getSymbols());

			// Check symbol is resolved
			if (!CurrentSymbol) {
				Diag(AST.getLocation(), diag::err_sema_ctor_not_found) << AST.getName();
				CurrentScope = SavedScope;
				return;
			}

			// check is a class method
			if (CurrentSymbol->getRef()->getKind() != SemaKind::METHOD) {
				Diag(AST.getLocation(), diag::err_sema_symbol_not_method) << AST.getName();
				CurrentScope = SavedScope;
				return;
			}

			// Symbol is Constructor
			SemaClassMethod *Constr = static_cast<SemaClassMethod *>(CurrentSymbol->getRef());

			// Check method visibility
			if (Constr->getVisibility() == SemaVisibilityKind::PRIVATE) {
				Diag(AST.getLocation(), diag::err_sema_ctor_private) << AST.getName();
				CurrentScope = SavedScope;
				return;
			}

			// Check if the Call is a Base Class Constructor Method
			// SemaClassType *CurrentClass = static_cast<SemaClassMethod *>(CurrentFunction)->getClass();
			// SemaType * T = Reg.LookupNamedType(AST.getName(), CurrentScope);
			//
			// if (T->isClass()) {
			// 	SemaClassType * C = static_cast<SemaClassType *>(T);
			//
			// 	// Call to Base Class Constructor Method
			// 	if (C->isBaseOrEquals(CurrentClass)) {
			// 		// Resolve Call with Class Constructor if is not private
			// 		SemaClassMethod *Constructor = static_cast<SemaClassMethod *>(Reg.LookupFunction(AST.getName(), Types, C->getSymbols()));
			//
			// 		// check if Constructor is private
			// 		if (Constructor->getVisibility() == SemaVisibilityKind::PRIVATE) {
			// 			// Error: method is private, cannot be called from outside the class
			// 			Diag(AST.getLocation(), diag::err_syntax_error);
			// 			return;
			// 		}
			// 		// Take the Constructor
			// 		Func = Constructor;
			// 	} else {
			// 		// Static Call to Class Method
			// 		SemaClassMethod *Method = static_cast<SemaClassMethod *>(Reg.LookupFunction(AST.getName(), Types, C->getSymbols()));
			//
			// 		// Check if Method is static
			// 		if (!Method->isStatic()) {
			// 			// Error: method is not static, cannot be called statically
			// 			Diag(AST.getLocation(), diag::err_syntax_error);
			// 		}
			//
			// 		// Check if Method is private
			// 		if (Method->getVisibility() == SemaVisibilityKind::PRIVATE) {
			// 			// Error: method is private, cannot be called from outside the class
			// 			Diag(AST.getLocation(), diag::err_syntax_error);
			// 		}
			//
			// 		// Take the Method
			// 		Func = Method;
			// 	}
			// }
			Sema = SemaBuilder::CreateCall(AST, ClassType, Constr);

			// Store the resolved Symbol on the ASTCall
			AST.setSymbol(CurrentSymbol);

		} else {
			// Lookup Function or Method — three-way overload resolution
			auto Candidates = Reg.FindFunctionCandidates(AST.getName(), CurrentScope);
			if (Candidates.empty()) {
				Diag(AST.getLocation(), diag::err_sema_function_not_found) << AST.getName();
				CurrentScope = SavedScope;
				return;
			}

			auto Matches = Reg.FindFunctionMatches(AST.getName(), ArgTypes, CurrentScope);
			if (Matches.empty()) {
				Diag(AST.getLocation(), diag::err_sema_wrong_args) << AST.getName();
				for (Symbol *C : Candidates) {
					SemaFunctionBase *F = static_cast<SemaFunctionBase *>(C->getRef());
					Diag(F->getAST().getLocation(), diag::note_sema_candidate)
					    << BuildCandidateSignature(F);
				}
				CurrentScope = SavedScope;
				return;
			}

			if (Matches.size() > 1) {
				Diag(AST.getLocation(), diag::err_sema_call_ambiguous) << AST.getName();
				for (Symbol *M : Matches) {
					SemaFunctionBase *F = static_cast<SemaFunctionBase *>(M->getRef());
					Diag(F->getAST().getLocation(), diag::note_sema_candidate)
					    << BuildCandidateSignature(F);
				}
				CurrentScope = SavedScope;
				return;
			}

			Symbol *CurrentSymbol = Matches[0];

			SemaKind RefKind = CurrentSymbol->getRef()->getKind();
			if (RefKind != SemaKind::FUNCTION && RefKind != SemaKind::METHOD) {
				Diag(AST.getLocation(), diag::err_sema_not_callable) << AST.getName();
				CurrentScope = SavedScope;
				return;
			}

			// Sym is a Function or Method
			SemaFunctionBase *Func = static_cast<SemaFunctionBase *>(CurrentSymbol->getRef());
			Sema = SemaBuilder::CreateCall(AST, Func->getReturnType(), Func);

			// Store the resolved Symbol on the ASTCall
			AST.setSymbol(CurrentSymbol);
		}

		// Configure CurrentExpr
		CurrentExpr = Sema;

		// Set the Parent expression for instance method calls
		if (ResolvedParentExpr) {
			Sema->Parent = ResolvedParentExpr;
		}

		// Store resolved argument expressions in SemaCall
		for (SemaExpr *ArgExpr : ResolvedCallArgs) {
			Sema->addArg(ArgExpr);
		}
		ResolvedCallArgs.clear();

		// Set the Call Sema ErrorHandler
		// Search until parent is null or parent is a Handle Stmt
		// When Parent Stmt is nullptr assign Function ErrorHandler to Call ErrorHandler
		Sema->ErrorHandler = CurrentErrorHandler;

		// Restore the scope
		CurrentScope = SavedScope;
	}
}

void Resolver::visit(ASTUnary &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTUnaryOp)");

	// Resolve Expr
	ASTExpr *Expr = AST.getExpr();
	Expr->accept(*this);
	SemaExpr *ResolvedExpr = CurrentExpr;

	// Create Sema
	SemaUnary *Sema = SemaBuilder::CreateUnary(AST, ResolvedExpr);
	CurrentExpr = Sema;
}

void Resolver::visit(ASTBinary &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTBinaryOp)");

	// For a plain '=' whose LHS is a bare identifier the variable is written but
	// not read.  Set InAssignLHS so visit(ASTIdentifier) can skip the read-mark.
	InAssignLHS = AST.getBinaryKind() == ASTBinaryKind::OP_BINARY_ASSIGN &&
	              AST.getLeftExpr()->getExprKind() == ASTExprKind::EXPR_IDENTIFIER;

	// Resolve Left and Right Expr
	AST.getLeftExpr()->accept(*this);
	InAssignLHS = false;
	SemaExpr *Left = CurrentExpr;
	// Any assignment (=, +=, …) to a bare-identifier param counts as a modification
	if (AST.isAssign() && Left && Left->getKind() == SemaKind::PARAM_VAR) {
		UnmodifiedParams.erase(static_cast<SemaParam *>(Left));
	}
    AST.getRightExpr()->accept(*this);
	SemaExpr *Right = CurrentExpr;


	// Promote Types if needed
	PromoteTypes(AST, Left, Right);

	if (Validator->CheckBinary(AST, Left, Right)) {

		// Create Sema
		SemaBinary *Sema = SemaBuilder::CreateBinary(AST, Left, Right);
		CurrentExpr = Sema;
	}
}

void Resolver::visit(ASTTernary &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTTernaryOp)");

	// Resolve Condition Expr
	AST.getConditionExpr()->accept(*this);
	SemaExpr *Cond = CurrentExpr;

	// Validate Condition Type
	Validator->CheckConvertibleTypes(Cond->getType(), SemaBuiltin::getBoolType());

	// Resolve True and False Expr
	AST.getTrueExpr()->accept(*this);
	SemaExpr *TrueExpr = CurrentExpr;
	AST.getFalseExpr()->accept(*this);
	SemaExpr *FalseExpr = CurrentExpr;

	// Promote Number Types if needed
	if (TrueExpr->getType()->isNumber() &&
		FalseExpr->getType()->isNumber()) {
		SemaType *PromotedType = PromoteNumberTypes(
			TrueExpr->getType(),
			FalseExpr->getType()
		);
		TrueExpr->setType(PromotedType);
		FalseExpr->setType(PromotedType);
	}

	// Create Sema
	SemaTernary *Sema = SemaBuilder::CreateTernary(AST, Cond, TrueExpr, FalseExpr);
	CurrentExpr = Sema;
}

void Resolver::visit(ASTCast &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTCast)");

	// Resolve the expression being cast
	AST.getExpr()->accept(*this);
	SemaExpr *From = CurrentExpr;

	// Resolve the target type (sets CurrentType)
	AST.getToType()->accept(*this);
	SemaType *ToType = CurrentType;

	// Build the SemaCast node; its type IS ToType, so Cast->getType() == ToType
	SemaCast *Cast = SemaBuilder::CreateCast(AST, From, ToType);
	CurrentExpr = Cast;

	Validator->CheckCast(From, Cast);
}

void Resolver::visit(ASTBoolValue &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTBoolValue)");
	SemaBoolValue *Sema = SemaBuilder::CreateBoolValue(AST);
	CurrentExpr = Sema;
}

void Resolver::visit(ASTNumberValue &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTNumberValue)");

	// The result type of the number value depends on the left side of an assignment
	SemaValue *Sema = SemaBuilder::CreateNumberValue(AST);
	CurrentExpr = Sema;
}

void Resolver::visit(ASTStringValue &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTStringValue)");
	SemaValue *Sema = SemaBuilder::CreateStringValue(AST);
	CurrentExpr = Sema;
}

void Resolver::visit(ASTArrayValue &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTArrayValue)");

	// Resolve Values
	SemaType *ElementType = nullptr;
	llvm::SmallVector<SemaValue *, 8> Values;
	for (auto Value : AST.getValues()) {
		Value->accept(*this);
		SemaExpr *ValueExpr = CurrentExpr;
		SemaType *ValType = ValueExpr ? ValueExpr->getType() : nullptr;

		// First Value
		if (ElementType == nullptr) {
			ElementType = ValType;
		} else if (ValType && ValType->isNumber()) { // Promote Number Types
			ElementType = (ElementType == nullptr || (ElementType->isNumber() &&
				static_cast<SemaNumberType *>(ValType)->getRank() >
				static_cast<SemaNumberType *>(ElementType)->getRank())) ?
				ValType :
				ElementType;
		} else if (!Validator->CheckEqualTypes(ElementType, ValType)) {
			Diag(Value->getLocation(), diag::err_sema_array_value_type_mismatch);
		}

		Values.push_back(static_cast<SemaValue *>(ValueExpr));
	}

	// Determine Array Type from parent binary assignment's left side
	if (CurrentExpr && CurrentExpr->getKind() == SemaKind::BINARY &&
	           static_cast<SemaBinary *>(CurrentExpr)->getAST().isAssign()) {
		SemaExpr *LeftSema = static_cast<SemaBinary *>(CurrentExpr)->getLeft();
		if (LeftSema && LeftSema->getType() && LeftSema->getType()->isArray()) {
			SemaArrayType *ArrayType = static_cast<SemaArrayType *>(LeftSema->getType());
			ElementType = ArrayType->getElementType();
		}
	}

	// Create Sema
	SemaArrayValue *Sema = SemaBuilder::CreateArrayValue(AST, ElementType, Values);
	CurrentExpr = Sema;
}

void Resolver::visit(ASTStructValue &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTStructValue)");

	llvm::StringMap<SemaValue *> Values;
	for (auto &Entry : AST.getValues()) {
		Entry.second->accept(*this);
		Values.insert(std::make_pair(Entry.getKey(), static_cast<SemaValue *>(CurrentExpr)));
	}

	SemaStructValue *Sema = SemaBuilder::CreateStructValue(AST, Values);
	CurrentExpr = Sema;
}

void Resolver::visit(ASTNullValue &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTNullValue)");
	SemaValue *Sema = SemaBuilder::CreateNullValue(AST);
	CurrentExpr = Sema;
}

void Resolver::visit(ASTUnsetValue &AST) {
	FLY_DEBUG_SCOPE("Resolver", "visit(ASTUnsetValue)");
	SemaValue *Sema = SemaBuilder::CreateUnsetValue(AST);
	CurrentExpr = Sema;
}

void Resolver::Resolver::EnterScope() {
	FLY_DEBUG_SCOPE("Resolver", "EnterScope");
	CurrentScope = CurrentScope->pushScope();
}

void Resolver::Resolver::ExitScope() {
	FLY_DEBUG_SCOPE("Resolver", "ExitScope");
	CurrentScope = CurrentScope->getParent();
}

void Resolver::addSymbol(Symbol *Sym) {
	FLY_DEBUG_SCOPE("Resolver", "addSymbol");
	// Warn when a local variable shadows a variable in an outer scope
	if (Sym->isVarKind() && CurrentScope->getParent()) {
		auto *FoundInCurrent = CurrentScope->lookup(Sym->getName());
		if (!FoundInCurrent) {
			auto *FoundInParents = CurrentScope->getParent()->lookupInParents(Sym->getName());
			if (FoundInParents && !FoundInParents->empty() &&
			    (*FoundInParents)[0]->isVarKind()) {
				SemaVar *Var = static_cast<SemaVar *>(Sym->getRef());
				Diag(Var->getAST()->getLocation(), diag::warn_sema_var_shadow) << Sym->getName();
			}
		}
	}
	CurrentScope->insert(Sym);
}

void Resolver::ResetCurrents() {
	FLY_DEBUG_SCOPE("Resolver", "ResetCurrent");
	CurrentClass = nullptr;
	CurrentEnum = nullptr;
	CurrentFunction = nullptr;
	CurrentComment =nullptr;
	CurrentStmt = nullptr;
}

Resolver::~Resolver() {
}

void Resolver::Resolve() {
	FLY_DEBUG_SCOPE("Resolver", "Resolve");
	// Resolve Modules
	for (auto Module : Reg.getModules()) {

		// Set current scope
		CurrentScope = Module->getSymbols();

		// Resolve Imports
		ResolveImports(Module);

		// Resolve Nodes
		for (auto &Node : Module->getNodes()) {
			if (Node->getKind() == SemaKind::FUNCTION) {
				ResolveFunction(static_cast<SemaFunction *>(Node));
			} else if (Node->getKind() == SemaKind::TYPE_CLASS) {
				ResolveClassType(static_cast<SemaClassType *>(Node));
			}
		}
	}

	// Resolve Functions/Methods Bodies
	for (auto FunctionBase : Reg.getBodies()) {
		CurrentFunction = FunctionBase;
		CurrentClass = nullptr;

		// Function/Method Scope
		CurrentScope = FunctionBase->getSymbols();

		// For non-static class methods, add 'this' to the method scope so the body can reference it
		if (FunctionBase->getKind() == SemaKind::METHOD) {
			SemaClassMethod *Method = static_cast<SemaClassMethod *>(FunctionBase);
			CurrentClass = Method->getClass(); // track owning class for feature 5b
			if (!Method->isStatic()) {
				Symbol *ThisSym = new Symbol(std::string("this"), SymbolKind::VAR, Method->getThis());
				CurrentScope->insert(ThisSym);
			}
		}

		// Enter Body Scope (already created in the first pass, just need to set it as current scope)
		CurrentScope = CurrentScope->getChildren()[0];

		// Reset CurrentSemaBlock for the new function body
		CurrentSemaBlock = nullptr;

		// Start unused-variable tracking fresh for every function body
		UnusedLocalVars.clear();

		// Populate UnmodifiedParams with all non-const parameters of this function
		UnmodifiedParams.clear();
		for (SemaParam *P : FunctionBase->getParams()) {
			if (!P->isConstant())
				UnmodifiedParams.insert(P);
		}

		// Resolve Body - visit(ASTBlockStmt) will create SemaBlockStmt and set as function body
		ASTBlockStmt *Body = FunctionBase->getAST().getBody();
		Body->accept(*this);

		// Emit warnings for local variables declared but never read in this function
		for (SemaLocalVar *V : UnusedLocalVars) {
			Diag(V->getAST()->getLocation(), diag::warn_sema_var_unused) << V->getAST()->getName();
		}
		UnusedLocalVars.clear();

		// Emit warnings for parameters that were never modified (could be declared const).
		// Skip empty-body functions — they are extern stubs; their params are analysed
		// by the C implementation, not the Fly body.
		SemaBlockStmt *ResolvedBody = FunctionBase->getBody();
		bool BodyHasContent = ResolvedBody && !ResolvedBody->getContent().empty();
		if (BodyHasContent) {
			for (SemaParam *P : UnmodifiedParams) {
				Diag(P->getAST()->getLocation(), diag::warn_sema_param_missing_const)
				    << P->getAST()->getName();
			}
		}
		UnmodifiedParams.clear();

		ExitScope();
	}
}

void Resolver::ResolveImports(SemaModule *Module) {
	FLY_DEBUG_SCOPE("Resolver", "ResolveImports");

	for (auto Import : Module->getImports()) {

		// Resolve the target NameSpace, Class or Enum
		Symbol *ImportedSymbol = Reg.LookupImport(Import->getTarget());

		// Check if namespace was found
		if (!ImportedSymbol) {
			std::string TargetName = Helper::Flatten(Import->getTarget());
			Diag(Import->getAST()->getLocation(), diag::err_sema_namespace_not_found) << TargetName;
			continue;
		}

		if (Import->isWildcard()) {
			// Wildcard: add all direct children of the target namespace to module scope
			if (ImportedSymbol->getKind() == SymbolKind::NAMESPACE) {
				SemaNameSpace *NS = static_cast<SemaNameSpace *>(ImportedSymbol->getRef());
				for (auto &Entry : NS->getSymbols()->getAll()) {
					for (Symbol *Sym : Entry.second) {
						Module->getSymbols()->insert(Sym);
					}
				}
			}
		} else if (!Import->getAST()->getAlias().empty()) {
			// Alias: create a new symbol with the alias name pointing to the same ref
			const auto &AliasNames = Import->getAST()->getAlias();
			llvm::StringRef AliasName = AliasNames[0]->getName();
			Symbol *AliasSym = new Symbol(AliasName, ImportedSymbol->getKind(), ImportedSymbol->getRef());
			Module->getSymbols()->insert(AliasSym);
		} else {
			// Plain import: add symbol under its own name
			Module->getSymbols()->insert(ImportedSymbol);
		}
	}
}

/**
 * Resolve Module Function Definitions
 */
void Resolver::ResolveFunction(SemaFunction *Sema) {
	FLY_DEBUG_SCOPE("Resolver", "ResolveFunction");
	ASTFunction &AST = Sema->getAST();

	// External functions (from .fly.h headers) have no body; their params and
	// return type were already resolved in visit(ASTFunction). Skip them here.
	if (AST.getBody() == nullptr) {
		return;
	}

	// Set currents
	CurrentScope = Sema->getSymbols();

	// Enter Body Scope
	EnterScope();

	// Resolve Parameters Types
	for (auto Param : AST.getParams()) {

		// resolve parameter type
		Param->accept(*this);
		SemaParam *ResolvedParam = Param->getSymbol()->getRefAs<SemaParam>();
		Sema->addParam(ResolvedParam);
		addSymbol(Param->getSymbol());
	}

	// Add to Body list for resolve in the next step
	Reg.addBody(Sema);

	// Exit Body Scope
	ExitScope();
}

void Resolver::ResolveClassType(SemaClassType *ClassType) {
	FLY_DEBUG_SCOPE("Resolver", "ResolveClassType");

	// Guard against double-resolution (can happen when a base class appears both
	// in the module node list and via ResolveBaseClasses from a derived class)
	if (ClassType->Resolved) {
		return;
	}
	ClassType->Resolved = true;

	CurrentScope = ClassType->getSymbols();
	CurrentClass = ClassType;

	// Resolve Base Classes (includes kind checks, final check, diamond check)
	this->ResolveBaseClasses(ClassType);

	// Resolve Nodes: Attributes, Methods and Constructors
	for (auto &Node: ClassType->getAST().getNodes()) {
		Node->accept(*this);
	}

	// Create a Default Constructor if not exists (not for interfaces or abstract classes)
	if (CurrentClass->getDefaultConstructor() == nullptr && CurrentClass->getClassKind() != SemaClassKind::INTERFACE) {
		CreateDefaultConstructor();
	}

	// Validate that all inherited abstract methods are implemented (concrete classes only)
	CheckAbstractMethodsImplemented(ClassType);
}

void Resolver::ResolveBaseClasses(SemaClassType *DerivedClass) {
	FLY_DEBUG_SCOPE("Resolver", "ResolveBaseClasses");

	// Check abstract+final conflict on the derived class itself
	if (DerivedClass->isAbstract() && DerivedClass->isFinal()) {
		Diag(DerivedClass->getAST().getLocation(), diag::err_sema_abstract_final_conflict)
			<< DerivedClass->getName();
		return;
	}

	int classBaseCount = 0;

	for (auto AST : DerivedClass->getAST().getBases()) {

		if (AST->getTypeKind() != ASTTypeKind::TYPE_NAMED) {
			Diag(AST->getLocation(), diag::err_sema_base_not_named_type);
			return;
		}

		// Resolve the base symbol
		Symbol *Sym = Reg.LookupNamedType(*static_cast<ASTNamedType *>(AST), CurrentScope);
		if (!Sym) {
			return;
		}
		SemaType *NamedType = static_cast<SemaType *>(Sym->getRef());
		if (!NamedType->isClass()) {
			Diag(AST->getLocation(), diag::err_sema_base_not_class) << NamedType->getName();
			return;
		}

		SemaClassType *BaseClass = static_cast<SemaClassType *>(NamedType);

		// Kind compatibility rules
		SemaClassKind DerivedKind = DerivedClass->getClassKind();
		SemaClassKind BaseKind = BaseClass->getClassKind();

		if (DerivedKind == SemaClassKind::INTERFACE) {
			// Interface can only extend interfaces
			if (BaseKind != SemaClassKind::INTERFACE) {
				Diag(AST->getLocation(), diag::err_sema_interface_extends_class)
					<< DerivedClass->getName() << BaseClass->getName();
				return;
			}
		} else if (DerivedKind == SemaClassKind::STRUCT) {
			// Struct can only extend another struct
			if (BaseKind != SemaClassKind::STRUCT) {
				Diag(AST->getLocation(), diag::err_sema_struct_extends_non_struct)
					<< DerivedClass->getName() << BaseClass->getName();
				return;
			}
			classBaseCount++;
		} else {
			// CLASS: at most one CLASS/STRUCT base, any number of INTERFACE bases
			if (BaseKind == SemaClassKind::CLASS || BaseKind == SemaClassKind::STRUCT) {
				classBaseCount++;
				if (classBaseCount > 1) {
					Diag(AST->getLocation(), diag::err_sema_multiple_class_bases)
						<< DerivedClass->getName();
					return;
				}
			}
			// INTERFACE base for a CLASS is allowed (implements)
		}

		// Cannot extend a final class/struct
		if (BaseClass->isFinal()) {
			Diag(AST->getLocation(), diag::err_sema_final_class_subclassed) << BaseClass->getName();
			return;
		}

		// Resolve the base class type (depth-first)
		ResolveClassType(BaseClass);

		// Add to the derived class base list
		DerivedClass->BaseClasses.push_back(BaseClass);
	}

	// Propagate concrete class method symbols (depth-first) into the derived class
	// symbol table so that inherited methods are resolvable when the caller holds
	// a variable of the derived type (e.g. CLang c = new CLang(); c.open(...)).
	// Only class-kind bases are propagated; interface abstract methods are not.
	llvm::StringSet<> Propagated;
	std::function<void(SemaClassType *)> PropagateInherited = [&](SemaClassType *Base) {
		if (Base->getClassKind() != SemaClassKind::CLASS) return;
		for (auto &Entry : Base->getMethods()) {
			llvm::StringRef Name = Entry.first();
			SemaClassMethod *Method = Entry.second;
			if (!DerivedClass->getMethods().count(Name) && !Propagated.count(Name)) {
				Propagated.insert(Name);
				Symbol *Sym = new Symbol(std::string(Name), SymbolKind::FUNCTION, Method);
				DerivedClass->getSymbols()->insert(Sym);
			}
		}
		for (auto *GrandBase : Base->getBaseClasses())
			PropagateInherited(GrandBase);
	};
	for (auto *Base : DerivedClass->getBaseClasses())
		PropagateInherited(Base);

	// Check diamond ambiguity from interface default methods
	CheckDiamondAmbiguity(DerivedClass);
}

void Resolver::CollectInterfaceDefaultMethods(
		SemaClassType *Interface,
		llvm::StringMap<llvm::SmallVector<SemaClassType *, 2>> &MethodSources) {
	// Recurse into base interfaces first
	for (auto *Base : Interface->getBaseClasses()) {
		if (Base->getClassKind() == SemaClassKind::INTERFACE) {
			CollectInterfaceDefaultMethods(Base, MethodSources);
		}
	}
	// Collect default methods (METHOD kind, not ABSTRACT) defined directly here
	for (auto *Node : Interface->getNodes()) {
		if (Node->getKind() == SemaKind::METHOD) {
			auto *Method = static_cast<SemaClassMethod *>(Node);
			if (!Method->isAbstract() && !Method->isConstructor()) {
				MethodSources[Method->getName()].push_back(Interface);
			}
		}
	}
}

void Resolver::CheckDiamondAmbiguity(SemaClassType *ClassType) {
	// Collect all interface bases (direct and indirect)
	llvm::StringMap<llvm::SmallVector<SemaClassType *, 2>> DefaultMethodSources;

	for (auto *Base : ClassType->getBaseClasses()) {
		if (Base->getClassKind() == SemaClassKind::INTERFACE) {
			CollectInterfaceDefaultMethods(Base, DefaultMethodSources);
		}
	}

	// Check each method name that has more than one source
	for (auto &Entry : DefaultMethodSources) {
		if (Entry.second.size() < 2) continue;

		llvm::StringRef MethodName = Entry.first();

		// Check if the current class directly overrides this method (resolves the ambiguity)
		bool overriddenLocally = false;
		for (auto *Node : ClassType->getNodes()) {
			if (Node->getKind() == SemaKind::METHOD) {
				auto *Method = static_cast<SemaClassMethod *>(Node);
				if (!Method->isConstructor() && Method->getName() == MethodName) {
					overriddenLocally = true;
					break;
				}
			}
		}

		if (!overriddenLocally) {
			// Report ambiguity: take first two conflicting sources
			Diag(ClassType->getAST().getLocation(), diag::err_sema_diamond_ambiguity)
				<< MethodName << Entry.second[0]->getName()
				<< Entry.second[1]->getName() << ClassType->getName();
		}
	}
}

void Resolver::CheckAbstractMethodsImplemented(SemaClassType *ClassType) {
	// Only applies to concrete (non-abstract) classes
	if (ClassType->isAbstract() || ClassType->getClassKind() == SemaClassKind::INTERFACE) return;

	// Collect all abstract methods that need to be implemented
	// Walk all base classes (depth-first) and gather unimplemented abstract methods
	llvm::StringMap<SemaClassType *> RequiredImpls; // method name → interface/abstract class that declared it

	std::function<void(SemaClassType *)> CollectAbstract = [&](SemaClassType *Base) {
		for (auto *Base2 : Base->getBaseClasses()) {
			CollectAbstract(Base2);
		}
		for (auto *Node : Base->getNodes()) {
			if (Node->getKind() == SemaKind::METHOD) {
				auto *Method = static_cast<SemaClassMethod *>(Node);
				if (Method->isAbstract()) {
					RequiredImpls[Method->getName()] = Base;
				}
			}
		}
	};

	for (auto *Base : ClassType->getBaseClasses()) {
		CollectAbstract(Base);
	}

	// Check each required method is implemented in ClassType's own nodes
	for (auto &Entry : RequiredImpls) {
		bool implemented = false;
		for (auto *Node : ClassType->getNodes()) {
			if (Node->getKind() == SemaKind::METHOD) {
				auto *Method = static_cast<SemaClassMethod *>(Node);
				if (!Method->isAbstract() && !Method->isConstructor() && Method->getName() == Entry.first()) {
					implemented = true;
					break;
				}
			}
		}
		if (!implemented) {
			SemaClassType *DeclaredIn = Entry.second;
			if (DeclaredIn->getClassKind() == SemaClassKind::INTERFACE) {
				Diag(ClassType->getAST().getLocation(), diag::err_sema_interface_not_implemented)
					<< ClassType->getName() << Entry.first() << DeclaredIn->getName();
			} else {
				Diag(ClassType->getAST().getLocation(), diag::err_sema_abstract_method_not_implemented)
					<< ClassType->getName() << Entry.first() << DeclaredIn->getName();
			}
		}
	}
}

// bool Resolver::CanInheritMethod(SemaClassMethod *BaseMethod) {
// 	// Add Methods from Super SuperClassType
// 	// Check if Method Visibility is not private and not static
// 	if (BaseMethod->getVisibility() > SemaVisibilityKind::PRIVATE && !BaseMethod->isStatic()) {
//
// 		// Check Methods already exists and type conflicts in Super Methods
// 		auto It = CurrentClass->getMethods().find(BaseMethod->getMangledName());
// 		if (It == CurrentClass->getMethods().end()) { // Not Found, add new Method
// 			return true;
// 		} else { // Duplicate Found, check conflicts
//
// 			// Check Return Type conflicts
// 			if (It->second->getReturnType() != BaseMethod->getReturnType()) {
// 				Diag(It->second->getAST().getLocation(), diag::err_syntax_error);
// 			}
//
// 			// Check Visibility conflicts
// 			if (It->second->getVisibility() < BaseMethod->getVisibility()) {
// 				Diag(It->second->getAST().getLocation(), diag::err_syntax_error);
// 			}
//
// 			// Check Static conflicts
// 			if (It->second->isStatic()) {
// 				Diag(It->second->getAST().getLocation(), diag::err_syntax_error);
// 			}
//
// 			// If the inherited method appear in more than one super class: need to be re-defined
// 			// Check if class methods contains the same inherited method without redefine it
// 			if (CurrentClass->getName() != It->getValue()->getClass()->getName()) {
//
//                 // Error: method already exists in super class
//                 Diag(BaseMethod->getAST().getLocation(), diag::err_syntax_error);
//             }
//
// 			return false;
// 		}
// 	}
// }
//
// bool Resolver::CanInheritAttribute(SemaClassAttribute *BaseAttribute) {
// 	// Check if Attribute Visibility is not private and not static
// 	if (BaseAttribute->getVisibility() > SemaVisibilityKind::PRIVATE && !BaseAttribute->isStatic()) {
//
// 		// Check Attribute already exists and type conflicts in Super Vars
// 		auto It = CurrentClass->Attributes.find(BaseAttribute);
// 		if (It == CurrentClass->Attributes.end()) { // Not Found
// 			return true;
// 		} else { // Duplicate Found
//
// 			// Check Type conflicts
// 			if (!It->second->getType()->isEquals(BaseAttribute->getType())) {
// 				Diag(It->second->getAST()->getLocation(), diag::err_syntax_error);
// 			}
//
// 			// Check Visibility conflicts
// 			if (It->second->getVisibility() < BaseAttribute->getVisibility()) {
// 				Diag(It->second->getAST()->getLocation(), diag::err_syntax_error);
// 			}
//
// 			// Check Constant conflicts
// 			if (!It->second->isConstant() && BaseAttribute->isConstant()) {
// 				Diag(It->second->getAST()->getLocation(), diag::err_syntax_error);
// 			}
//
// 			// Check Static conflicts
// 			if (It->second->isStatic()) {
// 				Diag(It->second->getAST()->getLocation(), diag::err_syntax_error);
// 			}
//
// 			return false;
// 		}
// 	}
// }

void Resolver::CreateDefaultConstructor() {
	FLY_DEBUG_SCOPE("Resolver", "CreateDefaultConstructor");

	// Create Default Modifier
	llvm::SmallVector<ASTModifier *, 8> Modifiers;
	Modifiers.push_back(ASTBuilder::CreateModifier(SourceLocation(), ASTModifierKind::MOD_PUBLIC));

	// Create Class Method
	SemaClassMethod *Sema = SemaBuilder::CreateDefaultConstructor(CurrentClass, CurrentScope);

	// Add Method/Constructor
	CurrentClass->addMethod(Sema); // Function Local var to be allocated

	// Set current function
	// CurrentFunction = Sema;

	// Enter Function Scope
	EnterScope();

	// Create the Symbol and add to Symbol Table of the parent scop
	Symbol *Sym = new Symbol(Sema->getAST().getName(), SymbolKind::FUNCTION, Sema);

	// Exit constructor scope
	ExitScope();

	// Add Symbol to the current scope
	addSymbol(Sym);
}

SmallVector<SemaType *, 8> Resolver::ResolveCallArgs(ASTCall *AST) {
	FLY_DEBUG_SCOPE("Resolver", "ResolveCallArgs");
	SmallVector<SemaType *, 8> Types;

	// Track identifiers to detect duplicate arguments
	llvm::StringSet<> SeenIdentifiers;

	// Store resolved arg expressions in a temporary vector
	ResolvedCallArgs.clear();

	for (auto Arg : AST->getArgs()) {
		Arg->getExpr()->accept(*this);
		SemaExpr *ArgExpr = CurrentExpr;
		Types.push_back(ArgExpr ? ArgExpr->getType() : nullptr);
		ResolvedCallArgs.push_back(ArgExpr);

		// Check for duplicate identifier arguments
		if (Arg->getExpr()->getExprKind() == ASTExprKind::EXPR_IDENTIFIER) {
			ASTIdentifier *Ident = static_cast<ASTIdentifier *>(Arg->getExpr());
			llvm::StringRef Name = Ident->getName();
			if (!SeenIdentifiers.insert(Name).second) {
				Diag(Ident->getLocation(), diag::err_sema_duplicate_arg) << Name;
			}
		}
	}
	return std::move(Types);
}

SmallVector<SemaType *, 8> Resolver::ResolveParams(ASTFunction &AST) {
	FLY_DEBUG_SCOPE("Resolver", "ResolveParams");
	SmallVector<SemaType *, 8> Types;
	for (auto Param : AST.getParams()) {
		Param->accept(*this);
		SemaVar *ParamSema = Param->getSymbol() ?
			Param->getSymbol()->getRefAs<SemaVar>() : nullptr;
		Types.push_back(ParamSema ? ParamSema->getType() : nullptr);
	}
	return std::move(Types);
}

SemaType * Resolver::PromoteNumberTypes(SemaType *Left, SemaType *Right) {
	FLY_DEBUG_SCOPE("Resolver", "PromoteNumberTypes");
	SemaNumberType *LeftNum = static_cast<SemaNumberType *>(Left);
	SemaNumberType *RightNum = static_cast<SemaNumberType *>(Right);
	if (LeftNum->getRank() >= RightNum->getRank()) {
		return Left;
	}
	return Right;
}

void Resolver::PromoteTypes(ASTBinary &AST, SemaExpr *Left, SemaExpr *Right) {
	FLY_DEBUG_SCOPE("Resolver", "PromoteTypes");

	// Guard: if either side failed to resolve, skip promotion
	if (!Left || !Right) {
		return;
	}

	SemaType *LeftType = Left->getType();
	SemaType *RightType = Right->getType();

	// Promote Number Types if both operands are numbers.
	// For assignments, the CodeGen converts the RHS value at emit time.
	// For arithmetic/compare ops, only promote literal/value expressions — never variable
	// references (LOCAL_VAR, PARAM_VAR, etc.) since mutating their types corrupts
	// alloca setup, name mangling, and causes cascading type errors across the function.
	// CodeGen's ConvertNumber handles the actual value promotion at emit time.
	if (!AST.isAssign() && LeftType && RightType && LeftType->isNumber() && RightType->isNumber()) {
		SemaType *PromotedType = PromoteNumberTypes(LeftType, RightType);
		auto IsVarRef = [](SemaExpr *E) {
			SemaKind K = E->getKind();
			return K == SemaKind::LOCAL_VAR || K == SemaKind::PARAM_VAR ||
			       K == SemaKind::ERROR_VAR || K == SemaKind::ATTRIBUTE ||
			       K == SemaKind::INSTANCE_VAR;
		};
		if (!IsVarRef(Left))  Left->setType(PromotedType);
		if (!IsVarRef(Right)) Right->setType(PromotedType);
	}

	// Promote Array Types if both operands are arrays
	if (AST.isAssign() && LeftType && RightType && LeftType->isArray() &&
		RightType->isArray() && Right->getKind() == SemaKind::VALUE) {
		SemaArrayValue *ArrayValue = static_cast<SemaArrayValue *>(Right);
		SemaType *ElementType = static_cast<SemaArrayType *>(LeftType)->getElementType();
		for (auto &Val : ArrayValue->getValues()) {
			Val->setType(ElementType);
		}
		static_cast<SemaArrayType *>(ArrayValue->getType())->setElementType(ElementType);
	}

	// Promote Struct Value Type: set the struct value's type from the left-hand side class type
	if (AST.isAssign() && LeftType && LeftType->isClass() &&
		Right->getKind() == SemaKind::VALUE && !RightType) {
		Right->setType(LeftType);

		// Promote the inner values' types to match the struct attribute types
		SemaClassType *ClassType = static_cast<SemaClassType *>(LeftType);
		SemaStructValue *StructValue = static_cast<SemaStructValue *>(Right);
		for (auto &Entry : StructValue->Values) {
			SemaClassAttribute *Attr = ClassType->LookupAttribute(Entry.getKey());
			if (Attr && Entry.getValue() && Attr->getType()) {
				Entry.getValue()->setType(Attr->getType());
			}
		}
	}
}

SemaExpr * Resolver::ResolveMemberSymbol(ASTMember &AST, SymbolTable *Symbols, SemaKind ExpectedKind, SemaVar *ParentVar) {
	FLY_DEBUG_SCOPE("Resolver", "ResolveMemberSymbol");

	// Save the current scope and switch to the member's scope
	SymbolTable *SavedScope = CurrentScope;
	CurrentScope = Symbols;

	// Lookup Member in Parent Scope
	Symbol *CurrentSymbol = Reg.LookupName(AST.getName(), CurrentScope);
	if (CurrentSymbol) {

		// Store the resolved Symbol on the ASTMember
		AST.setSymbol(CurrentSymbol);

		SemaNode *Node = CurrentSymbol->getRef();

		// Get the Sema Node
		if (Node->getKind() != ExpectedKind) {
			llvm::StringRef KindName = (ExpectedKind == SemaKind::ATTRIBUTE) ? "field" : "enum entry";
			Diag(AST.getLocation(), diag::err_sema_member_type_mismatch) << AST.getName() << KindName;
			CurrentScope = SavedScope;
			return nullptr;
		}

		// Create the appropriate Sema Node for the Member Access
		if (ExpectedKind == SemaKind::ATTRIBUTE) {
			SemaClassAttribute *Attr = static_cast<SemaClassAttribute *>(Node);
			if (Attr->getVisibility() == SemaVisibilityKind::PRIVATE &&
			    CurrentClass != &Attr->getClass()) {
				Diag(AST.getLocation(), diag::err_sema_symbol_not_accessible)
				    << AST.getName() << Attr->getClass().getName();
				CurrentScope = SavedScope;
				return nullptr;
			}
			CurrentScope = SavedScope;
			if (ParentVar) {
				return SemaBuilder::CreateMemberVar(AST, Attr, ParentVar);
			}
			return Attr;
		} else if (ExpectedKind == SemaKind::ENUM_ENTRY) {
			CurrentScope = SavedScope;
			if (ParentVar) {
				return SemaBuilder::CreateMemberVar(AST, static_cast<SemaEnumEntry *>(Node), ParentVar);
			}
			return static_cast<SemaEnumEntry *>(Node);
		}
	}

	CurrentScope = SavedScope;
	return nullptr;
}
