//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/Resolver.cpp - The Resolver
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/Resolver.h"
#include "Sema/SemaBuiltin.h"
#include "Sema/SemaImport.h"
#include "AST/ASTBuilder.h"
#include "Sema/SemaBuilder.h"
#include "Sema/SemaValidator.h"
#include "Sema/SymbolTable.h"
#include "Sema/SemaNameSpace.h"
#include "Sema/SemaModule.h"
#include "Sema/SemaClassType.h"
#include "Sema/SemaClassMethod.h"
#include "Sema/SemaClassAttribute.h"
#include "Sema/SemaEnumType.h"
#include "Sema/SemaMemberVar.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTClass.h"
#include "AST/ASTAttribute.h"
#include "AST/ASTMethod.h"
#include "AST/ASTEnum.h"
#include "AST/ASTEnumEntry.h"
#include "AST/ASTType.h"
#include "AST/ASTModule.h"
#include "AST/ASTArg.h"
#include "AST/ASTIfStmt.h"
#include "AST/ASTImport.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTSwitchStmt.h"
#include "AST/ASTLoopStmt.h"
#include "AST/ASTLoopInStmt.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTValue.h"
#include "AST/ASTExpr.h"
#include "AST/ASTReturnStmt.h"
#include "AST/ASTHandleStmt.h"
#include "AST/ASTDeleteStmt.h"
#include "AST/ASTFailStmt.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTOpExpr.h"
#include "CodeGen/CodeGen.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"
#include "llvm/ADT/StringMap.h"

#include <AST/ASTCast.h>
#include <AST/ASTParam.h>
#include <Sema/Helper.h>
#include <llvm/Transforms/IPO/FunctionImport.h>
#include <Sema/SemaCall.h>
#include <Sema/SemaEnumEntry.h>
#include <Sema/SemaFunction.h>
#include <Sema/SemaLocalVar.h>
#include <Sema/Registry.h>
#include <Sema/SemaParam.h>
#include <Sema/SemaValue.h>

using namespace fly;

Resolver::Resolver(DiagnosticsEngine &Diags, Registry &Reg) : Diags(Diags),
	CurrentModule(nullptr),
	Reg(Reg),
	CurrentNameSpace(Reg.getDefaultNameSpace()),
    CurrentScope(Reg.getDefaultNameSpace()->getSymbols()) {
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
	CurrentModule = new SemaModule(AST);
	Reg.addModule(CurrentModule);

	bool InModuleScope = false;
	for (size_t i = 0; i < AST.getNodes().size(); ++i) {
		auto Def = AST.getNodes()[i];

		if (i == 0) {
			if (Def->getKind() == ASTKind::AST_NAMESPACE) {
				// Visit Definition
				Def->accept(*this);
			} else {
				// Set the Module NameSpace
				CurrentNameSpace = Reg.getDefaultNameSpace();
			}
		} else {
			if (Def->getKind() == ASTKind::AST_NAMESPACE) {
				// Error: Namespace must be the first definition in the Module
				Diag(AST.getLocation(), diag::err_syntax_error);

				// Cannot go ahead with resolving
				return;
			}

			// Enter Module Scope
			if (!InModuleScope) {
				CurrentScope = CurrentNameSpace->getSymbols();
				EnterScope();
                InModuleScope = true;
            }

			// Visit Definition
			Def->accept(*this);
		}
	}

	ExitScope();
}

void Resolver::visit(ASTNameSpace &AST) {

	// Build the CurrentNameSpace
	SemaNameSpace *NameSpace = Reg.getOrCreateNameSpace(AST.getNames());

	// Set Symbol Table
	CurrentModule->setNameSpace(NameSpace);
	CurrentNameSpace = NameSpace;
	CurrentScope = CurrentNameSpace->getSymbols();
}

