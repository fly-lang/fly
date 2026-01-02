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
#include "AST/ASTOp.h"
#include "AST/ASTReturnStmt.h"
#include "AST/ASTSwitchStmt.h"
#include "AST/ASTType.h"
#include "AST/ASTValue.h"
#include "Basic/Debug.h"
#include "Basic/Diagnostic.h"
#include "CodeGen/CodeGen.h"
#include "Sema/SemaBuilder.h"
#include "Sema/SemaBuiltin.h"
#include "Sema/SemaClassAttribute.h"
#include "Sema/SemaClassMethod.h"
#include "Sema/SemaClassType.h"
#include "Sema/SemaEnumType.h"
#include "Sema/SemaImport.h"
#include "Sema/SemaMemberVar.h"
#include "Sema/SemaModule.h"
#include "Sema/SemaNameSpace.h"
#include "Sema/SemaValidator.h"
#include "Sema/SymbolTable.h"

#include "llvm/ADT/StringMap.h"

#include <AST/ASTCast.h>
#include <AST/ASTDeclStmt.h>
#include <AST/ASTParam.h>
#include <Sema/Helper.h>
#include <Sema/Registry.h>
#include <Sema/SemaCall.h>
#include <Sema/SemaEnumEntry.h>
#include <Sema/SemaFunction.h>
#include <Sema/SemaLocalVar.h>
#include <Sema/SemaParam.h>
#include <Sema/SemaValue.h>
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
	return Diags.Report(DiagID);
}

void Resolver::visit(ASTModule &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTModule)");
	if (!AST.isVisited()) {
		AST.setVisited(true);

		CurrentModule = new SemaModule(AST);
		Reg.addModule(CurrentModule);

		for (size_t i = 0; i < AST.getNodes().size(); ++i) {
			auto Node = AST.getNodes()[i];

			if (i == 0) {
				if (Node->getKind() != ASTKind::AST_NAMESPACE) {
					// Set the Module NameSpace
					CurrentNameSpace = Reg.getDefaultNameSpace();
				}

				// Set Symbol Table
				CurrentScope = CurrentNameSpace->getSymbols();
				EnterScope();

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
		CurrentScope = CurrentNameSpace->getSymbols();
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
	ResetCurrent();

	// Enter Function Scope
	EnterScope();

	// Create Sema Function
	CurrentFunction = SemaBuilder::CreateFunction(*CurrentModule, CurrentScope, AST);

	// // Add to Symbol Table of Parent Scope (Module or Class)
	Symbol *Sym = new Symbol(AST.getName(), CurrentFunction->getKind(), CurrentFunction);

	// Exit Function Scope
	ExitScope();

	// Add Symbol to the current scope
	addSymbol(Sym);

	FLY_DEBUG_END("Resolver", "visit(ASTFunction)");
}

void Resolver::visit(ASTClass &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTClass)");
	ResetCurrent();

	// Enter Class Scope
	EnterScope();

	// Create Sema Class
	CurrentClass = SemaBuilder::CreateClass(*CurrentModule, CurrentScope, AST);

	// Create Symbol
	Symbol *Sym = new Symbol(AST.getName(), SemaKind::CLASS, CurrentClass);

	// Note: Nodes will be added in the next Resolve Steps

	// Exit Class Scope
	ExitScope();

	// Add Symbol to the current scope
	addSymbol(Sym);

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
	SemaType * Type = AST.getType()->getSema();

	// Create Class Attribute
	SemaClassAttribute *Sema = SemaBuilder::CreateClassAttribute(*CurrentClass, AST, Type);
	CurrentClass->addAttribute(Sema); // Function Local var to be allocated

	// Set Expr or Default Value
	if (AST.getExpr()) {
		AST.getExpr()->accept(*this);
	} else {
		// Create default Sema Value
		SemaValue *Sema = SemaBuilder::CreateDefaultValue(*Type);
		ASTValue *Value = ASTBuilder::CreateDefaultValue();
		Value->setSema(Sema);
		AST.setExpr(Value);
	}

	// Add to Symbol Table
	Symbol *Sym = new Symbol(AST.getName(), Sema->getKind(), Sema);

	// Add Symbol to the current scope
	addSymbol(Sym);

	FLY_DEBUG_END("Resolver", "visit(ASTAttribute)");
}

void Resolver::visit(ASTMethod &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTMethod)");

	// Resolve Return Type add Params Type
	AST.getReturnType()->accept(*this);
	ResolveParams(AST);

	// Create Mangled Name
	std::string MangledName = Helper::Mangle(&AST);

	// Find Method duplication
	SemaClassMethod *ExistingMethod = CurrentClass->LookupMethod(MangledName);
	if (ExistingMethod) {
		Diag(AST.getLocation(), diag::err_sema_var_redefinition) << AST.getName();
		FLY_DEBUG_END("Resolver", "visit(ASTMethod)");
		return;
	}

	// Create Class Method
	SemaClassMethod *Sema = SemaBuilder::CreateClassMethod(CurrentClass, AST, MangledName);
	CurrentClass->addMethod(Sema); // Function Local var to be allocated
	CurrentFunction = Sema;

	// Add to Symbol Table
	Symbol *Sym = new Symbol(AST.getName(), Sema->getKind(), Sema);

	// Enter Method Scope
	EnterScope();

	// Add to Body list for resolve in the next step
	// Enter Body Scope
	EnterScope();
	Reg.addBody(CurrentScope, AST.getBody());
	ExitScope();

	// Exit Method Scope
	ExitScope();

	// Add Symbol to the current scope
	addSymbol(Sym);

	FLY_DEBUG_END("Resolver", "visit(ASTMethod)");
}

