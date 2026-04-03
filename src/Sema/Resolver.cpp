//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/Resolver.cpp - The Resolver
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
#include "Sema/SemaValidator.h"
#include "Sema/SymbolTable.h"

#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringSet.h"

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
	if (DiagID == diag::err_invalid_behavior) {
		llvm::errs() << "DEBUG Resolver::Diag err_invalid_behavior called\n";
	}
	return Diags.Report(DiagID);
}

void Resolver::visit(ASTModule &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTModule)");
	if (!AST.isVisited()) {
		AST.setVisited(true);

		EnterScope();

		CurrentModule = new SemaModule(AST, CurrentScope);
		Reg.addModule(CurrentModule);

		for (size_t i = 0; i < AST.getNodes().size(); ++i) {
			auto Node = AST.getNodes()[i];

			if (i == 0) {
				if (Node->getKind() != ASTKind::AST_NAMESPACE) {
					// Set the Module NameSpace
					CurrentNameSpace = Reg.getDefaultNameSpace();
				}

				// Visit Definition
				Node->accept(*this);

			} else {

				// Visit Definition
				Node->accept(*this);
			}
		}

		ExitScope();
	}
	FLY_DEBUG_END("Resolver", "visit(ASTModule)");
}

void Resolver::visit(ASTNameSpace &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTNameSpace)");
	if (!AST.isVisited()) {
		AST.setVisited(true);

		// Build the CurrentNameSpace
		SemaNameSpace *NameSpace = Reg.getOrCreateNameSpace(AST.getNames());

		// Set Symbol Table
		CurrentModule->setNameSpace(NameSpace);
		CurrentNameSpace = NameSpace;
	}
	FLY_DEBUG_END("Resolver", "visit(ASTNameSpace)");
}

void Resolver::visit(ASTImport &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTImport)");
	if (!AST.isVisited()) {
		AST.setVisited(true);

		Validator->CheckImport(AST);

		// Add import in Module
		SemaBuilder::CreateImport(*CurrentModule, AST);
	}
	FLY_DEBUG_END("Resolver", "visit(ASTImport)");
}

void Resolver::visit(ASTFunction &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTFunction)");
	ResetCurrents();

	// Enter Function Scope
	EnterScope();

	// Create Sema Function
	CurrentFunction = SemaBuilder::CreateFunction(*CurrentModule, CurrentScope, AST);

	// Add to Symbol Table of Parent Scope (Module or Class)
	Symbol *Sym = new Symbol(AST.getName(), SymbolKind::FUNCTION, CurrentFunction);

	// Exit Function Scope
	ExitScope();

	// Add Symbol to the current scope
	addSymbol(Sym);

	FLY_DEBUG_END("Resolver", "visit(ASTFunction)");
}

void Resolver::visit(ASTClass &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTClass)");
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
	}

	FLY_DEBUG_END("Resolver", "visit(ASTClass)");
}

void Resolver::visit(ASTAttribute &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTAttribute)");

	// Find Var duplication
	SemaClassAttribute *ExistingAttr = CurrentClass->LookupAttribute(AST.getName());
	if (ExistingAttr) {
		Diag(AST.getLocation(), diag::err_sema_var_redefinition) << AST.getName();
		FLY_DEBUG_END("Resolver", "visit(ASTAttribute)");
		return;
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
	}

	// Create the Symbol and add to Symbol Table
	Symbol *Sym = new Symbol(AST.getName(), SymbolKind::ATTRIBUTE, Sema);
	AST.setSymbol(Sym);
	addSymbol(Sym);

	FLY_DEBUG_END("Resolver", "visit(ASTAttribute)");
}

