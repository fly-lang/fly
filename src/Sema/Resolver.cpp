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
#include "Basic/Logger.h"
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
#include "AST/ASTEnum.h"
#include "AST/ASTType.h"
#include "AST/ASTModule.h"
#include "AST/ASTArg.h"
#include "AST/ASTIfStmt.h"
#include "AST/ASTImport.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTVar.h"
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
	// Enter Function Scope
	EnterScope();

	// Create Sema Function or Method
	if (AST.getKind() == ASTKind::AST_FUNCTION) {

		// Create Sema Function
		CurrentFunction = SemaBuilder::CreateFunction(*CurrentModule, CurrentScope, AST);
	} else if (AST.getKind() == ASTKind::AST_METHOD) {

		// Methods must be defined inside a Class
		CurrentFunction = SemaBuilder::CreateClassMethod(CurrentClass, AST, *CurrentComment);
	} else {

		Diag(AST.getLocation(), diag::err_invalid_behavior);
		return;
	}

	// // Add to Symbol Table of Parent Scope (Module or Class)
	Symbol *Sym = new Symbol(AST.getName(), CurrentFunction->getKind(), CurrentFunction);
	CurrentScope->getParent()->insert(Sym);

	ExitScope();
}

void Resolver::visit(ASTClass &AST) {
	// Enter Class Scope
	EnterScope();

	// Create Sema Class
	CurrentClass = SemaBuilder::CreateClass(*CurrentModule, CurrentScope, AST);

	// Add to Symbol Table of Parent Scope (Module)
	Symbol *Sym = new Symbol(AST.getName(), SemaKind::CLASS, CurrentClass);
	CurrentScope->getParent()()->insert(Sym);

	// Add attributes or methods
	for (auto Def : AST.getNodes()) {
		if (Def->getKind() == ASTKind::AST_VAR)
			Def->setKind(ASTKind::AST_ATTRIBUTE);
		else if (Def->getKind() == ASTKind::AST_FUNCTION)
			Def->setKind(ASTKind::AST_METHOD);
		Def->accept(*this);
	}
	ExitScope();
}

void Resolver::visit(ASTEnum &AST) {
	// Enter Enum Scope
	EnterScope();

	// Create Sema Enum
	CurrentEnum = SemaBuilder::CreateEnum(*CurrentModule, CurrentScope, AST);

	// Add to Symbol Table of Parent Scope (Module)
	Symbol *Sym = new Symbol(AST.getName(), SemaKind::ENUM, CurrentEnum);
	CurrentScope->getParent()->insert(Sym);

	// Add enum entries
	for (auto Def : AST.getNodes()) {
		Def->setKind(ASTKind::AST_ENUM_ENTRY);
		Def->accept(*this);
	}
	ExitScope();
}


void Resolver::visit(ASTVar &AST) {
	SemaVar *Var = nullptr;
	if (AST.getKind() == ASTKind::AST_VAR) {
		Var = SemaBuilder::CreateLocalVar(AST);
	} else if (AST.getKind() == ASTKind::AST_ATTRIBUTE) {
		Var = SemaBuilder::CreateClassAttribute(*CurrentClass, AST, *CurrentComment);
	} else if (AST.getKind() == ASTKind::AST_ENUM_ENTRY) {
		Var = SemaBuilder::CreateEnumEntry(*CurrentEnum, AST, CurrentComment);
	} else {
		Diag(AST.getLocation(), diag::err_invalid_behavior);
		return;
	}

	// Add to Symbol Table
	Symbol *Sym = new Symbol(AST.getName(), Var->getKind(), Var);
	CurrentScope->insert(Sym);
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
		SemaType * Sema = Reg.LookupNamedType(AST.getNames(), CurrentNameSpace);
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
	ResolveRef(AST.getParent(), AST.getVarRef());
}

void Resolver::visit(ASTExprStmt &AST) {
	ASTExpr * Expr = AST.getExpr();

	Expr->accept(*this);
}