void Resolver::visit(ASTEnum &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTEnum)");
	ResetCurrent();

	// Enter Enum Scope
	EnterScope();

	// Create Sema Enum
	CurrentEnum = SemaBuilder::CreateEnum(*CurrentModule, CurrentScope, AST);

	// Create Symbol
	Symbol *Sym = new Symbol(AST.getName(), SemaKind::ENUM, CurrentEnum);

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
	Symbol *Sym = new Symbol(AST.getName(), SemaKind::ENUM_ENTRY, Sema);

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

		// Create Sema Local Var
		SemaLocalVar *Sema = SemaBuilder::CreateLocalVar(AST, AST.getType()->getSema());

		// Add LocalVar to the Function Base LocalVars
		CurrentFunction->addLocalVar(Sema);

		// Find Var duplication in the current scope
		// TODO: check also in parent scopes for shadowing?
		Symbol *ExistingSym = CurrentScope->lookup(AST.getName());
		if (ExistingSym) {
			Diag(AST.getLocation(), diag::err_sema_var_redefinition) << AST.getName();
		}

		// Add to Symbol Table
		Symbol *Sym = new Symbol(AST.getName(), Sema->getKind(), Sema);

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
		SemaType *Type = AST.getType()->getSema();

		// Create Sema Param
		SemaParam * Sema = SemaBuilder::CreateParam(AST, Type);

		AST.setSema(Sema);
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
		AST.setSema(Sema);
	}
	FLY_DEBUG_END("Resolver", "visit(ASTBuiltinType)");
}

void Resolver::visit(ASTNamedType &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTNamedType)");
	if (!AST.isVisited()) {
		AST.setVisited(true);

		SemaType * Sema = Reg.LookupNamedType(AST, CurrentNameSpace);
		AST.setSema(Sema);
	}
	FLY_DEBUG_END("Resolver", "visit(ASTNamedType)");
}

void Resolver::visit(ASTArrayType &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTArrayType)");
	if (!AST.isVisited()) {
		AST.setVisited(true);

		// Resolve Element Type
		ASTType * ElementType = AST.getElementType();
		ElementType->accept(*this);

		// Resolve Size Expression
		ASTExpr * SizeExpr = AST.getSizeExpr();
		SizeExpr->accept(*this);

		// Create Sema Array Type
		SemaType *Sema = SemaBuiltin::CreateArrayType(ElementType->getSema(), SizeExpr);
		AST.setSema(Sema);
	}
	FLY_DEBUG_END("Resolver", "visit(ASTArrayType)");
}

void Resolver::visit(ASTExprStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTExprStmt)");
	CurrentStmt = &AST;
	ASTExpr *Expr = AST.getExpr();

	Expr->accept(*this);
	FLY_DEBUG_END("Resolver", "visit(ASTExprStmt)");
}

void Resolver::visit(ASTDeclStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTDeclStmt)");
	CurrentStmt = &AST;
	ASTLocalVar *LocalVar = AST.getLocalVar();

	// Resolve LocalVar Type
	LocalVar->accept(*this);
	SemaType * Type = LocalVar->getType()->getSema();

	// Create Default Value if not initialized
	if (AST.getExpr() == nullptr) {

		// Create default Sema Value
        SemaValue *Sema = SemaBuilder::CreateDefaultValue(*Type);

		// Create Left Side Expression
		ASTIdentifier *LeftExpr = ASTBuilder::CreateIdentifier(LocalVar);
        LeftExpr->setSema(LocalVar->getSema());

		// Create Assignment Expression
		ASTBinaryOp *BinaryExpr = ASTBuilder::CreateBinary(LocalVar->getLocation(), ASTBinaryOpKind::OP_BINARY_ASSIGN, LeftExpr, Sema->getAST());
        AST.setExpr(BinaryExpr);

    } else {

        // Resolve Initialization Expression
        AST.getExpr()->accept(*this);

    	if (AST.getExpr()->getExprKind() == ASTExprKind::EXPR_BINARY &&
    		static_cast<ASTBinaryOp *>(AST.getExpr())->getBinaryKind() == ASTBinaryKind::OP_BINARY_ASSIGN) {
    		AST.getExpr()->getSema()->setType(Type);
    	}
	}

	FLY_DEBUG_END("Resolver", "visit(ASTDeclStmt)");
}

void Resolver::visit(ASTFailStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTFailStmt)");
	CurrentStmt = &AST;
	ASTExpr *Expr = AST.getExpr();

	if (Expr != nullptr) {
		Expr->accept(*this);
	}
	FLY_DEBUG_END("Resolver", "visit(ASTFailStmt)");
}

void Resolver::visit(ASTHandleStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTHandleStmt)");
	CurrentStmt = &AST;
	AST.getHandle()->accept(*this);
	ASTExpr *ErrorHandler = AST.getErrorHandler();

	if (ErrorHandler != nullptr) {
		ErrorHandler->accept(*this);
	}
	FLY_DEBUG_END("Resolver", "visit(ASTHandleStmt)");
}

void Resolver::visit(ASTReturnStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTReturnStmt)");
	CurrentStmt = &AST;
	SemaType *ReturnType = CurrentFunction->getReturnType(); // Force Return Expr to be of Return Type
	ASTExpr *Expr= AST.getExpr();
	if (Expr != nullptr) {
		Expr->accept(*this);
	} else if (!ReturnType->isVoid()) {
		Diag(AST.getLocation(), diag::err_invalid_return_type);
	}
	FLY_DEBUG_END("Resolver", "visit(ASTReturnStmt)");
}

void Resolver::visit(ASTDeleteStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTDeleteStmt)");
	CurrentStmt = &AST;
	ASTExpr * Expr = AST.getExpr();

	Expr->accept(*this);
	FLY_DEBUG_END("Resolver", "visit(ASTDeleteStmt)");
}


void Resolver::visit(ASTBreakStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTBreakStmt)");
	CurrentStmt = &AST;
	// Do Nothing
	FLY_DEBUG_END("Resolver", "visit(ASTBreakStmt)");
}

void Resolver::visit(ASTContinueStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTContinueStmt)");
	CurrentStmt = &AST;
	// Do Nothing
	FLY_DEBUG_END("Resolver", "visit(ASTContinueStmt)");
}