void Resolver::visit(ASTMethod &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTMethod)");

	// Enter Method Scope
	EnterScope();

	// Methods are implicitly void - no return type to resolve
	SmallVector<SemaType *, 8> Types = ResolveParams(AST);

	// Find Method duplication
	if (Reg.LookupFunction(AST.getName(), Types, CurrentClass->getSymbols())) {
		Diag(AST.getLocation(), diag::err_sema_var_redefinition) << AST.getName();
		FLY_DEBUG_END("Resolver", "visit(ASTMethod)");
		return;
	}

	// Create Class Method
	SemaClassMethod *Sema = SemaBuilder::CreateClassMethod(CurrentClass, AST, CurrentScope);
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

	// Add to Body list for resolve in the next step
	Reg.addBody(Sema);

	// Create the Symbol and add to Symbol Table of the parent scop
	Symbol *Sym = new Symbol(AST.getName(), SymbolKind::FUNCTION, Sema);

	// Exit Method Scope
	ExitScope();

	// Add Symbol to the current scope
	addSymbol(Sym);

	FLY_DEBUG_END("Resolver", "visit(ASTMethod)");
}

void Resolver::visit(ASTEnum &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTEnum)");
	ResetCurrents();

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

	FLY_DEBUG_END("Resolver", "visit(ASTEnum)");
}

void Resolver::visit(ASTEnumEntry &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTEnumEntry)");

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

	FLY_DEBUG_END("Resolver", "visit(ASTEnumEntry)");
}

void Resolver::visit(ASTLocalVar &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTLocalVar)");
	if (!AST.isVisited()) {
		AST.setVisited(true);

		// Resolve Type
		AST.getType()->accept(*this);
		SemaType *Type = CurrentType;

		// Create Sema Local Var
		SemaLocalVar *Sema = SemaBuilder::CreateLocalVar(AST, Type);

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
	FLY_DEBUG_END("Resolver", "visit(ASTLocalVar)");
}

void Resolver::visit(ASTParam &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTParam)");
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
	FLY_DEBUG_END("Resolver", "visit(ASTParam)");
}

void Resolver::visit(ASTComment &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTComment)");
	CurrentComment = SemaBuilder::CreateComment(AST);
	FLY_DEBUG_END("Resolver", "visit(ASTComment)");
}

void Resolver::visit(ASTBuiltinType &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTBuiltinType)");
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
		case ASTBuiltinTypeKind::TYPE_STRING:
			Sema = SemaBuiltin::getStringType();
			break;
		case ASTBuiltinTypeKind::TYPE_ERROR:
			Sema = SemaBuiltin::getErrorType();
			break;
	}
	CurrentType = Sema;
	FLY_DEBUG_END("Resolver", "visit(ASTBuiltinType)");
}

void Resolver::visit(ASTNamedType &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTNamedType)");
	if (!AST.isVisited()) {
		AST.setVisited(true);
	}

	SymbolTable *Scope = CurrentScope;
	Symbol *Sym = Reg.LookupNamedType(AST, Scope);
	SemaType *Sema = static_cast<SemaType *>(Sym->getRef());

	CurrentType = Sema;
	FLY_DEBUG_END("Resolver", "visit(ASTNamedType)");
}

void Resolver::visit(ASTArrayType &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTArrayType)");
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
	FLY_DEBUG_END("Resolver", "visit(ASTArrayType)");
}

void Resolver::visit(ASTExprStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTExprStmt)");
	CurrentStmt = &AST;
	ASTExpr *Expr = AST.getExpr();

	Expr->accept(*this);
	SemaExpr *ResolvedExpr = CurrentExpr;

	// Create SemaExprStmt and add to current block
	if (CurrentSemaBlock && ResolvedExpr) {
		SemaExprStmt *SemaStmt = SemaBuilder::CreateExprStmt(&AST, ResolvedExpr);
		CurrentSemaBlock->addContent(SemaStmt);
	}
	FLY_DEBUG_END("Resolver", "visit(ASTExprStmt)");
}

