//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaResolverClass.cpp - The Sema Resolver
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaResolverClass.h"

#include <AST/ASTComment.h>
#include <AST/ASTFunction.h>
#include <AST/ASTModifier.h>
#include <AST/ASTVar.h>
#include <Sema/ASTBuilder.h>
#include <Sema/Sema.h>
#include <Sema/SemaBuilder.h>
#include <Sema/SemaBuilderModifiers.h>
#include <Sema/SemaClassAttribute.h>
#include <Sema/SemaClassMethod.h>
#include <Sema/SemaClassType.h>
#include <Sema/SemaModule.h>
#include <Sema/SemaResolver.h>
#include "Basic/Diagnostic.h"

#include <AST/ASTBlockStmt.h>
#include <AST/ASTExpr.h>
#include <Sema/SemaValidator.h>


using namespace fly;

void SemaResolverClass::BaseClasses(SemaResolver *R, SemaClassType *Class) {
	// ClassDefinition Base Classes on first pass
	for (auto &BaseTypeRef : Class->getAST()->getBaseClasses()) {

		// Search for the SuperClass in the Module, NameSpace or Imports
		if (R->ResolveTypeRef(BaseTypeRef)) {
			SemaType *BaseType = BaseTypeRef->getSema();

			if (BaseType->getKind() != SemaTypeKind::TYPE_CLASS) {
				// Error: invalid superclass type
				R->S.Diag(BaseTypeRef->getLocation(), diag::err_syntax_error);
				break;
			}

			SemaClassType *BaseClassType = static_cast<SemaClassType *>(BaseType);
			Class->BaseClasses.push_back(BaseClassType);
		}
	}
}

void SemaResolverClass::ClassDefinition(SemaResolver *R, SemaClassType *Class) {
	SemaResolverClass * Resolver = new SemaResolverClass(R, Class);

	// Create Class This Attribute
	Class->This = R->S.getSemaBuilder().CreateThisInstance(Class);

	// Resolve Class Definitions
	Resolver->CreateDefinitions();

	// Resolve Base Classes definitions by inheriting all
	Resolver->CreateBaseDefinitions(Class);

	// Create Class Default Constructor
	// Create the default constructor if no constructors are defined
	if (Class->getClassKind() != SemaClassKind::INTERFACE && Class->getConstructors().empty())
		Resolver->CreateDefaultConstructor();

	// ClassDefinition Class Attributes
	Resolver->SetDefaultValueInAttributes();

	// ClassDefinition Class Methods
	if (Class->getClassKind() == SemaClassKind::CLASS)
		Resolver->AddBodies();
}

SemaResolverClass::SemaResolverClass(SemaResolver *R, SemaClassType *Class) : R(R), S(R->S), Class(Class) {

}

void SemaResolverClass::CreateDefinitions() {
	SemaComment *Comment = nullptr;

	// ClassDefinition Class Definitions: Var, Function and Comment
	for (auto AST : Class->getAST()->getDefinitions()) {
		switch (AST->getKind()) {

			// ClassDefinition Class Var: Attribute
			case ASTKind::AST_VAR: {
				ASTVar * Var = static_cast<ASTVar *>(AST);
				SemaClassAttribute *Attribute = DefineAttribute(Class->getThis(), Var, Comment);

				Var->Sema->Type = Var->TypeRef->getSema();
				Class->Attributes.insert(std::make_pair(Var->getName(), Attribute));

				Comment = nullptr;
			}
			break;

			// ClassDefinition Class Function: Method or Constructor
			case ASTKind::AST_FUNCTION: {
				ASTFunction *Function = static_cast<ASTFunction *>(AST);

				// Define a Method
				SemaClassMethod * Method = DefineMethod(Class->getThis(), Function, Comment);
				if (Method->isConstructor()) {
					Class->Constructors.insert(std::make_pair(Method->getMangledName(), Method));
				} else {
					Class->Methods.insert(std::make_pair(Method->getMangledName(), Method));
				}

				// Reset Comment
				Comment = nullptr;
			}
			break;

			// ClassDefinition Class Comment
			case ASTKind::AST_COMMENT:
				Comment = S.getSemaBuilder().CreateComment(static_cast<ASTComment *>(AST));
			break;

			default:
				// Error: invalid declaration in class
					S.Diag(AST->getLocation(), diag::err_syntax_error);
			break;
		}
	}
}

SemaClassAttribute *SemaResolverClass::DefineAttribute(SemaClassInstance *This, ASTVar *Var, SemaComment *Comment) {
	// ClassDefinition Type
	R->ResolveTypeRef(Var->TypeRef);

	// Create Attribute
	SemaClassAttribute *Attribute = S.getSemaBuilder().CreateClassAttribute(Class, Class->getThis(), Var, Comment);

	return Attribute;
}