void Resolver::visit(ASTImport &AST) {
	// Error: Empty Import
	if (AST.getNames().empty()) {
		Diag(AST.getLocation(), diag::err_sema_import_undefined);
	}

	// TODO
	// Error: name is equals to the current ASTModule namespace
	// if (AST.getName() == Module->getNameSpace()->getName()) {
	// 	Diag(AST.getLocation(), diag::err_import_conflict_namespace) << AST.getName();
	// }

	// Replace with alias name if exists
	// llvm::StringRef Name = AST.getImport()->getName();

	// Error: alias is equals to the current ASTModule namespace
	// if (AST.getAlias()) {
	//
	// 	// Set Import Name
	// 	Name = AST.getAlias()->getName();
	//
	// 	// Check Alias
	// 	if (Module->getImports().lookup(Name) != nullptr) {
	// 		Diag(AST.getLocation(), diag::err_conflict_import_alias) << Name;
	// 		return;
	// 	}
	//
	// 	if (AST.getAlias()->getName() == Module->getNameSpace()->getName()) {
	// 		Diag(AST.getAlias()->getLocation(), diag::err_alias_conflict_namespace) << AST.getAlias()->getName();
	// 		return;
	// 	}
	// }

	// Add import in Module
	SemaImport * Import = SemaBuilder::CreateImport(*CurrentModule, AST);

	// Symbol *Sym = new Symbol(Import->getNames(), Import->getKind(), Import);
	// CurrentScope->insert(Sym);
}

void Resolver::visit(ASTFunction &AST) {
	ResetCurrent();

	// Enter Function Scope
	EnterScope();

	// Create Sema Function
	CurrentFunction = SemaBuilder::CreateFunction(*CurrentModule, CurrentScope, AST);

	// // Add to Symbol Table of Parent Scope (Module or Class)
	Symbol *Sym = new Symbol(AST.getName(), CurrentFunction->getKind(), CurrentFunction);
	CurrentScope->getParent()->insert(Sym);

	ExitScope();
}

void Resolver::visit(ASTClass &AST) {
	ResetCurrent();

	// Enter Class Scope
	EnterScope();

	// Create Sema Class
	CurrentClass = SemaBuilder::CreateClass(*CurrentModule, CurrentScope, AST);

	// Add to Symbol Table of Parent Scope (Module)
	Symbol *Sym = new Symbol(AST.getName(), SemaKind::CLASS, CurrentClass);
	CurrentScope->getParent()->insert(Sym);

	// Add Nodes in the next Resolve Steps

	// Exit Class Scope
	ExitScope();
}

void Resolver::visit(ASTAttribute &AST) {
	// Do not resolve again
	if (AST.isVisited()) {
		return;
	}
	AST.setVisited(true);

	// Find Var duplication
	SemaClassAttribute *ExistingAttr = CurrentClass->LookupAttribute(AST.getName());
	if (ExistingAttr) {
		Diag(AST.getLocation(), diag::err_sema_var_redefinition) << AST.getName();
		return;
	}

	// Resolve Type
	AST.getType()->accept(*this);

	// Create Class Attribute
	SemaClassAttribute *Sema = SemaBuilder::CreateClassAttribute(*CurrentClass, AST, *CurrentComment);
	CurrentClass->addAttribute(Sema); // Function Local var to be allocated

	// Set Sema Type
	SemaType * Type = AST.getType()->getSema();
	Sema->setType(Type);

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
	CurrentScope->insert(Sym);
}

void Resolver::visit(ASTMethod &AST) {
	// Do not resolve again
	if (AST.isVisited()) {
		return;
	}
	AST.setVisited(true);

	// Resolve Return Type add Params Type
	AST.getReturnType()->accept(*this);
	llvm::SmallVector<SemaType *, 8> Types = ResolveParams(AST);

	// Create Mangled Name
	std::string Mangled = Helper::MangleFunction(AST.getName(), Types);

	// Find Method duplication
	SemaClassMethod *ExistingMethod = CurrentClass->LookupMethod(Mangled);
	if (ExistingMethod) {
		Diag(AST.getLocation(), diag::err_sema_var_redefinition) << AST.getName();
		return;
	}

	// Create Class Method
	SemaClassMethod *Sema = SemaBuilder::CreateClassMethod(CurrentClass, AST, *CurrentComment);
	CurrentClass->addMethod(Sema); // Function Local var to be allocated
	CurrentFunction = Sema;

	// Add to Symbol Table
	Symbol *Sym = new Symbol(AST.getName(), Sema->getKind(), Sema);
	CurrentScope->insert(Sym);

	// Add to Body list for resolve in the next step
	// Enter Function Scope
	EnterScope();
	Reg.addBody(CurrentScope, AST.getBody());
	ExitScope();
}