void Resolver::visit(ASTDeclStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTDeclStmt)");
	CurrentStmt = &AST;
	ASTLocalVar *LocalVar = AST.getLocalVar();

	// Resolve LocalVar Type
	LocalVar->accept(*this);
	SemaLocalVar *SemaLV = LocalVar->getSymbol() ?
		LocalVar->getSymbol()->getRefAs<SemaLocalVar>() : nullptr;

	// Resolve Initialization Expression
	SemaExpr *InitExpr = nullptr;
	if (AST.getExpr()) {
		AST.getExpr()->accept(*this);
		InitExpr = CurrentExpr;
	}

	// Check for array without size expression or initialization expression
	if (SemaLV && SemaLV->getType() && SemaLV->getType()->isArray()) {
		SemaArrayType *ArrayType = static_cast<SemaArrayType *>(SemaLV->getType());
		if (ArrayType->getSizeExpr() == nullptr && AST.getExpr() == nullptr) {
			Diag(LocalVar->getLocation(), diag::err_sema_array_size_missing);
		}
	}

	// Create SemaDeclStmt and add to current block
	if (CurrentSemaBlock) {
		SemaDeclStmt *SemaStmt = SemaBuilder::CreateDeclStmt(&AST, SemaLV, InitExpr);
		CurrentSemaBlock->addContent(SemaStmt);
	}

	FLY_DEBUG_END("Resolver", "visit(ASTDeclStmt)");
}

void Resolver::visit(ASTFailStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTFailStmt)");
	CurrentStmt = &AST;

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

	FLY_DEBUG_END("Resolver", "visit(ASTFailStmt)");
}

void Resolver::visit(ASTHandleStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTHandleStmt)");
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

	FLY_DEBUG_END("Resolver", "visit(ASTHandleStmt)");
}

void Resolver::visit(ASTReturnStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTReturnStmt)");
	CurrentStmt = &AST;

	if (CurrentSemaBlock) {
		SemaReturnStmt *SemaStmt = SemaBuilder::CreateReturnStmt(&AST);
		CurrentSemaBlock->addContent(SemaStmt);
	}

	FLY_DEBUG_END("Resolver", "visit(ASTReturnStmt)");
}

void Resolver::visit(ASTDeleteStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTDeleteStmt)");
	CurrentStmt = &AST;
	ASTExpr * Expr = AST.getExpr();

	Expr->accept(*this);
	SemaExpr *ResolvedExpr = CurrentExpr;

	// Create SemaDeleteStmt and add to current block
	if (CurrentSemaBlock && ResolvedExpr) {
		SemaDeleteStmt *SemaStmt = SemaBuilder::CreateDeleteStmt(&AST, ResolvedExpr);
		CurrentSemaBlock->addContent(SemaStmt);
	}

	FLY_DEBUG_END("Resolver", "visit(ASTDeleteStmt)");
}


void Resolver::visit(ASTBreakStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTBreakStmt)");
	CurrentStmt = &AST;

	// Create SemaBreakStmt and add to current block
	if (CurrentSemaBlock) {
		SemaBreakStmt *SemaStmt = SemaBuilder::CreateBreakStmt(&AST);
		CurrentSemaBlock->addContent(SemaStmt);
	}

	FLY_DEBUG_END("Resolver", "visit(ASTBreakStmt)");
}

void Resolver::visit(ASTContinueStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTContinueStmt)");
	CurrentStmt = &AST;

	// Create SemaContinueStmt and add to current block
	if (CurrentSemaBlock) {
		SemaContinueStmt *SemaStmt = SemaBuilder::CreateContinueStmt(&AST);
		CurrentSemaBlock->addContent(SemaStmt);
	}

	FLY_DEBUG_END("Resolver", "visit(ASTContinueStmt)");
}

void Resolver::visit(ASTBlockStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTBlockStmt)");
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

	// Resolve Statements
	for (ASTStmt *Stmt : AST.getContent()) {
		Stmt->accept(*this);
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

	FLY_DEBUG_END("Resolver", "visit(ASTBlockStmt)");
}

void Resolver::visit(ASTRuleStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTRuleStmt)");
	CurrentStmt = &AST;
	AST.getExpr()->accept(*this);
	AST.getStmt()->accept(*this);
	FLY_DEBUG_END("Resolver", "visit(ASTRuleStmt)");
}

void Resolver::visit(ASTIfStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTIfStmt)");
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

	FLY_DEBUG_END("Resolver", "visit(ASTIfStmt)");
}

void Resolver::visit(ASTSwitchStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTSwitchStmt)");
	CurrentStmt = &AST;

	// Switch Variable
	AST.getExpr()->accept(*this);
	SemaExpr *SwitchExpr = CurrentExpr;
	SemaType * CaseType = SwitchExpr ? SwitchExpr->getType() : nullptr;

	// Create SemaSwitchStmt
	SemaSwitchStmt *SemaSwitch = SemaBuilder::CreateSwitchStmt(&AST, SwitchExpr);

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

	// Add to current block
	if (CurrentSemaBlock) {
		CurrentSemaBlock->addContent(SemaSwitch);
	}

	FLY_DEBUG_END("Resolver", "visit(ASTSwitchStmt)");
}