void Resolver::visit(ASTBlockStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTBlockStmt)");
	CurrentStmt = &AST;

	// Resolve Statements
	for (ASTStmt *Stmt : AST.getContent()) {
		Stmt->accept(*this);
	}

	// Check LocalVar initialization
	// TODO
	//    for (auto &LocalVar : Block->LocalVars) {
	//        if (!LocalVar.second->isInitialized())
	//            Diag(LocalVar.getValue()->getLocation(), diag::err_sema_uninit_var) << LocalVar.getValue()->getName();
	//    }
	FLY_DEBUG_END("Resolver", "visit(ASTBlockStmt)");
}

void Resolver::visit(ASTRuleStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTRuleStmt)");
	CurrentStmt = &AST;
	AST.getRule()->accept(*this);
	AST.getStmt()->accept(*this);
	FLY_DEBUG_END("Resolver", "visit(ASTRuleStmt)");
}

void Resolver::visit(ASTIfStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTIfStmt)");
	CurrentStmt = &AST;
	AST.getRule()->accept(*this);

	// Validate Rule Type
	// AST->Rule->Type = SemaBuiltin::getBoolType();

	AST.getStmt()->accept(*this);

	// Elsif Blocks
	for (ASTRuleStmt *Elsif : AST.getElsif()) {
		Elsif->accept(*this);
		// Validate Rule Type
		// Elsif->Rule->Type = SemaBuiltin::getBoolType();
		Elsif->getStmt()->accept(*this);
	}

	// Else Block
	if (AST.getElse()) {
		AST.getElse()->accept(*this);
	}
	FLY_DEBUG_END("Resolver", "visit(ASTIfStmt)");
}

void Resolver::visit(ASTSwitchStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTSwitchStmt)");
	CurrentStmt = &AST;

	// Switch Variable
	AST.getVar()->accept(*this);

	// Case Blocks
	for (ASTRuleStmt *Case : AST.getCases()) {
		Case->getRule()->accept(*this);
        Case->getStmt()->accept(*this);

		// Validate Case Type
		// Case->getRule()->getType()->isInteger();
	}

	// Default Block
	if (AST.getDefault()) {
        AST.getDefault()->accept(*this);
    }
	FLY_DEBUG_END("Resolver", "visit(ASTSwitchStmt)");
}

void Resolver::visit(ASTLoopStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTLoopStmt)");
	CurrentStmt = &AST;

	if (AST.getRule()) { // Error: empty condition expr
		AST.getRule()->accept(*this);
	}

	// Check Init
	if (AST.getInit()) {
		AST.getInit()->accept(*this);
	}

	// Validate Rule Type
	// Validator->CheckConvertibleTypes(LoopStmt->getRule()->getType(), SemaBuiltin::getBoolType());

	// Loop Statement
	AST.getStmt()->accept(*this);

	// Post Loop Statement
	if (AST.getPost()) {
		AST.getPost()->accept(*this);
	}
	FLY_DEBUG_END("Resolver", "visit(ASTLoopStmt)");
}

void Resolver::visit(ASTLoopInStmt &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTLoopInStmt)");
	CurrentStmt = &AST;

	AST.getItem()->accept(*this);
    AST.getList()->accept(*this);

	// Loop Statement
	AST.getStmt()->accept(*this);
	FLY_DEBUG_END("Resolver", "visit(ASTLoopInStmt)");
}

void Resolver::visit(ASTIdentifier &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTIdentifier)");
	ResolveExpr(AST);
	FLY_DEBUG_END("Resolver", "visit(ASTIdentifier)");
}

void Resolver::visit(ASTMember &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTMember)");
	ResolveExpr(AST);
	FLY_DEBUG_END("Resolver", "visit(ASTMember)");
}

void Resolver::visit(ASTCall &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTCall)");
	ResolveExpr(AST);
	FLY_DEBUG_END("Resolver", "visit(ASTCall)");
}

void Resolver::visit(ASTUnaryOp &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTUnaryOp)");
	ASTExpr *Expr = AST.getExpr();
	Expr->accept(*this);

	// Create Sema
	SemaUnary *Sema = SemaBuilder::CreateUnary(AST);
	AST.setSema(Sema);

	FLY_DEBUG_END("Resolver", "visit(ASTUnaryOp)");
}

void Resolver::visit(ASTBinaryOp &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTBinaryOp)");

	AST.getLeftExpr()->accept(*this);
    AST.getRightExpr()->accept(*this);

	Validator->CheckBinary(AST);

	// Create Sema
	SemaBinary *Sema = SemaBuilder::CreateBinary(AST);
	AST.setSema(Sema);

	FLY_DEBUG_END("Resolver", "visit(ASTBinaryOp)");
}

void Resolver::visit(ASTTernaryOp &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTTernaryOp)");
	// Resolve Condition Expr
	AST.getConditionExpr()->accept(*this);
	Validator->CheckConvertibleTypes(AST.getConditionExpr()->getType(), SemaBuiltin::getBoolType());

	// Resolve True and False Expr
	AST.getTrueExpr()->accept(*this);
	AST.getFalseExpr()->accept(*this);

	// Create Sema
	SemaTernary *Sema = SemaBuilder::CreateTernary(AST);
	AST.setSema(Sema);

	FLY_DEBUG_END("Resolver", "visit(ASTTernaryOp)");
}

void Resolver::visit(ASTCast &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTCast)");
	AST.getCast()->accept(*this);
    AST.getExpr()->accept(*this);
	// TODO: Validate Cast
	FLY_DEBUG_END("Resolver", "visit(ASTCast)");
}

void Resolver::visit(ASTBoolValue &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTBoolValue)");
	SemaBoolValue *Sema = SemaBuilder::CreateBoolValue(AST);
	AST.setSema(Sema);
	FLY_DEBUG_END("Resolver", "visit(ASTBoolValue)");
}