void Resolver::visit(ASTEnum &AST) {
	ResetCurrent();

	// Enter Enum Scope
	EnterScope();

	// Create Sema Enum
	CurrentEnum = SemaBuilder::CreateEnum(*CurrentModule, CurrentScope, AST);

	// Add to Symbol Table of Parent Scope (Module)
	Symbol *Sym = new Symbol(AST.getName(), SemaKind::ENUM, CurrentEnum);
	CurrentScope->getParent()->insert(Sym);

	// Add enum entries
	for (auto Def : AST.getNodes()) {
		Def->accept(*this);
	}

	// Exit Class Scope
	ExitScope();
}

void Resolver::visit(ASTEnumEntry &AST) {
	// Do not resolve again
	if (AST.isVisited()) {
		return;
	}
	AST.setVisited(true);

	// Find Var duplication in the current scope
	SemaEnumEntry *ExistingEnum = CurrentEnum->LookupEntry(AST.getName());
	if (ExistingEnum) {
		Diag(AST.getLocation(), diag::err_sema_var_redefinition) << AST.getName();
	}

	SemaEnumEntry *Sema = SemaBuilder::CreateEnumEntry(*CurrentEnum, AST, CurrentComment);
	CurrentEnum->addEntry(Sema);
}

void Resolver::visit(ASTLocalVar &AST) {
	// Do not resolve again
	if (AST.isVisited()) {
		return;
	}
	AST.setVisited(true);

	SemaVar *Sema = SemaBuilder::CreateLocalVar(AST);
	CurrentFunction->getLocalVars().push_back(Sema); // Function Local var to be allocated

	// Find Var duplication in the current scope
	// TODO: check also in parent scopes for shadowing?
	Symbol *ExistingSym = CurrentScope->lookup(AST.getName());
    if (ExistingSym) {
        Diag(AST.getLocation(), diag::err_sema_var_redefinition) << AST.getName();
    }

	// Add to Symbol Table
	Symbol *Sym = new Symbol(AST.getName(), Sema->getKind(), Sema);
	CurrentScope->insert(Sym);
}

void Resolver::visit(ASTParam &AST) {
	// Create Sema Param
	SemaParam * Sema = SemaBuilder::CreateParam(AST);

	// Set Sema Type
	ASTType *ParamType = AST.getType();
	ParamType->accept(*this);
	Sema->setType(ParamType->getSema());

	AST.setSema(Sema);
}

void Resolver::visit(ASTComment &AST) {
	CurrentComment = SemaBuilder::CreateComment(AST);
}