void Resolver::visit(ASTLoopStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTLoopStmt)");
	CurrentStmt = &AST;

	// Create SemaLoopStmt
	SemaLoopStmt *SemaLoop = SemaBuilder::CreateLoopStmt(&AST);

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

	// Add to current block
	if (CurrentSemaBlock) {
		CurrentSemaBlock->addContent(SemaLoop);
	}

	FLY_DEBUG_END("Resolver", "visit(ASTLoopStmt)");
}

void Resolver::visit(ASTLoopInStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTLoopInStmt)");
	CurrentStmt = &AST;

	AST.getItem()->accept(*this);
	SemaExpr *ItemExpr = CurrentExpr;
    AST.getList()->accept(*this);
	SemaExpr *ListExpr = CurrentExpr;

	// Loop Statement
	AST.getStmt()->accept(*this);

	// Create SemaLoopInStmt and add to block
	SemaLoopInStmt *SemaLoopIn = SemaBuilder::CreateLoopInStmt(&AST, ItemExpr, ListExpr, nullptr);
	if (CurrentSemaBlock) {
		CurrentSemaBlock->addContent(SemaLoopIn);
	}

	FLY_DEBUG_END("Resolver", "visit(ASTLoopInStmt)");
}

void Resolver::visit(ASTIdentifier &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTIdentifier)");

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
			FLY_DEBUG_END("Resolver", "ResolveParent(ASTIdentifier)");
			return;
		}

		// Always store the resolved Symbol on the AST
		AST.setSymbol(CurrentSymbol);

		// Sym Found as Variable - set CurrentExpr
		if (CurrentSymbol->isVarKind()) {
			SemaVar *Sema = static_cast<SemaVar *>(CurrentSymbol->getRef());
			CurrentExpr = Sema;
		}
	}

	FLY_DEBUG_END("Resolver", "visit(ASTIdentifier)");
}

void Resolver::visit(ASTMember &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTMember)");

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

		// check namespace
		if (ParentSymbol->getKind() == SymbolKind::NAMESPACE) {
			CurrentScope = static_cast<SemaNameSpace *>(ParentSymbol->getRef())->getSymbols();
			Diag(diag::err_invalid_behavior); // Member access on namespace is not allowed because global var not exists
			return;
		}

		if (ParentSymbol->getKind() == SymbolKind::CLASS) {
			SemaClassType *ParentSema = static_cast<SemaClassType *>(ParentSymbol->getRef());
			Sema = ResolveMemberSymbol(AST, ParentSema->getSymbols(), SemaKind::ATTRIBUTE);
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
				Sema = ResolveMemberSymbol(AST, ClassType->getSymbols(), SemaKind::ATTRIBUTE, ParentVar);
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

	FLY_DEBUG_END("Resolver", "visit(ASTMember)");
}

void Resolver::visit(ASTCall &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTCall)");

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
			// Lookup Function or Method
			Symbol *CurrentSymbol = Reg.LookupFunction(AST.getName(), ArgTypes, CurrentScope);

			if (!CurrentSymbol) {
				CurrentScope = SavedScope;
				return;
			}

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

	FLY_DEBUG_END("Resolver", "visit(ASTCall)");
}

void Resolver::visit(ASTUnary &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTUnaryOp)");

	// Resolve Expr
	ASTExpr *Expr = AST.getExpr();
	Expr->accept(*this);
	SemaExpr *ResolvedExpr = CurrentExpr;

	// Create Sema
	SemaUnary *Sema = SemaBuilder::CreateUnary(AST, ResolvedExpr);
	CurrentExpr = Sema;

	FLY_DEBUG_END("Resolver", "visit(ASTUnaryOp)");
}