void Resolver::visit(ASTNumberValue &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTNumberValue)");

	ASTExpr *Expr = nullptr;
	if (CurrentStmt->getStmtKind() == ASTStmtKind::STMT_DECL) {
		Expr = static_cast<ASTDeclStmt *>(CurrentStmt)->getExpr();
	} else if (CurrentStmt->getStmtKind() == ASTStmtKind::STMT_EXPR) {
		Expr = static_cast<ASTExprStmt *>(CurrentStmt)->getExpr();
	}

	SemaValue *Sema = nullptr;
	if (Expr->getExprKind() == ASTExprKind::EXPR_BINARY) {
		ASTBinaryOp *Binary = static_cast<ASTBinaryOp *>(Expr);
		if (Binary && Binary->getBinaryKind() == ASTBinaryKind::OP_BINARY_ASSIGN) {
			SemaType *Type = Binary->getLeftExpr()->getType();

			// Integer Value
			if (Type->isInteger()) {
				Sema = SemaBuilder::CreateIntValue(AST, static_cast<SemaIntType *>(Type));
				AST.setSema(Sema);
				return;
			}

			// Float Value
			if (Type->isFloatingPoint()) {
				Sema = SemaBuilder::CreateFloatValue(AST, static_cast<SemaFloatType *>(Type));
				AST.setSema(Sema);
				return;
			}
		}
	}


	Sema = SemaBuilder::CreateNumberValue(AST);
	AST.setSema(Sema);

	FLY_DEBUG_END("Resolver", "visit(ASTNumberValue)");
}

void Resolver::visit(ASTStringValue &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTStringValue)");
	SemaValue *Sema = SemaBuilder::CreateStringValue(AST);
	AST.setSema(Sema);
	FLY_DEBUG_END("Resolver", "visit(ASTStringValue)");
}

void Resolver::visit(ASTArrayValue &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTArrayValue)");

	llvm::SmallVector<SemaValue *, 8> Values;
	for (auto Value : AST.getValues()) {
		Value->accept(*this);
		Values.push_back(Value->getSema());
	}

	SemaArrayValue *Sema = SemaBuilder::CreateArrayValue(AST, Values);
	AST.setSema(Sema);

	FLY_DEBUG_END("Resolver", "visit(ASTArrayValue)");
}

void Resolver::visit(ASTStructValue &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTStructValue)");

	llvm::StringMap<SemaValue *> Values;
	for (auto &Entry : AST.getValues()) {
		Entry.second->accept(*this);
		Values.insert(std::make_pair(Entry.getKey(), Entry.second->getSema()));
	}

	SemaStructValue *Sema = SemaBuilder::CreateStructValue(AST, Values);
	AST.setSema(Sema);
	FLY_DEBUG_END("Resolver", "visit(ASTStructValue)");
}

void Resolver::visit(ASTNullValue &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTNullValue)");
	SemaValue *Sema = SemaBuilder::CreateNullValue(AST);
	AST.setSema(Sema);
	FLY_DEBUG_END("Resolver", "visit(ASTNullValue)");
}

void Resolver::visit(ASTDefaultValue &AST) {
	FLY_DEBUG_START("Resolver", "visit(ASTDefaultValue)");
	if (CurrentStmt) {
		if (CurrentStmt->getStmtKind() == ASTStmtKind::STMT_EXPR) {
			ASTExprStmt *Stmt = static_cast<ASTExprStmt *>(CurrentStmt);
			ASTExpr *Expr = Stmt->getExpr();

			// Check if the expression is a binary assignment operation
			if (Expr && Expr->getExprKind() == ASTExprKind::EXPR_BINARY) {
				ASTBinaryOp *BinOp = static_cast<ASTBinaryOp *>(Expr);

				// Check if it's an assignment operation
				if (BinOp->getOpKind() == ASTBinaryOpKind::OP_BINARY_ASSIGN) {
					// This AST Default Value is the only Expr in the assignment
					if (BinOp->getRightExpr()->getExprKind() == ASTExprKind::EXPR_VALUE) {
						SemaType *T = BinOp->getLeftExpr()->getType();

						// Create the default value
						SemaValue *Sema = SemaBuilder::CreateDefaultValue(*T);
						AST.setSema(Sema);
					}
				}
			}
		}
	}
	FLY_DEBUG_END("Resolver", "visit(ASTDefaultValue)");
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

void Resolver::ResetCurrent() {
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

		// Resolve Imports
		ResolveImports(Module);

		// Resolve Nodes
		for (auto &Node : Module->getNodes()) {
			switch (Node->getKind()) {

				case SemaKind::FUNCTION:
					ResolveFunction(static_cast<SemaFunction *>(Node));
					break;
				case SemaKind::CLASS:
					ResolveClassType(static_cast<SemaClassType *>(Node));
					break;
				case SemaKind::ENUM: {
					ResolveEnumType(static_cast<SemaEnumType *>(Node));
				} break;
			}
		}
	}

	// Resolve Bodies
	for (auto Scope : Reg.getBodies()) {
		ResolveBody(Scope);
	}
	FLY_DEBUG_END("Resolver", "Resolve");
}

// TODO: remove Comment
// void Resolver::ResolveComment(SemaComment *Comment, ASTNode* AST) {
// 	if (AST->getKind() == ASTKind::AST_FUNCTION) {
// 		ASTFunction * Function = (ASTFunction *) AST;
// 		Validator->CheckCommentParams(Comment, Function->getParams());
// 		Validator->CheckCommentReturn(Comment, Function->getReturnTypeRef());
// 		Validator->CheckCommentFail(Comment);
// 	}
// }

void Resolver::ResolveImports(SemaModule *Module) {
	FLY_DEBUG_START("Resolver", "ResolveImports");
	for (auto Import : Module->getImports()) {

		// NameSpace, Class or Enum
		SemaNameSpace * NS = Reg.getNameSpace(Import->getTarget());
		Import->setSymbols(NS->getSymbols());

		// Create Symbol of Import
		std::string Name = Helper::Flatten(Import->getNames());
		Symbol *Sym = new Symbol(Name, Import->getKind(), Import);

		// Add Symbol to the module scope
		addSymbol(Sym);
	}
	FLY_DEBUG_END("Resolver", "ResolveImports");
}

/**
 * Resolve Module Function Definitions
 */