void Resolver::visit(ASTFailStmt &AST) {
	ASTExpr *Expr= AST.getExpr();

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

void Resolver::visit(ASTVarStmt &AST) {
	AST.getVarRef()->accept(*this);
	AST.getExpr()->accept(*this);
}

void Resolver::visit(ASTBlockStmt &AST) {
	// Resolve LocalVar Type
	for (auto &VarEntry : AST.getLocalVars()) {
		ASTVar *LocalVar = VarEntry.getValue();

		// Resolve LocalVar Type
		LocalVar->getTypeRef()->accept(*this);

		// Create LocalVar Sema
		SemaLocalVar * Sema = SemaBuilder::CreateLocalVar(*LocalVar);

		// Assign the Type Symbol to LocalVar
		if (LocalVar->getTypeRef() != nullptr && LocalVar->getTypeRef()->isVisited()) {
			Sema->Type = LocalVar->getTypeRef()->getSema();
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
	// Validate ref
	// SemaValidator::CheckVar(Stmt, VarRef);

	// TODO: Resolve Ref
}


void Resolver::visit(ASTMember &AST) {
	// TODO
}


void Resolver::visit(ASTCall &AST) {
	// Validate Call
	// SemaValidator::CheckCall(CurrentStmt, AST);

	// TODO: Resolve Call
	switch (AST.getCallKind()) {

		// Call a Function
		case ASTCallKind::CALL_DIRECT:
			AST.setType(AST.getSema()->getFunction()->getReturnType());

		// Call a Constructor Method
		case ASTCallKind::CALL_NEW: {
			SemaClassMethod *Method = static_cast<SemaClassMethod *>(AST.getSema()->getFunction());
			AST.setType(Method->getClass());
		}
	}

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
	// TODO
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
	for (auto Entry : AST.getValues()) {
		// FIXME: resolve type of value
		Sema->Values.insert(std::make_pair(Entry.getKey(), Entry.second->getSema()));
	}
	AST.setSema(Sema);
}

void Resolver::visit(ASTNullValue &AST) {
	SemaValue *Sema = SemaBuilder::CreateNullValue(AST);
	AST.setSema(Sema);
}

void Resolver::Resolver::EnterScope() {
	SymbolTable* NewScope = new SymbolTable(CurrentScope);
	CurrentScope = NewScope;
}

void Resolver::Resolver::ExitScope() {
	CurrentScope = CurrentScope->getParent();
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
				case SemaKind::ENUM:
					ResolveEnumType(static_cast<SemaEnumType *>(Node));
					break;
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
	CurrentScope = Func->getSymbols();

	// Resolve Return Type
	ASTType *ReturnType = AST.getReturnTypeRef();
	ReturnType->accept(*this);
	Func->setReturnType(ReturnType->getSema());

	// Resolve Parameters Types
	for (auto Param : AST.getParams()) {

		// resolve parameter type
		ASTType *ParamType = Param->getTypeRef();
		ParamType->accept(*this);

		// Create Sema Param
		SemaParam * P = SemaBuilder::CreateParam(*Param);
		P->setType(ParamType->getSema());
		Func->addParam(P);
	}

	// Add to Body list for resolve in the next step
	// Enter Function Scope
	EnterScope();
	Reg.addBody(CurrentScope, AST.getBody());
	ExitScope();
}

void Resolver::ResolveClassType(SemaClassType *ClassType) {
	// Create Class Default Constructor
	// Create the default constructor if no constructors are defined
	if (ClassType->getClassKind() != SemaClassKind::INTERFACE && ClassType->getConstructors().empty())
		this->CreateDefaultConstructor();

	// Resolve Base Classes
	this->ResolveBaseClasses(ClassType);

	// ClassDefinition Class Attributes
	this->SetDefaultValueInAttributes();

	for (auto &Node: ClassType->getNodes()) {
		switch (Node->getKind()) {
			case SemaKind::ATTRIBUTE:
				ResolveClassAttribute(static_cast<SemaClassAttribute *>(Node));
				break;
			case SemaKind::METHOD:
				ResolveClassMethod(static_cast<SemaClassMethod *>(Node));
				break;
			default:
				Diag(diag::err_invalid_behavior);
		}
	}
}


void Resolver::ResolveBaseClasses(SemaClassType *DerivedClass) {
	// ClassDefinition Base Classes on first pass
	for (auto &BaseTypeRef : DerivedClass->getAST().getBaseClasses()) {

		// TODO: Recursively add definition from base classes
		// ResolveBaseClasses(BaseClassType);

		// Search for the SuperClass in the Module, CurrentNameSpace or Imports
		if (ResolveTypeRef(BaseTypeRef)) {
			SemaType *BaseType = BaseTypeRef->getSema();

			if (BaseType->getTypeKind() != SemaTypeKind::TYPE_CLASS) {
				// Error: invalid superclass type
				Diag(BaseTypeRef->getLocation(), diag::err_syntax_error);
				break;
			}

			SemaClassType *BaseClassType= static_cast<SemaClassType *>(BaseType);

			// Create the Class Instance to be added to Base Instance Children
			for (auto AST : BaseClassType->getAST().getNodes()) {
				switch (AST->getKind()) {

					// ClassDefinition Class Var: Attribute
					case ASTKind::AST_VAR: {
						ASTVar * Var = static_cast<ASTVar *>(AST);

						// Define an Attribute
						SemaClassAttribute *Attribute = DefineAttribute(Var, nullptr);

						// Inherit only protected or public Attributes (not Static)
						if (CanInheritAttribute(Attribute)) {
							Var->Sema->Type = Attribute->getType();
							Class->Attributes.insert(std::make_pair(Var->getName(), Attribute));
						}
					}
					break;

					// ClassDefinition Class Function: Method or Constructor
					case ASTKind::AST_FUNCTION: {
						ASTFunction *Function = static_cast<ASTFunction *>(AST);

						// Define a Method
						//SemaClassMethod *Method = DefineMethod(AST, Function, nullptr);

						// Inherit only Methods (not Constructors)
						// if (!Method->isConstructor() && CanInheritMethod(Method)) {
						// 	Class->Methods.insert(std::make_pair(Method->getMangledName(), Method));
						// }

						// Set Overridden Methods
						// if (!Method->isConstructor()) {
                        // 	auto It = Class->getMethods().find(Method->getMangledName());
					}
					break;

					// ClassDefinition Class Comment
					case ASTKind::AST_COMMENT:
						// No action needed
							break;

					default:
						// Error: invalid declaration in class
							Diag(AST->getLocation(), diag::err_syntax_error);
					break;
				}
			}

			// Add Base Class to the list
			DerivedClass->BaseClasses.push_back(BaseClassType);
		}
	}
}

bool Resolver::CanInheritMethod(SemaClassMethod *BaseMethod) {
	// Add Methods from Super SuperClassType
	// Check if Method Visibility is not private and not static
	if (BaseMethod->getVisibility() > SemaVisibilityKind::PRIVATE && !BaseMethod->isStatic()) {

		// Check Methods already exists and type conflicts in Super Methods
		auto It = CurrentClass->getMethods().find(BaseMethod->getMangledName());
		if (It == CurrentClass->getMethods().end()) { // Not Found, add new Method
			return true;
		} else { // Duplicate Found, check conflicts

			// Check Return Type conflicts
			if (It->second->getReturnType() != BaseMethod->getReturnType()) {
				Diag(It->second->getAST().getLocation(), diag::err_syntax_error);
			}

			// Check Visibility conflicts
			if (It->second->getVisibility() < BaseMethod->getVisibility()) {
				Diag(It->second->getAST().getLocation(), diag::err_syntax_error);
			}

			// Check Static conflicts
			if (It->second->isStatic()) {
				Diag(It->second->getAST().getLocation(), diag::err_syntax_error);
			}

			// If the inherited method appear in more than one super class: need to be re-defined
			// Check if class methods contains the same inherited method without redefine it
			if (CurrentClass->getName() != It->getValue()->getClass()->getName()) {

                // Error: method already exists in super class
                Diag(BaseMethod->getAST().getLocation(), diag::err_syntax_error);
            }

			return false;
		}
	}
}

bool Resolver::CanInheritAttribute(SemaClassAttribute *BaseAttribute) {
	// Check if Attribute Visibility is not private and not static
	if (BaseAttribute->getVisibility() > SemaVisibilityKind::PRIVATE && !BaseAttribute->isStatic()) {

		// Check Attribute already exists and type conflicts in Super Vars
		auto It = CurrentClass->Attributes.find(BaseAttribute->getAST().getName());
		if (It == CurrentClass->Attributes.end()) { // Not Found
			return true;
		} else { // Duplicate Found

			// Check Type conflicts
			if (!It->second->getType()->isEquals(BaseAttribute->getType())) {
				Diag(It->second->getAST()->getLocation(), diag::err_syntax_error);
			}

			// Check Visibility conflicts
			if (It->second->getVisibility() < BaseAttribute->getVisibility()) {
				Diag(It->second->getAST()->getLocation(), diag::err_syntax_error);
			}

			// Check Constant conflicts
			if (!It->second->isConstant() && BaseAttribute->isConstant()) {
				Diag(It->second->getAST()->getLocation(), diag::err_syntax_error);
			}

			// Check Static conflicts
			if (It->second->isStatic()) {
				Diag(It->second->getAST()->getLocation(), diag::err_syntax_error);
			}

			return false;
		}
	}
}

void Resolver::CreateDefaultConstructor() {

	// Create Default Modifier
	llvm::SmallVector<ASTModifier *, 8> Modifiers;
	Modifiers.push_back(ASTBuilder::CreateModifier(SourceLocation(), ASTModifierKind::MOD_DEFAULT));

	llvm::SmallVector<ASTVar *, 8> Params;
	ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLocation());
	ASTFunction * AST = ASTBuilder::CreateClassMethod(CurrentClass->getAST().getLocation(), CurrentClass->getAST(),
											   nullptr, CurrentClass->getAST().getName(), Modifiers, Params, Body);

	// Call default constructor of the super classes (if exists)
	// if (!Class->getBaseClasses().empty()) {
	// 	for (auto &BaseClass : Class->getBaseClasses()) {
 //
	// 		// Create Call to Base Constructor
	// 		if (BaseClass->DefaultConstructor) {
	// 			SemaClassMethod * DefaultConstructor = BaseClass->DefaultConstructor;
	// 			llvm::SmallVector<ASTExpr *, 8> Args;
	// 			SourceLocation Loc = SourceLocation();
 //
	// 			// TODO: Create Call to Base Constructor
	// 			// ASTCall *BaseCall = S.getASTBuilder().CreateCall(DefaultConstructor->getAST()->getName(), Args);
	// 			// SemaBuilderStmt * ExprStmt = S.getASTBuilder().CreateExprStmt(Body, Loc);
	// 			// ExprStmt->setExpr(S.getASTBuilder().CreateExpr(BaseCall));
	// 		} else {
 //                // Error: Base Class has no default constructor
 //                S.Diag(BaseClass->getAST()->getLocation(), diag::err_syntax_error);
 //            }
	// 	}
	// }

	// Create the default constructor
	SemaClassMethod *Constructor = SemaBuilder::CreateClassMethod(CurrentClass, CurrentClass->This, AST, nullptr);

	CurrentClass->Constructors.insert(std::make_pair(Constructor->getMangledName(), Constructor));
}

void Resolver::SetDefaultValueInAttributes() {
	// Set default values in attributes
	for (auto &AttributeEntry : CurrentClass->getAttributes()) {
		SemaClassAttribute *Attribute = AttributeEntry.getValue();

		// Generate default values
		if (Attribute->getAST()->getExpr() == nullptr) {
			ResolveTypeRef(Attribute->getAST()->getTypeRef());

			// Create default Sema Value
			SemaValue *Sema = SemaBuilder::CreateDefaultValue(*Attribute->getType());
			ASTValue *Value = ASTBuilder::CreateDefaultValue();
			Value->setSema(Sema);

			// Set AST Attribute with Expr
			ASTExpr *Expr = ASTBuilder::CreateExpr(Value);
			Expr->setType(Attribute->getType());
			Attribute->getAST()->setExpr(Expr);
		} else {

			// Check Value is default value expression
			SemaValidator::CheckIsValueExpr(Attribute->getAST()->getExpr());
		}
	}
}

void Resolver::ResolveClassAttribute(SemaClassAttribute *Attribute) {
	ASTType *AST = Attribute->getAST().getTypeRef();
	AST->accept(*this);
	Attribute->setType(AST->getSema());
}

void Resolver::ResolveClassMethod(SemaClassMethod *Method) {
	ASTFunction &AST = Method->getAST();

	// Resolve Return Type
	ASTType *ReturnType = AST.getReturnTypeRef();
	ReturnType->accept(*this);
	Method->setReturnType(ReturnType->getSema());

	// Resolve Parameters Types
	for (auto Param : AST.getParams()) {

		// resolve parameter type
		ASTType *ParamType = Param->getTypeRef();
		ParamType->accept(*this);

		// Create Sema Param
		SemaParam * P = SemaBuilder::CreateParam(*Param);
		P->setType(ParamType->getSema());
		Method->addParam(P);
	}

	// Add to Body list for resolve in the next step
	// Enter Function Scope
	EnterScope();
	Reg.addBody(CurrentScope, AST.getBody());
	ExitScope();
}

void Resolver::ResolveEnumType(SemaEnumType *Enum) {
	for (auto &Node: Enum->getNodes()) {
		switch (Node->getKind()) {

			case SemaKind::ENUM_ENTRY:
				ResolveEnumEntry(static_cast<SemaEnumEntry *>(Node));
				break;
			default:
				Diag(diag::err_invalid_behavior);
		}
	}
}

void Resolver::ResolveEnumEntry(SemaEnumEntry *Node) {

}

void Resolver::ResolveBody(LocalScope &Scope) {
	CurrentScope = Scope.Symbols;
	Scope.Body->accept(*this);
}

SemaType *Resolver::ResolveTypeRef(ASTType *&TypeRef) {
	if (!TypeRef->isVisited()) {

		// Set current with the Top Parent
		ASTIdentifier *Current = TypeRef;
		while (Current->getParent() != nullptr) {
			Current->getParent()->setChild(Current);
			Current = Current->getParent();
		}

		// Ref is a CurrentNameSpace ?
		SemaNameSpace * CurrentNameSpace = ResolveNameSpace(Current);

		// Resolve from top-bottom
		if (TypeRef->getSema()) {
			// TypeRef is already resolved
			TypeRef->setVisited(true);
			return TypeRef->getSema();
		}

		// TypeRef is an Array
		if (TypeRef->isArray()) {
			auto ArrayTypeRef = static_cast<ASTArrayType *>(TypeRef);
			return ResolveTypeRef(ArrayTypeRef->ElementType);
		}

		// Type is Class or Enum
		TypeRef->setSema(ResolveType(TypeRef->getName(), CurrentNameSpace));

		// Take Identity from CurrentNameSpace
		TypeRef->setVisited(TypeRef->getSema() != nullptr); // Evict Cycle Loop: can be resolved only now
	}

	if (!TypeRef->getSema()) {
		Diag(TypeRef->getLocation(), diag::err_unref_type);
		return false;
	}

	return TypeRef->getSema();
}

/**
 * Resolve a Reference, continue to resolve until the Ref is completely resolved
 * @param Stmt
 * @param Ref
 * @param NameSpaces
 * @param ...
 * @return
 */
void Resolver:: ResolveFromTopRef(ASTStmt *Stmt, ASTIdentifier *Ref, SemaNameSpace *CurrentNameSpace) {
	if (!Ref->isVisited()) {

		// Ref is a Function
		if (Ref->isCall()) {
			ASTCall *CallRef = static_cast<ASTCall *>(Ref);
			SemaCall *Call = ResolveCall(Stmt, CallRef, CurrentNameSpace);

			if (CallRef->getChild())
				ResolveInstanceRef(Stmt, CallRef->getChild(), Call);
		}

		//Ref is a Var
		else if (Ref->isVarRef()) {
			SemaVar *Sema = ResolveVar(Stmt, Ref);

			if (Ref->getChild())
				ResolveInstanceRef(Stmt, Ref->getChild(), Sema);
		}

		// Ref is a Class or an Enum Type?
		else {
			SemaType *Type = ResolveType(Ref->getName(), CurrentNameSpace);

			// Call can be a local or base class constructor method
			if (Ref->getChild()) {

				// Check if Type is a Current or Base Class of the current Method
				// class TestClass : BaseClass, BaseClass2 {
				//   void do() {
				//		BaseClass.do()
				//      or
				//      TestClass.do()
				//   }
				// }
				if (Type->isClass() && CurrentEnum->getKind() == SemaKind::METHOD &&
					static_cast<SemaClassType *>(Type)->isBaseOrEquals(static_cast<SemaClassMethod *>(CurrentFunction)->getClass())) {

					// Get the Class Instance related to Class Type
					// TestClass.do()  TestClass->this  TestClass.do(TestClass->this)
					// BaseClass.do()  BaseClass->this
					SemaClassInstance *This = static_cast<SemaClassMethod *>(CurrentFunction)->getClass().getThis();

					// Resolve instance as Class Instance of the current class or base class
					ResolveInstanceRef(Stmt, Ref->getChild(), This);

				} else {

					// Resolve a Static Ref to a Class or Enum
					ResolveStaticRef(Stmt, Ref->getChild(), Type);
				}
			}
		}
	}
}

/**
 * Resolve static Ref to a Class or Enum
 * @param Type
 * @param Ref
 * @return
 */
void Resolver:: ResolveStaticRef(ASTStmt *Stmt, ASTIdentifier *Ref, SemaType *Type) {
	if (!Ref->isVisited()) {

		// Class
		if (Type->isClass()) {
			SemaClassType *ClassType = static_cast<SemaClassType *>(Type);

			// class method
			if (Ref->isCall()) {
				ASTCall *Call = static_cast<ASTCall *>(Ref);

				// Set as Resolved
				Call->setVisited(true);

				SmallVector<SemaType *, 8> CallTypes = ResolveCallArgTypes(Stmt, Call);
				std::string Mangled = SemaFunctionBase::MangleFunction(Call->getName(), CallTypes);
				SemaClassMethod* Method = ClassType->getMethods().lookup(Mangled);

				// Create a call to class method
				if (Method) {

					// Check if Method belongs to a super class or is static
					if (!ClassType->isDerivedOrEquals(Method->getClass()) && !Method->isStatic()) {
                    	// Error: method cannot be called statically
                    	Diag(Ref->getLocation(), diag::err_syntax_error) << Ref->getName();
                    }

					SemaCall *Sema = SemaBuilder::CreateCall(*Call);
					Sema->setFunction(Method); // Set the Call Sema Function
					// Sema->setParent(nullptr); // Parent is not set, because this is a static call
					Call->setSema(Sema);

					// Set the Call Sema ErrorHandler
					ResolveErrorHandler(Stmt, Call->getSema());

					if (Ref->getChild())
						ResolveInstanceRef(Stmt, Ref->getChild(), Call->getSema());
				}
			}

			// class attribute
			else {

				// Set as Resolved
				Ref->setVisited(true);

				SemaClassAttribute *Attr = ClassType->getAttributes().lookup(Ref->getName());
				if (Attr) {

					if (!Attr->isStatic()) {
						// Error: cannot resolve a non-static attribute without a parent
						Diag(Ref->getLocation(), diag::err_syntax_error) << Ref->getName();
					}

					// Resolve a static attribute
					Ref->setSema(Attr);
					// Attr->setParent(nullptr); // Parent is not set, because this is a static attribute

					if (Ref->getChild())
						ResolveInstanceRef(Stmt, Ref->getChild(), Attr);
				}
			}
		}

		// Enum
		else if (Type->isEnum()) {
			ResoveEnumRef(Stmt, Ref, static_cast<SemaEnumType *>(Type));
		}
	}
}

/**
 * Resolve static Ref to a Class or Enum
 * @param Type
 * @param Ref
 * @return
 */
void Resolver:: ResolveInstanceRef(ASTStmt *Stmt, ASTIdentifier *Ref, SemaResult *Parent) {
	if (!Ref->isVisited()) {

		// Class
		if (Parent->getType()->isClass()) {
			SemaClassType *ClassType = static_cast<SemaClassType *>(Parent->getType());

			// class method
			if (Ref->isCall()) {
				ASTCall *Call = static_cast<ASTCall *>(Ref);

				// Set as Resolved
				Call->setVisited(true);

				SmallVector<SemaType *, 8> CallTypes = ResolveCallArgTypes(Stmt, Call);
				std::string Mangled = SemaFunctionBase::MangleFunction(Call->getName(), CallTypes);

				// Search method in the class instance
				SemaClassMethod* CalledMethod = ClassType->getMethods().lookup(Mangled);

				// Search method in the class instance base classes
				if (!CalledMethod) {
					for (SemaClassType *BaseClass : ClassType->getBaseClasses()) {
						while (BaseClass && !CalledMethod) {
							CalledMethod = BaseClass->getMethods().lookup(Mangled);
							// If not found, go up the inheritance chain of this base class
							const auto &NextBases = BaseClass->getBaseClasses();
							BaseClass = !NextBases.empty() ? NextBases.front() : nullptr;
						}
						if (CalledMethod) break;
					}
                }

				// Create a call to class method
				if (CalledMethod) {
					SemaCall *Sema = SemaBuilder::CreateCall(*Call);

					// Set the Call Sema Function
					Sema->setFunction(CalledMethod);
					// Call->Sema->setParent(Parent); // Set First and Parent

					Call->setSema(Sema);

					// Set the Call Sema ErrorHandler
					ResolveErrorHandler(Stmt, Sema);

					if (Ref->getChild())
						ResolveInstanceRef(Stmt, Ref->getChild(), Sema);
				}
			}

			// class attribute
			else {

				// Set as Resolved
				Ref->setVisited(true);

				SemaClassAttribute *Attr = ClassType->getAttributes().lookup(Ref->getName());
				if (Attr) {

					SemaMemberVar *Sema;
					if (Attr->isStatic()) {
						Ref->setSema(Attr);
					} else {
						SemaMemberVar *Member = SemaBuilder::CreateMemberVar(Attr->getAST(), *Parent);
						Member->Type = Attr->getType();
						Member->ClassAttribute = Attr;
						Ref->setSema(Member);
					}

					if (Ref->getChild())
						ResolveInstanceRef(Stmt, Ref->getChild(), Sema);
				}
			}
		}

		// Enum
		else if (Parent->getType()->isEnum()) {
			ResoveEnumRef(Stmt, Ref, static_cast<SemaEnumType *>(Parent->getType()));
		}
	}
}

void Resolver::ResoveEnumRef(ASTStmt *Stmt, ASTIdentifier *Ref, SemaEnumType *EnumType) {

	// Set as Resolved
	Ref->setVisited(true);

	// Enum Entry
	SemaVar *Entry = EnumType->getEntries().lookup(Ref->getName());
	if (Entry) {
		Ref->setSema(Entry);
	}
}

ASTIdentifier *Resolver::getParentRef(fly::ASTIdentifier *Ref) {
	// Set current with the Top Parent
	ASTIdentifier *Parent = Ref;
	while (Parent->getParent() != nullptr) {
		Parent->getParent()->setChild(Parent);
		Parent = Parent->getParent();
	}

	return Parent;
}


bool Resolver::ResolveRef(ASTStmt *Stmt, ASTCall *Call) {
	if (!Call->isVisited()) {

		// Get Parent Ref
		ASTIdentifier *Parent = getParentRef(Call);

		// Ref is a CurrentNameSpace ?
		SemaNameSpace * CurrentNameSpace = ResolveNameSpace(Parent);

		// Resolve from top-bottom
		ResolveFromTopRef(Stmt, Parent, CurrentNameSpace);
	}

	return Call->getSema() != nullptr;
}

bool Resolver::ResolveRef(ASTStmt *Stmt, ASTIdentifier *VarRef) {
	if (!VarRef->isVisited()) {

		// Get Parent Ref
		ASTIdentifier *Parent = getParentRef(VarRef);

		// Ref is a CurrentNameSpace ?
		SemaNameSpace * CurrentNameSpace = ResolveNameSpace(Parent);

		// Resolve from top-bottom
		ResolveFromTopRef(Stmt, Parent, CurrentNameSpace);
	}

	return VarRef->getSema() != nullptr;
}

SemaNameSpace *Resolver::ResolveNameSpace(ASTIdentifier *Ref) {
	// Ref is the current module namespace
	if (Ref->getName() == CurrentNameSpace->getName()) {
		return CurrentNameSpace;
	}

	// Import CurrentNameSpace
	SemaNameSpace *CurrentNameSpace = nullptr;
	SemaNameSpace *ChildNameSpace = nullptr;
	std::string NameSpaceStr = "";
	ASTIdentifier *Child = Ref;
	while (Child) {
		NameSpaceStr = NameSpaceStr.empty() ? Child->getName().data() : NameSpaceStr + "." + Child->getName().data();
		ChildNameSpace = Registry::Lookup(CurrentModule->getI, NameSpaceStr);
		if (ChildNameSpace) {
			CurrentNameSpace = ChildNameSpace;
		}
		Child = Child->getChild();
	}

	if (CurrentNameSpace)
		return CurrentNameSpace;

	return CurrentNameSpace;
}

SemaType * Resolver::ResolveType(llvm::StringRef Name, SemaNameSpace *CurrentNameSpace) {
	SemaType *Type = nullptr;

	if (CurrentNameSpace->getName() == Module->getNameSpace()->getName()) {
		// Search for private types in the current module
		Type = Module->getTypes().lookup(Name);
	}

	// Search for public types in current namespace
	if (!Type)
		Type = CurrentNameSpace->getTypes().lookup(Name);

	if (!Type && !isDefaultNameSpace)
		// Resolve in Default CurrentNameSpace
		Type = S.getSymTable().getDefaultNameSpace()->getTypes().lookup(Name);

	return Type;
}

void Resolver::ResolveErrorHandler(ASTStmt *Stmt, SemaCall *Sema) {
	// Search until parent is null or parent is a Handle Stmt
	// When Parent Stmt is nullptr assign Function ErrorHandler to Call ErrorHandler
	ASTStmt *Parent = Stmt;
	while (Sema->getErrorHandler() == nullptr) {
		Parent = Parent->getParent();
		if (Parent == nullptr) {
			Sema->ErrorHandler = CurrentFunction->getErrorHandler();
		} else if (Parent->getStmtKind() == ASTStmtKind::STMT_HANDLE) {
			ASTHandleStmt *HandleStmt = static_cast<ASTHandleStmt*>(Parent);
			if (HandleStmt->getErrorHandlerRef() != nullptr) {
				Sema->ErrorHandler = reinterpret_cast<SemaErrorHandler *>(HandleStmt->getErrorHandlerRef()->Sema);
			}
		}
	}
}

/**
 * Resolve a Call Reference
 * @param Stmt
 * @param Call
 * @param NameSpaces
 * @param ...
 * @return
 */
SemaCall *Resolver::ResolveCall(ASTStmt *Stmt, ASTCall *Call, SemaNameSpace *CurrentNameSpace) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveCall", Logger().Attr("Call", Call).End());
    assert(Stmt && "Stmt cannot be null");
    assert(Call && "Call cannot be null");

    // Resolve Expression in Arguments
    llvm::SmallVector<SemaType *, 8> TypeArgs = ResolveCallArgTypes(Stmt, Call);

    // if Arguments are not resolved is not possible go ahead with call reference resolution
    // cannot resolve with the function parameters types
    std::string Mangled = SemaFunctionBase::MangleFunction(Call->getName(), TypeArgs);

	// Set as Resolved: TODO check if Resolve == false at start
	Call->setVisited(true);
	SemaCall *Sema = nullptr;

	// Create a new instance using a Constructor
    if (Call->getCallKind() == ASTCallKind::CALL_NEW ||
        Call->getCallKind() == ASTCallKind::CALL_NEW_UNIQUE ||
        Call->getCallKind() == ASTCallKind::CALL_NEW_SHARED ||
        Call->getCallKind() == ASTCallKind::CALL_NEW_WEAK) {

        // Take the Type from the CurrentNameSpace
        SemaType *Type = ResolveType(Call->getName(), CurrentNameSpace);

    	// No type found, no constructor
    	if (Type == nullptr) {
    		Diag(Call->getLocation(), diag::err_unref_type);
    		return nullptr;
    	}

    	// Call Constructor
        if (!Type->isClass()) {
        	Diag(Call->getLocation(), diag::err_unref_type);
        	return nullptr;
        }

		SemaClassType *Class = static_cast<SemaClassType *>(Type);
		SemaClassMethod *Constructor = Class->getConstructors().lookup(Mangled);

    	if (Constructor == nullptr) {
    		// Error: func not found
    		Diag(Call->getLocation(), diag::err_syntax_error);
    		return nullptr;
    	}

        Sema = SemaBuilder::CreateCall(*Call);
        Sema->Function = Constructor;
    	Sema->Type = Type;
    } else {

    	// Call a Function or a Class Method
    	SemaFunctionBase *Func = nullptr;
    	if (CurrentFunction->getKind() == SemaKind::METHOD) {

    		// Check if the Call is a Base Class Constructor Method
    		SemaClassType *CurrentClass = static_cast<SemaClassMethod *>(CurrentFunction)->getClass();
    		SemaType * T = CurrentNameSpace ?
			            CurrentNameSpace->getTypes().lookup(Call->Name) :
			            CurrentNameSpace->getTypes().lookup(Call->Name);
    		if (T->isClass()) {
				SemaClassType * C = static_cast<SemaClassType *>(T);

    			// Call to Base Class Constructor Method
    			if (C->isBaseOrEquals(CurrentClass)) {
    				// Resolve Call with Class Constructor if is not private
    				SemaClassMethod *Constructor = C->getConstructors().lookup(Mangled);

    				// check if Constructor is private
    				if (Constructor->getVisibility() == SemaVisibilityKind::PRIVATE) {
    					// Error: method is private, cannot be called from outside the class
    					Diag(Call->getLocation(), diag::err_syntax_error);
    					return nullptr;
    				}
    				// Take the Constructor
    				Func = Constructor;
    			} else {
    				// Static Call to Class Method
    				SemaClassMethod *Method = C->getMethods().lookup(Mangled);

    				// Check if Method is static
    				if (!Method->isStatic()) {
    					// Error: method is not static, cannot be called statically
    					Diag(Call->getLocation(), diag::err_syntax_error);
    				}

    				// Check if Method is private
    				if (Method->getVisibility() == SemaVisibilityKind::PRIVATE) {
    					// Error: method is private, cannot be called from outside the class
    					Diag(Call->getLocation(), diag::err_syntax_error);
    				}

    				// Take the Method
    				Func = Method;
    			}
    		}
    	}

    	// Take the Function
    	if (Func == nullptr) {
    		Func = CurrentNameSpace ?
				CurrentNameSpace->getFunctions().lookup(Mangled) :
				CurrentNameSpace->getFunctions().lookup(Mangled);
    	}

    	if (Func == nullptr) {
    		// Error: func not found
    		Diag(Call->getLocation(), diag::err_syntax_error);
    		return nullptr;
    	}

    	Sema = SemaBuilder::CreateCall(*Call);
        Sema->Function = Func;
    	Sema->Type = Func->getReturnType();
    }

	// Set the Call Sema ErrorHandler
	ResolveErrorHandler(Stmt, Sema);

	Sema->AST = Call;
	Call->setSema(Sema);
	return Sema;
}

llvm::SmallVector<SemaType *, 8> Resolver::ResolveCallArgTypes(ASTStmt *Stmt, ASTCall *Call) {
	// Resolve Expression in Arguments
	llvm::SmallVector<SemaType *, 8> CallTypes;
	for (auto Arg : Call->getArgs()) {
		Arg->getExpr()->accept(*this);
		CallTypes.push_back(Arg->getExpr()->getType());
	}
	return CallTypes;
}

SemaVar *Resolver::ResolveVar(ASTStmt *Stmt, ASTIdentifier *VarRef) {
	SemaVar *Sema = nullptr;

	// Skip already resolved VarRef
	if (VarRef->getSema() != nullptr) {
		Sema = static_cast<SemaVar *>(VarRef->getSema());
	}

	// Search into Local Vars
	if (!Sema && Stmt->getStmtKind() == ASTStmtKind::STMT_BLOCK) {
		ASTBlockStmt *Block = static_cast<ASTBlockStmt*>(Stmt);
		const auto &It = Block->getLocalVars().find(VarRef->getName());
		if (It != Block->getLocalVars().end()) { // Search into this Block
			Sema = It->getValue()->getSema();
		}
	}

	// Search in parent Stmt
	if (!Sema && Stmt->getParent()) {
		// search recursively into Parent Stmt to find the right Var definition
		SemaVar * ParentVar = ResolveVar(Stmt->getParent(), VarRef);
		if (ParentVar) {
			Sema = ParentVar;
		}
	}

	// Search into Function Parameter list
	if (!Sema) {
		llvm::SmallVector<ASTVar *, 8> Params = Stmt->getFunction()->getParams();
		for (auto &Param : Params) {
			if (Param->getName() == VarRef->getName()) {
				Sema = Param->getSema();
				break;
			}
		}
	}

	// Search into Class Attributes
	bool isSemaAttribute = false;
	if (!Sema && CurrentFunction->getKind() == SemaKind::METHOD) {
		// Get the Class Type
		SemaClassMethod *Method = static_cast<SemaClassMethod *>(CurrentFunction);
		if (!Method->isStatic()) {
			if (VarRef->getName() == "this") {
				Sema = Method->getClass()->getThis();
			} else {
				Sema = Method->getClass()->getAttributes().lookup(VarRef->getName());
			}
			if (Sema) {
				isSemaAttribute = true; // VarRef is a Class Attribute
			}
		}
	}

	if (Sema == nullptr) {
		// Error: var not found
		Diag(VarRef->getLocation(), diag::err_syntax_error);
	}

	// Add Var to LocalVars of the SemaFunctionBase
	if (!isSemaAttribute)
		CurrentFunction->getLocalVars().push_back(Sema); // Function Local var to be allocated

	VarRef->Sema = Sema;
	VarRef->Visited = true;
	return Sema;
}