void Resolver::visit(ASTBinary &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTBinaryOp)");

	// Resolve Left and Right Expr
	AST.getLeftExpr()->accept(*this);
	SemaExpr *Left = CurrentExpr;
    AST.getRightExpr()->accept(*this);
	SemaExpr *Right = CurrentExpr;


	// Promote Types if needed
	PromoteTypes(AST, Left, Right);

	if (Validator->CheckBinary(AST, Left, Right)) {

		// Create Sema
		SemaBinary *Sema = SemaBuilder::CreateBinary(AST, Left, Right);
		CurrentExpr = Sema;
	}

	FLY_DEBUG_END("Resolver", "visit(ASTBinaryOp)");
}

void Resolver::visit(ASTTernary &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTTernaryOp)");

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

	FLY_DEBUG_END("Resolver", "visit(ASTTernaryOp)");
}

void Resolver::visit(ASTCast &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTCast)");

	// Resolve ToType and Expr
	AST.getToType()->accept(*this);
    AST.getExpr()->accept(*this);
	// TODO: Validate Cast
	FLY_DEBUG_END("Resolver", "visit(ASTCast)");
}

void Resolver::visit(ASTBoolValue &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTBoolValue)");
	SemaBoolValue *Sema = SemaBuilder::CreateBoolValue(AST);
	CurrentExpr = Sema;
	FLY_DEBUG_END("Resolver", "visit(ASTBoolValue)");
}

void Resolver::visit(ASTNumberValue &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTNumberValue)");

	// The result type of the number value depends on the left side of an assignment
	SemaValue *Sema = SemaBuilder::CreateNumberValue(AST);
	CurrentExpr = Sema;

	FLY_DEBUG_END("Resolver", "visit(ASTNumberValue)");
}

void Resolver::visit(ASTStringValue &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTStringValue)");
	SemaValue *Sema = SemaBuilder::CreateStringValue(AST);
	CurrentExpr = Sema;
	FLY_DEBUG_END("Resolver", "visit(ASTStringValue)");
}

void Resolver::visit(ASTArrayValue &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTArrayValue)");

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

	FLY_DEBUG_END("Resolver", "visit(ASTArrayValue)");
}

void Resolver::visit(ASTStructValue &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTStructValue)");

	llvm::StringMap<SemaValue *> Values;
	for (auto &Entry : AST.getValues()) {
		Entry.second->accept(*this);
		Values.insert(std::make_pair(Entry.getKey(), static_cast<SemaValue *>(CurrentExpr)));
	}

	SemaStructValue *Sema = SemaBuilder::CreateStructValue(AST, Values);
	CurrentExpr = Sema;
	FLY_DEBUG_END("Resolver", "visit(ASTStructValue)");
}

void Resolver::visit(ASTNullValue &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTNullValue)");
	SemaValue *Sema = SemaBuilder::CreateNullValue(AST);
	CurrentExpr = Sema;
	FLY_DEBUG_END("Resolver", "visit(ASTNullValue)");
}

void Resolver::visit(ASTUnsetValue &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTUnsetValue)");
	SemaValue *Sema = SemaBuilder::CreateUnsetValue(AST);
	CurrentExpr = Sema;
	FLY_DEBUG_END("Resolver", "visit(ASTUnsetValue)");
}

void Resolver::Resolver::EnterScope() {
	FLY_DEBUG_START("Resolver", "EnterScope");
	CurrentScope = CurrentScope->pushScope();
	FLY_DEBUG_END("Resolver", "EnterScope");
}

void Resolver::Resolver::ExitScope() {
	FLY_DEBUG_START("Resolver", "ExitScope");
	CurrentScope = CurrentScope->getParent();
	FLY_DEBUG_END("Resolver", "ExitScope");
}

void Resolver::addSymbol(Symbol *Sym) {
	FLY_DEBUG_START("Resolver", "addSymbol");
	CurrentScope->insert(Sym);
	FLY_DEBUG_END("Resolver", "addSymbol");
}

void Resolver::ResetCurrents() {
	FLY_DEBUG_START("Resolver", "ResetCurrent");
	CurrentClass = nullptr;
	CurrentEnum = nullptr;
	CurrentFunction = nullptr;
	CurrentComment =nullptr;
	CurrentStmt = nullptr;
	FLY_DEBUG_END("Resolver", "ResetCurrent");
}