SemaClassMethod *SemaResolverClass::DefineMethod(SemaClassInstance *This, ASTFunction *Function, SemaComment *Comment) {
	// ClassDefinition Return Type
	R->ResolveTypeRef(Function->ReturnTypeRef);

	// Create Method
	SemaClassMethod *Method = S.getSemaBuilder().CreateClassMethod(Class, This, Function, Comment);

	// ClassDefinition Parameters Types
	for (auto Param : Function->getParams()) {
		// Check duplicated params
		// TODO
		//S.getValidator().CheckDuplicateParams(Function->Params, Param);

		// resolve parameter type
		if (R->ResolveTypeRef(Param->TypeRef)) {
			SemaParam *P = S.getSemaBuilder().CreateParam(Param);
			P->Type = Param->TypeRef->getSema();
			Method->Params.push_back(P);
		}
	}

	// If not null: Add to Body List for resolve in the next step
	if (Function->Body)
		R->Bodies.push_back(Function->Body);

	return Method;
}

void SemaResolverClass::CreateBaseDefinitions(SemaClassType *InheritClass) {
	for (SemaClassType *BaseClass : InheritClass->getBaseClasses()) {

		// Create the Class Instance to be added to Base Instance Children
		SemaClassInstance *BaseThis = R->S.getSemaBuilder().CreateThisInstance(BaseClass);
		auto Pair = std::pair<size_t, SemaClassInstance *>(BaseClass->getId(), BaseThis);

		// Add the Base Class Instance to the Class This Instances
		Class->This->BaseInstances.insert(Pair);

		for (auto AST : BaseClass->getAST()->getDefinitions()) {
			switch (AST->getKind()) {

				// ClassDefinition Class Var: Attribute
				case ASTKind::AST_VAR: {
					ASTVar * Var = static_cast<ASTVar *>(AST);

					// Define an Attribute
					SemaClassAttribute *Attribute = DefineAttribute(BaseThis, Var, nullptr);

					if (CanInheritAttribute(Attribute)) {
						Var->Sema->Type = Var->TypeRef->getSema();
						Class->Attributes.insert(std::make_pair(Var->getName(), Attribute));
					}
				}
				break;

				// ClassDefinition Class Function: Method or Constructor
				case ASTKind::AST_FUNCTION: {
					ASTFunction *Function = static_cast<ASTFunction *>(AST);

					// Define a Method
					SemaClassMethod *Method = DefineMethod(BaseThis, Function, nullptr);

					if (!Method->isConstructor() && CanInheritMethod(Method)) {
						Class->Methods.insert(std::make_pair(Method->getMangledName(), Method));
					}
				}
				break;

				// ClassDefinition Class Comment
				case ASTKind::AST_COMMENT:
					// No action needed
				break;

				default:
					// Error: invalid declaration in class
						S.Diag(AST->getLocation(), diag::err_syntax_error);
				break;
			}
		}

		CreateBaseDefinitions(BaseClass);
	}
}

bool SemaResolverClass::CanInheritMethod(SemaClassMethod *BaseMethod) {
	// Add Methods from Super SuperClassType
	// Check if Method Visibility is not private and not static
	if (BaseMethod->getVisibility() > SemaVisibilityKind::PRIVATE && !BaseMethod->isStatic()) {

		// Check Methods already exists and type conflicts in Super Methods
		auto It = Class->getMethods().find(BaseMethod->getMangledName());
		if (It == Class->getMethods().end()) { // Not Found, add new Method
			return true;
		} else { // Duplicate Found, check conflicts

			// Check Return Type conflicts
			if (It->second->getReturnType() != BaseMethod->getReturnType()) {
				S.Diag(It->second->getAST()->getLocation(), diag::err_syntax_error);
			}

			// Check Visibility conflicts
			if (It->second->getVisibility() < BaseMethod->getVisibility()) {
				S.Diag(It->second->getAST()->getLocation(), diag::err_syntax_error);
			}

			// Check Static conflicts
			if (It->second->isStatic()) {
				S.Diag(It->second->getAST()->getLocation(), diag::err_syntax_error);
			}

			// If the inherited method appear in more than one super class: need to be re-defined
			// Check if class methods contains the same inherited method without redefine it
			if (Class->getName() != It->getValue()->getClass()->getName()) {

                // Error: method already exists in super class
                S.Diag(BaseMethod->getAST()->getLocation(), diag::err_syntax_error);
            }

			return false;
		}
	}
}