void Resolver::visit(ASTBuiltinType &AST) {
	if (!AST.isVisited()) {

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
}

void Resolver::visit(ASTNamedType &AST) {
	if (!AST.isVisited()) {
		SemaType * Sema = Reg.LookupNamedType(AST, CurrentNameSpace);
		AST.setSema(Sema);
	}
}

void Resolver::visit(ASTArrayType &AST) {
	if (!AST.isVisited()) {

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
}

void Resolver::visit(ASTBreakStmt &AST) {
	// Do Nothing
}

void Resolver::visit(ASTContinueStmt &AST) {
	// Do Nothing
}

void Resolver::visit(ASTDeleteStmt &AST) {
	ASTExpr * Expr = AST.getExpr();

	Expr->accept(*this);
}

void Resolver::visit(ASTExprStmt &AST) {
	ASTExpr *Expr = AST.getExpr();

	Expr->accept(*this);
}

void Resolver::visit(ASTFailStmt &AST) {
	ASTExpr *Expr = AST.getExpr();

	if (Expr != nullptr) {
		Expr->accept(*this);
	}
}

void Resolver::visit(ASTHandleStmt &AST) {
	AST.getHandle()->accept(*this);
	ASTIdentifier *ErrorHandler = AST.getErrorHandlerRef();

	if (ErrorHandler != nullptr) {
		ErrorHandler->accept(*this);
	}
}

void Resolver::visit(ASTReturnStmt &AST) {
	SemaType *ReturnType = CurrentFunction->getReturnType(); // Force Return Expr to be of Return Type
	ASTExpr *Expr= AST.getExpr();
	if (Expr != nullptr) {
		Expr->accept(*this);
	} else if (!ReturnType->isVoid()) {
		Diag(AST.getLocation(), diag::err_invalid_return_type);
	}
}

void Resolver::visit(ASTRuleStmt &AST) {
	AST.getRule()->accept(*this);
	AST.getStmt()->accept(*this);
}

void Resolver::visit(ASTIfStmt &AST) {
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
}

void Resolver::visit(ASTSwitchStmt &AST) {
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
}

void Resolver::visit(ASTLoopStmt &AST) {
	if (AST.getRule()) { // Error: empty condition expr
		AST.getRule()->accept(*this);
	}

	// Check Init
	if (AST.getInit()) {
		AST.getInit()->accept(*this);
	}

	// Validate Rule Type
	// SemaValidator::CheckConvertibleTypes(LoopStmt->getRule()->getType(), SemaBuiltin::getBoolType());

	// Loop Statement
	AST.getStmt()->accept(*this);

	// Post Loop Statement
	if (AST.getPost()) {
		AST.getPost()->accept(*this);
	}
}

void Resolver::visit(ASTLoopInStmt &AST) {
	AST.getVarRef()->accept(*this);
    AST.getBlock()->accept(*this);
}

void Resolver::visit(ASTAssignStmt &AST) {
	AST.getSource()->accept(*this);
	AST.getTarget()->accept(*this);
}

void Resolver::visit(ASTBlockStmt &AST) {
	// Resolve LocalVar Type
	for (auto &VarEntry : AST.getLocalVars()) {
		ASTLocalVar *LocalVar = VarEntry.getValue();

		// Resolve LocalVar Type
		LocalVar->getType()->accept(*this);

		// Create LocalVar Sema
		SemaLocalVar * Sema = SemaBuilder::CreateLocalVar(*LocalVar);

		// Assign the Type Symbol to LocalVar
		if (LocalVar->getType() != nullptr && LocalVar->getType()->isVisited()) {
			Sema->Type = LocalVar->getType()->getSema();
		}

		// Add LocalVar to the Function Base LocalVars
		CurrentFunction->addLocalVar(LocalVar->getSema());
	}

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
}

void Resolver::visit(ASTIdentifier &AST) {
	ResolveExpr(AST);
}

void Resolver::visit(ASTMember &AST) {
	ResolveExpr(AST);
}

void Resolver::visit(ASTCall &AST) {
	ResolveExpr(AST);
}

void Resolver::visit(ASTUnaryOpExpr &AST) {
	ASTExpr *Expr = AST.getExpr();
	Expr->accept(*this);
    AST.setType(Expr->getType());
}

void Resolver::visit(ASTBinaryOpExpr &AST) {
	AST.getLeftExpr()->accept(*this);
    AST.getRightExpr()->accept(*this);

	// Check if Left and Right Expr are resolved
	SemaType * LeftType = AST.getLeftExpr()->getType();
	SemaType * RightType = AST.getRightExpr()->getType();

	if (AST.getTypeKind() == ASTBinaryOpTypeExprKind::OP_BINARY_ARITH ||
			AST.getTypeKind() == ASTBinaryOpTypeExprKind::OP_BINARY_COMPARISON) {

		// Check Compatible Types Bool/Bool, Float/Float, Integer/Integer
		if (SemaValidator::CheckArithTypes(LeftType, RightType)) {
			// Set respectively the Left or Right Expr Type by chose the Expr which is not a Value Type
			// Ex.
			// int a = 0
			// int b = a + 1
			// 1 will have type int
			if (AST.getLeftExpr()->getExprKind() == ASTExprKind::EXPR_VALUE &&
				AST.getRightExpr()->getExprKind() != ASTExprKind::EXPR_VALUE) {
				AST.getLeftExpr()->setType(RightType);
			} else if (AST.getRightExpr()->getExprKind() == ASTExprKind::EXPR_VALUE &&
				AST.getLeftExpr()->getExprKind() != ASTExprKind::EXPR_VALUE) {
				AST.getRightExpr()->setType(LeftType);
			}

			// Promotes First or Second Expr Types in order to be equal
			if (LeftType->isInteger()) {
				if (static_cast<SemaIntType *>(LeftType)->getIntKind() > static_cast<SemaIntType*>(RightType)->getIntKind())
					AST.setType(LeftType);
				else
					AST.setType(RightType);
			} else if (LeftType->isFloatingPoint()) {
				if (static_cast<SemaFloatType*>(LeftType)->getFPKind() >
					static_cast<SemaFloatType*>(RightType)->getFPKind())
					AST.setType(LeftType);
				else
					AST.setType(RightType);
			}

			AST.setType(AST.getTypeKind() == ASTBinaryOpTypeExprKind::OP_BINARY_ARITH ?
				LeftType : SemaBuiltin::getBoolType());
		} else {
			Diag(AST.getLocation(), diag::err_sema_types_operation)
					  << LeftType->getName()
					  << RightType->getName();
		}
	} else if (AST.getTypeKind() == ASTBinaryOpTypeExprKind::OP_BINARY_LOGIC) {
		if (SemaValidator::CheckLogicalTypes(LeftType, RightType)) {
			AST.setType(SemaBuiltin::getBoolType());
		} else {
			Diag(AST.getLocation(), diag::err_sema_types_logical)
				<< LeftType->getName()
				<< RightType->getName();
		}
	}
}

void Resolver::visit(ASTTernaryOpExpr &AST) {
	// Resolve Condition Expr
	AST.getConditionExpr()->accept(*this);
	SemaValidator::CheckConvertibleTypes(AST.getConditionExpr()->getType(), SemaBuiltin::getBoolType());

	// Resolve True and False Expr
	AST.getTrueExpr()->accept(*this);
	AST.getFalseExpr()->accept(*this);

	// Set Expr Type
	AST.setType(AST.getTrueExpr()->getType());
}

void Resolver::visit(ASTCast &AST) {
	AST.getType()->accept(*this);
    AST.getExpr()->accept(*this);
	// TODO: Validate Cast
}

void Resolver::visit(ASTBoolValue &AST) {
	SemaBoolValue *Sema = SemaBuilder::CreateBoolValue(AST);
	AST.setSema(Sema);
}

void Resolver::visit(ASTNumberValue &AST) {
	SemaValue *Sema = SemaBuilder::CreateNumberValue(AST);
	AST.setSema(Sema);
}

void Resolver::visit(ASTStringValue &AST) {
	SemaValue *Sema = SemaBuilder::CreateStringValue(AST);
	AST.setSema(Sema);
}

void Resolver::visit(ASTArrayValue &AST) {
	SemaArrayValue *Sema = SemaBuilder::CreateArrayValue(AST);
	for (auto Value : AST.getValues()) {
		// FIXME: resolve type of value
		Sema->Values.push_back(Value->getSema());
	}
	AST.setSema(Sema);
}

void Resolver::visit(ASTStructValue &AST) {
	SemaStructValue *Sema = SemaBuilder::CreateStructValue(AST);
	for (auto &Entry : AST.getValues()) {
		// FIXME: resolve type of value
		Sema->Values.insert(std::make_pair(Entry.getKey(), Entry.second->getSema()));
	}
	AST.setSema(Sema);
}

void Resolver::visit(ASTNullValue &AST) {
	SemaValue *Sema = SemaBuilder::CreateNullValue(AST);
	AST.setSema(Sema);
}

void Resolver::visit(ASTDefaultValue &AST) {
	if (CurrentStmt) {
		if (CurrentStmt->getStmtKind() == ASTStmtKind::STMT_ASSIGN) {
			ASTAssignStmt *Stmt = static_cast<ASTAssignStmt *>(CurrentStmt);

			// This AST Default Value is the only Expr in the Stmt
			if (Stmt->getTarget()->getExprKind() == ASTExprKind::EXPR_VALUE) {
				SemaType *T = Stmt->getSource()->getType();

				// Create the default value
				SemaValue *Sema = SemaBuilder::CreateDefaultValue(*T);
				AST.setSema(Sema);
			}
		}
	}
}

void Resolver::Resolver::EnterScope() {
	SymbolTable* NewScope = new SymbolTable(CurrentScope);
	CurrentScope = NewScope;
}

void Resolver::Resolver::ExitScope() {
	CurrentScope = CurrentScope->getParent();
}

void Resolver::ResetCurrent() {
	CurrentClass = nullptr;
	CurrentEnum = nullptr;
	CurrentFunction = nullptr;
	CurrentComment =nullptr;
	CurrentStmt = nullptr;
}

Resolver::~Resolver() {
}

void Resolver::Resolve() {
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
}

// TODO: remove Comment
// void Resolver::ResolveComment(SemaComment *Comment, ASTNode* AST) {
// 	if (AST->getKind() == ASTKind::AST_FUNCTION) {
// 		ASTFunction * Function = (ASTFunction *) AST;
// 		SemaValidator::CheckCommentParams(Comment, Function->getParams());
// 		SemaValidator::CheckCommentReturn(Comment, Function->getReturnTypeRef());
// 		SemaValidator::CheckCommentFail(Comment);
// 	}
// }

void Resolver::ResolveImports(SemaModule *Module) {
	for (auto Import : Module->getImports()) {
		// NameSpace, Class or Enum
		SemaNameSpace * NS = Reg.getNameSpace(Import->getTarget());
		Import->setSymbols(NS->getSymbols());
	}
}

/**
 * Resolve Module Function Definitions
 */
void Resolver::ResolveFunction(SemaFunction *Func) {
	ASTFunction &AST = Func->getAST();
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

	// Add to Body list for resolve in the next step
	// Enter Function Scope
	EnterScope();
	Reg.addBody(CurrentScope, AST.getBody());
	ExitScope();
}

void Resolver::ResolveClassType(SemaClassType *ClassType) {
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
}

void Resolver::ResolveBaseClasses(SemaClassType *DerivedClass) {
	// ClassDefinition Base Classes on first pass
	for (auto AST : DerivedClass->getAST().getBases()) {

		if (AST->getTypeKind() != ASTTypeKind::TYPE_NAMED) {
			// Error: cannot extend a type which differ from Class
			Diag(diag::err_syntax_error);
			return;
		}

		// Resolve the Base Sema
		SemaType *NamedType = Reg.LookupNamedType(*static_cast<ASTNamedType *>(AST), CurrentNameSpace);
		if (!NamedType->isClass()) {
			// Error: cannot extend a type which differ from Class
			Diag(diag::err_syntax_error);
			return;
		}

		// Resolve the Base Class Type
		SemaClassType *BaseClass = static_cast<SemaClassType *>(NamedType);
		ResolveClassType(BaseClass);

		// FIXME insert attribute in Derived Class?

		// Add Base Class to the list
		DerivedClass->BaseClasses.push_back(BaseClass);
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

	// Create Default Modifier
	llvm::SmallVector<ASTModifier *, 8> Modifiers;
	Modifiers.push_back(ASTBuilder::CreateModifier(SourceLocation(), ASTModifierKind::MOD_DEFAULT));

	// Create Class Method
	SemaClassMethod *Sema = SemaBuilder::CreateDefaultConstructor(CurrentClass);
	CurrentClass->addMethod(Sema); // Function Local var to be allocated
	CurrentFunction = Sema;
	CurrentClass->addConstructor(Sema);
}

void Resolver::ResolveEnumType(SemaEnumType *Enum) {
	for (auto &AST: Enum->getAST().getNodes()) {
		AST->accept(*this);
	}
}

void Resolver::ResolveBody(LocalScope &Scope) {
	CurrentScope = Scope.Symbols;
	Scope.Body->accept(*this);
}

void Resolver::ResolveExpr(ASTExpr &Expr) {
	// Do not resolve again
	if (Expr.isVisited()) {
		return;
	}
	Expr.setVisited(true);

	// Move Expr to Root Parent
	ASTExpr *Parent = &Expr;
	while (Parent != nullptr) {
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
}

void Resolver::ResolveParent(ASTIdentifier *AST) {

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
		return;
	}

	// ---------------------------------------
	// 5. Continue with children
	// ---------------------------------------
	if (AST->getChild()) {
		ResolveChild(Sema, AST->getChild());
	}
}

void Resolver::ResolveParent(ASTCall *AST) {

	// Resolve Expression in Arguments
	llvm::SmallVector<SemaType *, 8> ArgTypes = ResolveCallArgs(AST);

    // if Arguments are not resolved is not possible go ahead with call reference resolution
    // cannot resolve with the function parameters types
    std::string Mangled = Helper::MangleFunction(AST->getName(), ArgTypes);

	// Set as Resolved: TODO check if Resolve == false at start
	SemaCall *Sema = SemaBuilder::CreateCall(*AST);

	// Create a new instance using a Constructor
    if (AST->getCallKind() == ASTCallKind::CALL_NEW ||
        AST->getCallKind() == ASTCallKind::CALL_NEW_UNIQUE ||
        AST->getCallKind() == ASTCallKind::CALL_NEW_SHARED ||
        AST->getCallKind() == ASTCallKind::CALL_NEW_WEAK) {

        // Take the Type from the CurrentNameSpace
        SemaType *Type = Reg.LookupNamedType(AST->getName(), CurrentNameSpace);

    	// No type found, no constructor
    	if (Type == nullptr) {
    		Diag(AST->getLocation(), diag::err_unref_type);
    		return;
    	}

    	// Call Constructor
        if (!Type->isClass()) {
        	Diag(AST->getLocation(), diag::err_unref_type);
        	return;
        }

		SemaClassType *Class = static_cast<SemaClassType *>(Type);
		SemaClassMethod *Constructor = Class->getConstructors().lookup(Mangled);

    	if (Constructor == nullptr) {
    		// Error: func not found
    		Diag(AST->getLocation(), diag::err_syntax_error);
    		return;
    	}

        Sema->Function = Constructor;
    	Sema->Type = Type;
    } else {

    	// Call a Function or a Class Method
    	SemaFunctionBase *Func = nullptr;
    	if (CurrentFunction->getKind() == SemaKind::METHOD) {

    		// Check if the Call is a Base Class Constructor Method
    		SemaClassType *CurrentClass = static_cast<SemaClassMethod *>(CurrentFunction)->getClass();
    		SemaType * T = Reg.LookupNamedType(AST->getName(), CurrentNameSpace);

    		if (T->isClass()) {
				SemaClassType * C = static_cast<SemaClassType *>(T);

    			// Call to Base Class Constructor Method
    			if (C->isBaseOrEquals(CurrentClass)) {
    				// Resolve Call with Class Constructor if is not private
    				SemaClassMethod *Constructor = C->getConstructors().lookup(Mangled);

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
    				SemaClassMethod *Method = C->getMethods().lookup(Mangled);

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
    		Func = Reg.LookupFunction(Mangled, CurrentNameSpace);
    	}

    	if (Func == nullptr) {
    		// Error: func not found
    		Diag(AST->getLocation(), diag::err_syntax_error);
    		return;
    	}

        Sema->Function = Func;
    	Sema->Type = Func->getReturnType();

    	// Continue by Resolving Child
    	if (AST->getChild()) {
    		ResolveChild(Sema, AST->getChild());
    	}
    }

	// Set the Call Sema ErrorHandler
	ResolveErrorHandler(Sema);
	AST->setSema(Sema);
}

void Resolver::ResolveChild(SemaNode *Parent, ASTExpr *AST) {

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

	Diag(diag::err_invalid_behavior);
}

void Resolver::ResolveChild(SemaNameSpace *NameSpace, ASTExpr *AST) {

	// Resolve as NameSpace Function
	if (AST->getExprKind() == ASTExprKind::EXPR_CALL) {
		ASTCall *Call = static_cast<ASTCall *>(AST);
		SmallVector<SemaType *, 8> CallTypes = ResolveCallArgs(Call);
		std::string Mangled = Helper::MangleFunction(Call->getName(), CallTypes);
		SemaCall *Sema = SemaBuilder::CreateCall(*Call);

		// Lookup FUnction
		Symbol * Sym = NameSpace->getSymbols()->lookup(Mangled);
		SemaFunction * Function = static_cast<SemaFunction *>(Sym->getRef());
		Sema->setFunction(Function);

		// Set Sema
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

	Diag(diag::err_invalid_behavior);
}

void Resolver::ResolveChild(SemaClassType *ClassType, ASTExpr *AST) {

	// Resolve as Static Class Method
	if (AST->getExprKind() == ASTExprKind::EXPR_CALL) {
		ASTCall *Call = static_cast<ASTCall *>(AST);

		// Resolve Call as Class Method
		SmallVector<SemaType *, 8> CallTypes = ResolveCallArgs(Call);
		std::string Mangled = Helper::MangleFunction(Call->getName(), CallTypes);

		// Create a call to class method
		SemaClassMethod* Method = ClassType->getMethods().lookup(Mangled);
		if (Method) {

			// Check if Method belongs to a super class or is static
			if (!ClassType->isDerivedOrEquals(Method->getClass()) && !Method->isStatic()) {
				// Error: method cannot be called statically
				Diag(Call->getLocation(), diag::err_syntax_error) << Call->getName();
			}

			SemaCall *Sema = SemaBuilder::CreateCall(*Call);
			Sema->setFunction(Method); // Set the Call Sema Function
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

	Diag(diag::err_invalid_behavior);
}

void Resolver::ResolveChild(SemaEnumType *EnumType, ASTExpr *AST) {

	// Resolve as Enum Entry
    if (AST->getExprKind() == ASTExprKind::EXPR_MEMBER) {
    	ASTMember *Member = static_cast<ASTMember *>(AST);
    	SemaVar *Entry = EnumType->getEntries().lookup(Member->getName());

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

    	Member->setType(EnumType);
    	Member->setSema(Entry);
    	return;
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

SemaCall *Resolver::ResolveChildCall(SemaResult *Parent, ASTCall *AST) {

	// set Mangled Identifier
	SmallVector<SemaType *, 8> CallTypes = ResolveCallArgs(AST);
	std::string MangledIdentifier = Helper::MangleFunction(AST->getName(), CallTypes);

	// Build Sema
	SemaCall *Sema = SemaBuilder::CreateCall(*AST);
	AST->setSema(Sema);
	AST->setVisited(true);

	// Check Parent Type is a Class
	SemaType *ParentType = Parent->getType();
	if (!ParentType->isClass()) {
		Diag(diag::err_invalid_behavior);
		return nullptr;
	}

	// Search Method into Class Scope
	SemaClassType *ClassType = static_cast<SemaClassType *>(ParentType);
	Symbol * Sym = ClassType->getSymbols()->lookup(MangledIdentifier);

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
	Sema->setFunction(Method);

	// Set the Call Sema ErrorHandler
	ResolveErrorHandler(Sema);

	// Look for child
	if (AST->getChild())
		ResolveChild(Sema, AST->getChild());

	return Sema;
}

SemaVar * Resolver::ResolveChildMember(SemaResult *Parent, ASTMember *AST) {

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
		SemaMemberVar *Member = SemaBuilder::CreateMemberVar(*Attribute->getAST(), *Parent);
		Member->setClassAttribute(Attribute);

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
	AST->setVisited(true);

	return Sema;
}

llvm::SmallVector<SemaType *, 8> Resolver::ResolveCallArgs(ASTCall *AST) {
	llvm::SmallVector<SemaType *, 8> Types;
	for (auto Arg : AST->getArgs()) {
		Arg->getExpr()->accept(*this);
		SemaType *Type = Arg->getExpr()->getType();
		if (Type)
			Types.push_back(Type);
	}
	return Types;
}

llvm::SmallVector<SemaType *, 8> Resolver::ResolveParams(ASTFunction &AST) {
	llvm::SmallVector<SemaType *, 8> Types;
	for (auto Param : AST.getParams()) {
		Param->accept(*this);
		SemaType *Type = Param->getType()->getSema();
		if (Type)
			Types.push_back(Type);
	}
}

void Resolver::ResolveErrorHandler(SemaCall *Sema) {
	// Search until parent is null or parent is a Handle Stmt
	// When Parent Stmt is nullptr assign Function ErrorHandler to Call ErrorHandler
	ASTStmt *Parent = CurrentStmt;
	while (Sema->getErrorHandler() == nullptr) {
		Parent = Parent->getParent();
		if (Parent == nullptr) {
			Sema->ErrorHandler = CurrentFunction->getErrorHandler();
		} else if (Parent->getStmtKind() == ASTStmtKind::STMT_HANDLE) {
			ASTHandleStmt *HandleStmt = static_cast<ASTHandleStmt*>(Parent);
			if (HandleStmt->getErrorHandlerRef() != nullptr) {
				Sema->ErrorHandler = reinterpret_cast<SemaErrorHandler *>(HandleStmt->getErrorHandlerRef()->getSema());
			}
		}
	}
}