Resolver::~Resolver() {
}

void Resolver::Resolve() {
	FLY_DEBUG_START("Resolver", "Resolve");
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

		// Function/Method Scope
		CurrentScope = FunctionBase->getSymbols();

		// For non-static class methods, add 'this' to the method scope so the body can reference it
		if (FunctionBase->getKind() == SemaKind::METHOD) {
			SemaClassMethod *Method = static_cast<SemaClassMethod *>(FunctionBase);
			if (!Method->isStatic()) {
				Symbol *ThisSym = new Symbol(std::string("this"), SymbolKind::VAR, Method->getThis());
				CurrentScope->insert(ThisSym);
			}
		}

		// Enter Body Scope (already created in the first pass, just need to set it as current scope)
		CurrentScope = CurrentScope->getChildren()[0];

		// Reset CurrentSemaBlock for the new function body
		CurrentSemaBlock = nullptr;

		// Resolve Body - visit(ASTBlockStmt) will create SemaBlockStmt and set as function body
		ASTBlockStmt *Body = FunctionBase->getAST().getBody();
		Body->accept(*this);

		ExitScope();
	}
	FLY_DEBUG_END("Resolver", "Resolve");
}

void Resolver::ResolveImports(SemaModule *Module) {
	FLY_DEBUG_START("Resolver", "ResolveImports");

	for (auto Import : Module->getImports()) {

		// Resolve the target NameSpace, Class or Enum
		Symbol *ImportedSymbol = Reg.LookupImport(Import->getTarget());

		// Check if namespace was found
		if (!ImportedSymbol) {
			std::string TargetName = Helper::Flatten(Import->getTarget());
			Diag(Import->getAST()->getLocation(), diag::err_codegen_unref_type) << TargetName;
			continue;
		}

		// Add Symbol to the Module current scope
		Module->getSymbols()->insert(ImportedSymbol);
	}
	FLY_DEBUG_END("Resolver", "ResolveImports");
}

/**
 * Resolve Module Function Definitions
 */
void Resolver::ResolveFunction(SemaFunction *Sema) {
	FLY_DEBUG_START("Resolver", "ResolveFunction");
	ASTFunction &AST = Sema->getAST();

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

	FLY_DEBUG_END("Resolver", "ResolveFunction");
}

void Resolver::ResolveClassType(SemaClassType *ClassType) {
	FLY_DEBUG_START("Resolver", "ResolveClassType");
	CurrentScope = ClassType->getSymbols();
	CurrentClass = ClassType;

	// Resolve Base Classes
	// this->ResolveBaseClasses(ClassType);

	// Resolve Nodes: Attributes, Methods and Constructors
	for (auto &Node: ClassType->getAST().getNodes()) {
		Node->accept(*this);
	}

	// Create a Default Constructor if not exists
	if (CurrentClass->getDefaultConstructor() == nullptr && CurrentClass->getClassKind() != SemaClassKind::INTERFACE) {
		CreateDefaultConstructor();
	}
	FLY_DEBUG_END("Resolver", "ResolveClassType");
}

void Resolver::ResolveBaseClasses(SemaClassType *DerivedClass) {
	FLY_DEBUG_START("Resolver", "ResolveBaseClasses");
	// ClassDefinition Base Classes on first pass
	for (auto AST : DerivedClass->getAST().getBases()) {

		if (AST->getTypeKind() != ASTTypeKind::TYPE_NAMED) {
			// Error: cannot extend a type which differ from Class
			Diag(AST->getLocation(), diag::err_sema_base_not_named_type);
			FLY_DEBUG_END("Resolver", "ResolveBaseClasses");
			return;
		}

		// Resolve the Base Sema
		Symbol *Sym = Reg.LookupNamedType(*static_cast<ASTNamedType *>(AST), CurrentScope);
		SemaType *NamedType = static_cast<SemaType *>(Sym->getRef());
		if (!NamedType->isClass()) {
			// Error: cannot extend a type which differ from Class
			Diag(AST->getLocation(), diag::err_sema_base_not_class) << NamedType->getName();
			FLY_DEBUG_END("Resolver", "ResolveBaseClasses");
			return;
		}

		// Resolve the Base Class Type
		SemaClassType *BaseClass = static_cast<SemaClassType *>(NamedType);
		ResolveClassType(BaseClass);

		// FIXME insert attribute in Derived Class?

		// Add Base Class to the list
		DerivedClass->BaseClasses.push_back(BaseClass);
	}
	FLY_DEBUG_END("Resolver", "ResolveBaseClasses");
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
	FLY_DEBUG_START("Resolver", "CreateDefaultConstructor");

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

	FLY_DEBUG_END("Resolver", "CreateDefaultConstructor");
}