bool SemaResolverClass::CanInheritAttribute(SemaClassAttribute *BaseAttribute) {
	// Check if Attribute Visibility is not private and not static
	if (BaseAttribute->getVisibility() > SemaVisibilityKind::PRIVATE && !BaseAttribute->isStatic()) {

		// Check Attribute already exists and type conflicts in Super Vars
		auto It = Class->Attributes.find(BaseAttribute->getAST()->getName());
		if (It == Class->Attributes.end()) { // Not Found
			return true;
		} else { // Duplicate Found

			// Check Type conflicts
			if (It->second->getType() != BaseAttribute->getType()) {
				S.Diag(It->second->getAST()->getLocation(), diag::err_syntax_error);
			}

			// Check Visibility conflicts
			if (It->second->getVisibility() < BaseAttribute->getVisibility()) {
				S.Diag(It->second->getAST()->getLocation(), diag::err_syntax_error);
			}

			// Check Constant conflicts
			if (!It->second->isConstant() && BaseAttribute->isConstant()) {
				S.Diag(It->second->getAST()->getLocation(), diag::err_syntax_error);
			}

			// Check Static conflicts
			if (It->second->isStatic()) {
				S.Diag(It->second->getAST()->getLocation(), diag::err_syntax_error);
			}

			return false;
		}
	}
}

void SemaResolverClass::CreateDefaultConstructor() {

	// Create Default Modifier
	llvm::SmallVector<ASTModifier *, 8> Modifiers;
	Modifiers.push_back(S.getASTBuilder().CreateModifier(SourceLocation(), ASTModifierKind::MOD_DEFAULT));

	llvm::SmallVector<ASTVar *, 8> Params;
	ASTBlockStmt *Body = S.getASTBuilder().CreateBlockStmt(SourceLocation());
	ASTFunction * AST = S.getASTBuilder().CreateFunction(R->Module->getAST(), Class->getAST()->getLocation(),
											   nullptr, Class->getAST()->getName(), Modifiers, Params, Body);

	// Call default constructor of the super classes (if exists)
	if (!Class->getBaseClasses().empty()) {
		for (auto &BaseClass : Class->getBaseClasses()) {
			// Create Call to Base Constructor
			if (BaseClass->DefaultConstructor) {
				SemaClassMethod * DefaultConstructor = BaseClass->DefaultConstructor;
				llvm::SmallVector<ASTExpr *, 8> Args;
				SourceLocation Loc = SourceLocation();

				// TODO: Create Call to Base Constructor
				// ASTCall *BaseCall = S.getASTBuilder().CreateCall(DefaultConstructor->getAST()->getName(), Args);
				// SemaBuilderStmt * ExprStmt = S.getASTBuilder().CreateExprStmt(Body, Loc);
				// ExprStmt->setExpr(S.getASTBuilder().CreateExpr(BaseCall));
			} else {
                // Error: Base Class has no default constructor
                S.Diag(BaseClass->getAST()->getLocation(), diag::err_syntax_error);
            }
		}
	}

	// Create the default constructor
	SemaClassMethod *Constructor = S.getSemaBuilder().CreateClassMethod(Class, Class->This, AST, nullptr);

	Class->Constructors.insert(std::make_pair(Constructor->getMangledName(), Constructor));
}

void SemaResolverClass::SetDefaultValueInAttributes() {
	// Set default values in attributes
	for (auto &AttributeEntry : Class->getAttributes()) {
		SemaClassAttribute *Attribute = AttributeEntry.getValue();

		// Generate default values
		if (Attribute->getAST()->getExpr() == nullptr) {
			R->ResolveTypeRef(Attribute->getAST()->TypeRef);

			ASTValue *DefaultValue = S.getASTBuilder().CreateDefaultValue(Attribute->getType());
			Attribute->AST->Expr = S.getASTBuilder().CreateExpr(DefaultValue);
			R->ResolveValue(DefaultValue);
			Attribute->AST->Expr->Type = Attribute->getType();
		} else {

			// Check Value is default value expression
			SemaValidator::CheckIsValueExpr(Attribute->getAST()->getExpr());
		}
	}
}

void SemaResolverClass::AddBodies() {
	// Constructors
	for (auto &ConstructorEntry: Class->getConstructors()) {
		ASTBlockStmt * Body = ConstructorEntry.getValue()->getAST()->getBody();

		// Add Attribute to Constructor LocalVars // FIXME remove this
		// for (auto &Attribute: Class->Attributes) {
		// 	ConstructorEntry.second->LocalVars.push_back(Attribute.getValue());
		// }

		R->Bodies.push_back(ConstructorEntry.getValue()->getAST()->getBody());
	}

	// Methods
	for (auto &MethodEntry : Class->getMethods()) {
		SemaClassMethod * Method = MethodEntry.getValue();

		if (Method->getAST()->getBody()) {
			R->Bodies.push_back(Method->getAST()->getBody());
		}
	}
}