void Resolver::ResolveFunction(SemaFunction *Func) {
	FLY_DEBUG_START("Resolver", "ResolveFunction");
	ASTFunction &AST = Func->getAST();

	// Set currents
	CurrentFunction = Func;
	CurrentScope = Func->getSymbols();

	// Resolve Return Type
	ASTType *ReturnType = AST.getReturnType();
	ReturnType->accept(*this);
	Func->setReturnType(ReturnType->getSema());

	// Resolve Parameters Types
	for (auto Param : AST.getParams()) {

		// resolve parameter type
		Param->accept(*this);
		Func->addParam(Param->getSema());
	}

	// Set Function Mangled Name
	std::string MangledName = Helper::Mangle(&AST);
	Func->setMangledName(MangledName);

	// Add to Body list for resolve in the next step
	// Enter Function Scope
	EnterScope();
	Reg.addBody(CurrentScope, AST.getBody());
	ExitScope();
	FLY_DEBUG_END("Resolver", "ResolveFunction");
}

void Resolver::ResolveClassType(SemaClassType *ClassType) {
	FLY_DEBUG_START("Resolver", "ResolveClassType");
	CurrentClass = ClassType;

	// Create the Default Constructor if no constructors are defined
	if (ClassType->getClassKind() != SemaClassKind::INTERFACE && ClassType->getConstructors().empty())
		this->CreateDefaultConstructor();

	// Resolve Base Classes
	// this->ResolveBaseClasses(ClassType);

	// Resolve Nodes: Attributes, Methods and Constructors
	for (auto &Node: ClassType->getAST().getNodes()) {
		Node->accept(*this);
	}

	// Create a Default Constructor if not exists
	if (CurrentClass->getConstructors().empty() && CurrentClass->getClassKind() != SemaClassKind::INTERFACE) {
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
			Diag(diag::err_syntax_error);
			FLY_DEBUG_END("Resolver", "ResolveBaseClasses");
			return;
		}

		// Resolve the Base Sema
		SemaType *NamedType = Reg.LookupNamedType(*static_cast<ASTNamedType *>(AST), CurrentNameSpace);
		if (!NamedType->isClass()) {
			// Error: cannot extend a type which differ from Class
			Diag(diag::err_syntax_error);
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
	SemaClassMethod *Sema = SemaBuilder::CreateDefaultConstructor(CurrentClass);

	// Add Method/Constructor
	CurrentClass->addMethod(Sema); // Function Local var to be allocated
	CurrentClass->addConstructor(Sema);

	// Set current function
	CurrentFunction = Sema;

	FLY_DEBUG_END("Resolver", "CreateDefaultConstructor");
}

void Resolver::ResolveEnumType(SemaEnumType *Enum) {
	FLY_DEBUG_START("Resolver", "ResolveEnumType");
	for (auto &AST: Enum->getAST().getNodes()) {
		AST->accept(*this);
	}
	FLY_DEBUG_END("Resolver", "ResolveEnumType");
}

void Resolver::ResolveBody(LocalScope &Scope) {
	FLY_DEBUG_START("Resolver", "ResolveBody");
	CurrentScope = Scope.Symbols;
	Scope.Body->accept(*this);
	FLY_DEBUG_END("Resolver", "ResolveBody");
}

void Resolver::ResolveExpr(ASTExpr &Expr) {
	FLY_DEBUG_START("Resolver", "ResolveExpr");
	if (!Expr.isVisited()) {

		// Move Expr to Root Parent
		ASTExpr *Parent = &Expr;
		while (Parent->getParent() != nullptr) {
			Parent = Parent->getParent();
		}

		// Start Resolving from the Root Parent
		switch (Parent->getExprKind()) {
			case ASTExprKind::EXPR_IDENTIFIER:
				ResolveParent(static_cast<ASTIdentifier*>(Parent));
			break;
			case ASTExprKind::EXPR_CALL:
				ResolveParent(static_cast<ASTCall*>(Parent));
			break;
			case ASTExprKind::EXPR_MEMBER:
				Diag(Parent->getLocation(), diag::err_invalid_behavior);
			break;
			default:
				Diag(Parent->getLocation(), diag::err_invalid_behavior);
		}

		Expr.setVisited(true);
	}
	FLY_DEBUG_END("Resolver", "ResolveExpr");
}

void Resolver::ResolveParent(ASTIdentifier *AST) {
	FLY_DEBUG_START("Resolver", "ResolveParent(ASTIdentifier)");
	if (!AST->isVisited()) {
		AST->setVisited(true);
		SemaNode * Sema = nullptr;

		// ---------------------------------------
		// 1. Try local and parent scopes
		// ---------------------------------------
		SymbolTable * Scope = CurrentScope;
		while (!Sema && Scope) {
			if (auto *S = Scope->lookup(AST->getName())) {
				Sema = S->getRef();
				break;
			}
			Scope = Scope->getParent();
		}

		// ---------------------------------------
		// 2. Try class members of 'this'
		// ---------------------------------------
		if (!Sema && CurrentFunction && CurrentFunction->getKind() == SemaKind::METHOD) {
			SemaClassMethod* Method = static_cast<SemaClassMethod*>(CurrentFunction);
			SemaClassType* OwnerClass = Method->getClass();

			// "this"
			if (AST->getName() == "this") {
				if (!Method->isStatic())
					Sema = OwnerClass->getThis();
			}

			// Attributes
			else if (!Method->isStatic()) {
				if (auto* Attr = OwnerClass->getAttributes().lookup(AST->getName()))
					Sema = Attr;
			}
		}

		// Store Sema into AST
		if (Sema && Sema->getKind() == SemaKind::VAR) {
			AST->setSema(static_cast<SemaVar *>(Sema));
		}

		// ---------------------------------------
		// 3. Try Named Types
		// ---------------------------------------
		if (!Sema) {
			if (auto* T = Reg.LookupNamedType(AST->getName(), CurrentNameSpace))
				Sema = T;
		}

		// ---------------------------------------
		// 4. Try Namespace
		// ---------------------------------------
		if (!Sema) {
			if (auto* NS = Reg.LookupNameSpace(AST->getName()))
				Sema = NS;
		}

		// ---------------------------------------
		// Not Found → Error
		// ---------------------------------------
		if (!Sema) {
			Diag(AST->getLocation(), diag::err_syntax_error);
			FLY_DEBUG_END("Resolver", "ResolveParent(ASTIdentifier)");
			return;
		}

		// ---------------------------------------
		// 5. Continue with children
		// ---------------------------------------
		if (AST->getChild()) {
			ResolveChild(Sema, AST->getChild());
		}
	}
	FLY_DEBUG_END("Resolver", "ResolveParent(ASTIdentifier)");
}

void Resolver::ResolveParent(ASTCall *AST) {
	FLY_DEBUG_START("Resolver", "ResolveParent(ASTCall)");
	if (!AST->isVisited()) {
		AST->setVisited(true);

		// Resolve Expression in Arguments
		ResolveCallArgs(AST);

		// if Arguments are not resolved is not possible go ahead with call reference resolution
		// cannot resolve with the function parameters types
		std::string MangledName = Helper::Mangle(AST);

		SemaCall *Sema= nullptr;

		// Create a new instance using a Constructor
		if (AST->getCallKind() == ASTCallKind::CALL_NEW ||
			AST->getCallKind() == ASTCallKind::CALL_NEW_UNIQUE ||
			AST->getCallKind() == ASTCallKind::CALL_NEW_SHARED ||
			AST->getCallKind() == ASTCallKind::CALL_NEW_WEAK) {

			// Take the Type from the CurrentNameSpace
			SemaType *Type = Reg.LookupNamedType(AST->getName(), CurrentNameSpace);

			// ---------------------------------------
			// Class Not Found → Error
			// ---------------------------------------
			if (Type == nullptr) {
				Diag(AST->getLocation(), diag::err_unref_type);
				return;
			}

			// ---------------------------------------
			// Type is not a Class → Error
			// ---------------------------------------
			if (!Type->isClass()) {
				Diag(AST->getLocation(), diag::err_unref_type);
				return;
			}

			SemaClassType *Class = static_cast<SemaClassType *>(Type);
			SemaClassMethod *Constructor = Class->getConstructors().lookup(MangledName);

			// ---------------------------------------
			// Constructor Not Found → Error
			// ---------------------------------------
			if (Constructor == nullptr) {
				// Error: func not found
				Diag(AST->getLocation(), diag::err_syntax_error);
				return;
			}

			Sema = SemaBuilder::CreateCall(*AST, Type, Constructor);
			} else {

				// Call a Function or a Class Method
				SemaFunctionBase *Func = nullptr;

				// Try with Method
				if (CurrentFunction->getKind() == SemaKind::METHOD) {

					// Check if the Call is a Base Class Constructor Method
					SemaClassType *CurrentClass = static_cast<SemaClassMethod *>(CurrentFunction)->getClass();
					SemaType * T = Reg.LookupNamedType(AST->getName(), CurrentNameSpace);

					if (T->isClass()) {
						SemaClassType * C = static_cast<SemaClassType *>(T);

						// Call to Base Class Constructor Method
						if (C->isBaseOrEquals(CurrentClass)) {
							// Resolve Call with Class Constructor if is not private
							SemaClassMethod *Constructor = C->getConstructors().lookup(MangledName);

							// check if Constructor is private
							if (Constructor->getVisibility() == SemaVisibilityKind::PRIVATE) {
								// Error: method is private, cannot be called from outside the class
								Diag(AST->getLocation(), diag::err_syntax_error);
								return;
							}
							// Take the Constructor
							Func = Constructor;
						} else {
							// Static Call to Class Method
							SemaClassMethod *Method = C->getMethods().lookup(MangledName);

							// Check if Method is static
							if (!Method->isStatic()) {
								// Error: method is not static, cannot be called statically
								Diag(AST->getLocation(), diag::err_syntax_error);
							}

							// Check if Method is private
							if (Method->getVisibility() == SemaVisibilityKind::PRIVATE) {
								// Error: method is private, cannot be called from outside the class
								Diag(AST->getLocation(), diag::err_syntax_error);
							}

							// Take the Method
							Func = Method;
						}
					}
				}

				// Take the Function
				if (Func == nullptr) {
					Func = Reg.LookupFunction(MangledName, CurrentNameSpace);
				}

				// ---------------------------------------
				// Function Not Found → Error
				// ---------------------------------------
				if (Func == nullptr) {
					Diag(AST->getLocation(), diag::err_syntax_error);
					return;
				}

				Sema = SemaBuilder::CreateCall(*AST, Func->getReturnType(), Func);

				// Continue by Resolving Child
				if (AST->getChild()) {
					ResolveChild(Sema, AST->getChild());
				}
			}

		// Set the Call Sema ErrorHandler
		ResolveErrorHandler(Sema);
		AST->setSema(Sema);
	}
	FLY_DEBUG_END("Resolver", "ResolveParent(ASTCall)");

}

void Resolver::ResolveChild(SemaNode *Parent, ASTExpr *AST) {
	if (!AST->isVisited()) {
		AST->setVisited(true);

		if (!Parent) {
			Diag(diag::err_invalid_behavior);
			return;
		}

		// Resolve if parent is a Namespace
		if (Parent->getKind() == SemaKind::NAMESPACE) {
			SemaNameSpace *NameSpace = static_cast<SemaNameSpace *>(Parent);
			ResolveChild(NameSpace, AST);
			return;
		}

		// Resolve if parent is a Type
		if (Parent->getKind() == SemaKind::TYPE) {
			SemaType *Type = static_cast<SemaType *>(Parent);

			// Resolve if parent is a Class Type
			if (Type->isClass()) {
				SemaClassType *ClassType = static_cast<SemaClassType *>(Type);
				ResolveChild(ClassType, AST);
				return;
			}

			// Resolve if parent is an Enum Type
			if (Type->isEnum()) {
				SemaEnumType *EnumType = static_cast<SemaEnumType *>(Type);
				ResolveChild(EnumType, AST);
				return;
			}
		}

		// Resolve if parent is a Call
		if (Parent->getKind() == SemaKind::CALL) {
			SemaCall *Call = static_cast<SemaCall *>(Parent);
			ResolveChild(Call, AST);
			return;
		}

		// Resolve if parent is a Var
		if (Parent->getKind() == SemaKind::VAR) {
			SemaVar *Var = static_cast<SemaVar *>(Parent);
			ResolveChild(Var, AST);
			return;
		}
	}
	Diag(diag::err_invalid_behavior);
}

void Resolver::ResolveChild(SemaNameSpace *NameSpace, ASTExpr *AST) {
	if (!AST->isVisited()) {
		AST->setVisited(true);

		// Resolve as NameSpace Function
		if (AST->getExprKind() == ASTExprKind::EXPR_CALL) {
			ASTCall *Call = static_cast<ASTCall *>(AST);
			ResolveCallArgs(Call);
			std::string MangledName = Helper::Mangle(Call);


			// Lookup Function
			Symbol * Sym = NameSpace->getSymbols()->lookup(MangledName);
			SemaFunction * Function = static_cast<SemaFunction *>(Sym->getRef());

			// Set Sema
			SemaCall *Sema = SemaBuilder::CreateCall(*Call, Function->getReturnType(), Function);
			Call->setSema(Sema);

			// Set the Call Sema ErrorHandler
			ResolveErrorHandler(Sema);

			if (AST->getChild())
				ResolveChild(Sema, AST->getChild());

			return;
		}

		// Resolve as NameSpace or Type
		if (AST->getExprKind() == ASTExprKind::EXPR_MEMBER) {
			ASTMember *Member = static_cast<ASTMember *>(AST);

			SemaNode *Sema = nullptr;
			if (auto* NS = Reg.LookupNameSpace(Member->getName(), NameSpace))
				Sema = NS;

			if (auto* T = Reg.LookupNamedType(Member->getName(), NameSpace))
				Sema = T;

			if (AST->getChild())
				ResolveChild(Sema, AST->getChild());

			return;
		}
	}

	Diag(diag::err_invalid_behavior);
}

void Resolver::ResolveChild(SemaClassType *ClassType, ASTExpr *AST) {
	if (!AST->isVisited()) {
		AST->setVisited(true);

		// Resolve as Static Class Method
		if (AST->getExprKind() == ASTExprKind::EXPR_CALL) {
			ASTCall *Call = static_cast<ASTCall *>(AST);

			// Resolve Call as Class Method
			ResolveCallArgs(Call);
			std::string MangledName = Helper::Mangle(Call);

			// Create a call to class method
			SemaClassMethod* Method = ClassType->getMethods().lookup(MangledName);
			if (Method) {

				// Check if Method belongs to a super class or is static
				if (!ClassType->isDerivedOrEquals(Method->getClass()) && !Method->isStatic()) {
					// Error: method cannot be called statically
					Diag(Call->getLocation(), diag::err_syntax_error) << Call->getName();
				}

				// Set Sema
				SemaCall *Sema = SemaBuilder::CreateCall(*Call, Method->getReturnType(), Method);
				Call->setSema(Sema);

				// Set the Call Sema ErrorHandler
				ResolveErrorHandler(Sema);

				if (AST->getChild())
					ResolveChild(Sema, AST->getChild());
			}
			return;
		}

		// Resolve as Static Class Attribute
		if (AST->getExprKind() == ASTExprKind::EXPR_MEMBER) {
			ASTMember *Member = static_cast<ASTMember *>(AST);

			SemaClassAttribute *Sema = ClassType->getAttributes().lookup(Member->getName());
			if (Sema) {
				Member->setSema(Sema);

				if (!Sema->isStatic()) {
					// Error: cannot resolve a non-static attribute without a parent
					Diag(Member->getLocation(), diag::err_syntax_error) << Member->getName();
				}

				if (AST->getChild())
					ResolveChild(Sema, AST->getChild());
			}
			return;
		}
	}

	Diag(diag::err_invalid_behavior);
}

void Resolver::ResolveChild(SemaEnumType *EnumType, ASTExpr *AST) {
	if (!AST->isVisited()) {
		AST->setVisited(true);

		// Resolve as Enum Entry
		if (AST->getExprKind() == ASTExprKind::EXPR_MEMBER) {
			ASTMember *Member = static_cast<ASTMember *>(AST);
			SemaEnumEntry *Entry = EnumType->getEntries().lookup(Member->getName());

			// Check: If No Enum Entry found
			if (Entry == nullptr) {
				Diag(diag::err_invalid_behavior);
				return;
			}

			// Check: Enum cannot have child expressions
			if (AST->getChild()) {
				Diag(diag::err_invalid_behavior);
				return;
			}

			Member->setSema(Entry);
			return;
		}
	}

	// Invalid Enum Reference
	Diag(diag::err_invalid_behavior);
}

void Resolver::ResolveChild(SemaCall *Parent, ASTExpr *AST) {

	// If AST is a Call
	if (AST->getExprKind() == ASTExprKind::EXPR_CALL) {

		// Resolve Call
		ASTCall *Call = static_cast<ASTCall *>(AST);
		SemaCall *Sema = ResolveChildCall(Parent, Call);
		if (Sema) {

			// Set the Call Sema ErrorHandler
			ResolveErrorHandler(Sema);

			// Go with child
			if (AST->getChild())
				ResolveChild(Sema, AST->getChild());
		}
	}

	// if AST is a Var
	if (AST->getExprKind() == ASTExprKind::EXPR_MEMBER) {

		// Resolve Member
		ASTMember *Member = static_cast<ASTMember *>(AST);
		SemaVar *Sema = ResolveChildMember(Parent, Member);
		if (Sema) {

			// Go with child
			if (AST->getChild())
				ResolveChild(Sema, AST->getChild());
		}
	}

	// Invalid Child
	Diag(diag::err_invalid_behavior);
}

void Resolver::ResolveChild(SemaVar *Parent, ASTExpr *AST) {

	// Validate Type
	SemaType *ParentType = Parent->getType();

	// If AST is a Call
	if (AST->getExprKind() == ASTExprKind::EXPR_CALL) {

		// Resolve Call
		ASTCall *Call = static_cast<ASTCall *>(AST);
		SemaCall *Sema = ResolveChildCall(Parent, Call);
	}

	// If AST is a Member
	if (AST->getExprKind() == ASTExprKind::EXPR_MEMBER) {

		ASTMember *Member = static_cast<ASTMember *>(AST);
		SemaVar *Sema = ResolveChildMember(Parent, Member);
	}

	Diag(diag::err_invalid_behavior);
}

SemaCall *Resolver::ResolveChildCall(SemaExpr *Parent, ASTCall *AST) {
	if (!AST->isVisited()) {
		AST->setVisited(true);

		// set Mangled Identifier
		ResolveCallArgs(AST);
		std::string MangledName = Helper::Mangle(AST);

		// Check Parent Type is a Class
		SemaType *ParentType = Parent->getType();
		if (!ParentType->isClass()) {
			Diag(diag::err_invalid_behavior);
			return nullptr;
		}

		// Search Method into Class Scope
		SemaClassType *ClassType = static_cast<SemaClassType *>(ParentType);
		Symbol * Sym = ClassType->getSymbols()->lookup(MangledName);

		// Check Symbol is not a Method
		if (Sym->getKind() != SemaKind::METHOD) {
			Diag(diag::err_invalid_behavior);
			return nullptr;
		}

		// TODO check in base classes
		// for (SemaClassType *BaseClass : ClassType->getBaseClasses()) {
		// 	while (BaseClass && !CalledMethod) {
		// 		CalledMethod = BaseClass->getMethods().lookup(Mangled);
		// 		// If not found, go up the inheritance chain of this base class
		// 		const auto &NextBases = BaseClass->getBaseClasses();
		// 		BaseClass = !NextBases.empty() ? NextBases.front() : nullptr;
		// 	}
		// 	if (CalledMethod) break;
		// }

		// Set Call -> Method
		SemaClassMethod *Method = static_cast<SemaClassMethod *>(Sym->getRef());

		// Build Sema
		SemaCall *Sema = SemaBuilder::CreateCall(*AST, Method->getReturnType(), Method);
		AST->setSema(Sema);

		// Set the Call Sema ErrorHandler
		ResolveErrorHandler(Sema);

		// Look for child
		if (AST->getChild())
			ResolveChild(Sema, AST->getChild());
	}
	return AST->getSema();
}

SemaVar * Resolver::ResolveChildMember(SemaExpr *Parent, ASTMember *AST) {
	if (!AST->isVisited()) {
		AST->setVisited(true);

		// Build Sema
		SemaVar *Sema = nullptr;
		SemaType *ParentType = Parent->getType();
		if (ParentType->isClass()) {

			// Check Parent Type is a Class
			SemaClassType * ClassType = static_cast<SemaClassType *>(ParentType);
			Symbol * Sym = ClassType->getSymbols()->lookup(AST->getName());

			// Check Symbol is not an Attribute
			if (Sym->getKind() != SemaKind::ATTRIBUTE) {
				Diag(diag::err_invalid_behavior);
				return nullptr;
			}

			// Set Class Attribute
			SemaClassAttribute *Attribute = static_cast<SemaClassAttribute *>(Sym->getRef());
			SemaMemberVar *Member = SemaBuilder::CreateMemberVar(*Attribute->getAST(), *Parent, Attribute);

			// Set the Sema
			Sema = Attribute;
		} else if (ParentType->isEnum()) {

			// Check Parent Type is an Enum
			SemaEnumType *EnumType = static_cast<SemaEnumType *>(ParentType);
			Symbol * Sym = EnumType->getSymbols()->lookup(AST->getName());

			// Check Symbol is not an Enum Entry
			if (Sym->getKind() != SemaKind::ENUM_ENTRY) {
				Diag(diag::err_invalid_behavior);
				return nullptr;
			}

			// Set Enum Entry
			SemaEnumEntry *EnumEntry = static_cast<SemaEnumEntry *>(Sym->getRef());

			// Set the Sema
			Sema = EnumEntry;
		} else {

			// Parent is not an object type
			Diag(diag::err_invalid_behavior);
			return nullptr;
		}

		// Configure AST
		AST->setSema(Sema);
	}

	return AST->getSema();
}

void Resolver::ResolveCallArgs(ASTCall *AST) {
	FLY_DEBUG_START("Resolver", "ResolveCallArgs");
	for (auto Arg : AST->getArgs()) {
		Arg->getExpr()->accept(*this);
	}
	FLY_DEBUG_END("Resolver", "ResolveCallArgs");
}

void Resolver::ResolveParams(ASTFunction &AST) {
	FLY_DEBUG_START("Resolver", "ResolveParams");
	for (auto Param : AST.getParams()) {
		Param->accept(*this);
	}
	FLY_DEBUG_END("Resolver", "ResolveParams");
}

void Resolver::ResolveErrorHandler(SemaCall *Sema) {
	FLY_DEBUG_START("Resolver", "ResolveErrorHandler");
	// Search until parent is null or parent is a Handle Stmt
	// When Parent Stmt is nullptr assign Function ErrorHandler to Call ErrorHandler
	ASTStmt *Parent = CurrentStmt;
	while (Sema->getErrorHandler() == nullptr) {
		Parent = Parent->getParent();
		if (Parent == nullptr) {
			Sema->ErrorHandler = CurrentFunction->getErrorHandler();
		} else if (Parent->getStmtKind() == ASTStmtKind::STMT_HANDLE) {
			ASTHandleStmt *HandleStmt = static_cast<ASTHandleStmt*>(Parent);
			if (HandleStmt->getErrorHandler() != nullptr) {
				Sema->ErrorHandler = reinterpret_cast<SemaErrorHandler *>(HandleStmt->getErrorHandler()->getSema());
			}
		}
	}
	FLY_DEBUG_END("Resolver", "ResolveErrorHandler");
}