SmallVector<SemaType *, 8> Resolver::ResolveCallArgs(ASTCall *AST) {
	FLY_DEBUG_START("Resolver", "ResolveCallArgs");
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
	FLY_DEBUG_END("Resolver", "ResolveCallArgs");
	return std::move(Types);
}

SmallVector<SemaType *, 8> Resolver::ResolveParams(ASTFunction &AST) {
	FLY_DEBUG_START("Resolver", "ResolveParams");
	SmallVector<SemaType *, 8> Types;
	for (auto Param : AST.getParams()) {
		Param->accept(*this);
		SemaVar *ParamSema = Param->getSymbol() ?
			Param->getSymbol()->getRefAs<SemaVar>() : nullptr;
		Types.push_back(ParamSema ? ParamSema->getType() : nullptr);
	}
	FLY_DEBUG_END("Resolver", "ResolveParams");
	return std::move(Types);
}

SemaType * Resolver::PromoteNumberTypes(SemaType *Left, SemaType *Right) {
	FLY_DEBUG_START("Resolver", "PromoteNumberTypes");
	SemaNumberType *LeftNum = static_cast<SemaNumberType *>(Left);
	SemaNumberType *RightNum = static_cast<SemaNumberType *>(Right);
	if (LeftNum->getRank() >= RightNum->getRank()) {
		FLY_DEBUG_END("Resolver", "PromoteNumberTypes");
		return Left;
	}
	FLY_DEBUG_END("Resolver", "PromoteNumberTypes");
	return Right;
}

void Resolver::PromoteTypes(ASTBinary &AST, SemaExpr *Left, SemaExpr *Right) {
	FLY_DEBUG_START("Resolver", "PromoteTypes");

	// Guard: if either side failed to resolve, skip promotion
	if (!Left || !Right) {
		FLY_DEBUG_END("Resolver", "PromoteTypes");
		return;
	}

	SemaType *LeftType = Left->getType();
	SemaType *RightType = Right->getType();

	// Promote Number Types if both operands are numbers
	if (LeftType && RightType && LeftType->isNumber() && RightType->isNumber()) {
		SemaType *PromotedType = PromoteNumberTypes(LeftType, RightType);
		Left->setType(PromotedType);
		Right->setType(PromotedType);
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

	FLY_DEBUG_END("Resolver", "PromoteTypes");
}

SemaExpr * Resolver::ResolveMemberSymbol(ASTMember &AST, SymbolTable *Symbols, SemaKind ExpectedKind, SemaVar *ParentVar) {
	FLY_DEBUG_START("Resolver", "ResolveMemberSymbol");

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
			Diag(diag::err_sema_generic); // Member access type mismatch
			CurrentScope = SavedScope;
			return nullptr;
		}

		// Create the appropriate Sema Node for the Member Access
		if (ExpectedKind == SemaKind::ATTRIBUTE) {
			CurrentScope = SavedScope;
			if (ParentVar) {
				return SemaBuilder::CreateMemberVar(AST, static_cast<SemaClassAttribute *>(Node), ParentVar);
			}
			return static_cast<SemaClassAttribute *>(Node);
		} else if (ExpectedKind == SemaKind::ENUM_ENTRY) {
			CurrentScope = SavedScope;
			if (ParentVar) {
				return SemaBuilder::CreateMemberVar(AST, static_cast<SemaEnumEntry *>(Node), ParentVar);
			}
			return static_cast<SemaEnumEntry *>(Node);
		}
	}

	CurrentScope = SavedScope;
	FLY_DEBUG_END("Resolver", "ResolveMemberSymbol");
	return nullptr;
}
