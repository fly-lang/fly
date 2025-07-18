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

void SemaResolverClass::Resolve(SemaResolver *R, SemaClassType *Class) {
	SemaResolverClass * Resolver = new SemaResolverClass(R, Class);

	// Create Class This Attribute
	if (Class->getClassKind() != SemaClassKind::INTERFACE)
		Resolver->S.getSemaBuilder().CreateThisAttribute(Class);

	// Resolve Super Classes
	Resolver->Extends();

	// Resolve Class Definitions
	Resolver->Definitions();

	// Create Class Default Constructor
	// Create the default constructor if no constructors are defined
	if (Class->getClassKind() != SemaClassKind::INTERFACE && Class->getConstructors().empty())
		Resolver->CreateDefaultConstructor();

	// Resolve Class Attributes
	Resolver->SetDefaultValueInAttributes();

	// Resolve Class Methods
	if (Class->getClassKind() == SemaClassKind::CLASS)
		Resolver->AddBodies();
}

SemaResolverClass::SemaResolverClass(SemaResolver *R, SemaClassType *Class) : R(R), S(R->S), Class(Class) {

}

void SemaResolverClass::Extends() {
	// Resolve Super Classes on first pass
	llvm::SmallVector<ASTTypeRef *, 4> SuperClassesTypeRef = Class->getAST()->getSuperClasses();
	for (auto &ClassTypeRef : SuperClassesTypeRef) {

		// Search for the SuperClass in the Module, NameSpace or Imports
		if (R->ResolveTypeRef(ClassTypeRef)) {
			SemaType *SuperType = ClassTypeRef->getSema();

			if (SuperType->getKind() != SemaTypeKind::TYPE_CLASS) {
				// Error: invalid superclass type
				S.Diag(ClassTypeRef->getLocation(), diag::err_syntax_error);
				break;
			}

			SemaClassType *SuperClassType = static_cast<SemaClassType *>(SuperType);
			switch (Class->getClassKind()) {

				case SemaClassKind::CLASS:
					InheritAttributes(SuperClassType);
					InheritMethods(SuperClassType);
					break;

				case SemaClassKind::INTERFACE: {
					if (SuperClassType->getClassKind() == SemaClassKind::INTERFACE) {
						InheritMethods(SuperClassType);
					} else {
						S.Diag(ClassTypeRef->getLocation(), diag::err_sema_interface_notext_interface);
					}
				} break;

				case SemaClassKind::STRUCT: {
					if (SuperClassType->getClassKind() == SemaClassKind::STRUCT) {
						InheritAttributes(SuperClassType);
					} else {
						S.Diag(ClassTypeRef->getLocation(), diag::err_sema_struct_notext_struct);
					}
				} break;
			}
		}
	}
}

void SemaResolverClass::InheritMethods(SemaClassType *ClassType) {
	// Add Methods from Super Class to Class
	for (auto &MethodEntry : ClassType->getMethods()) {
		SemaClassMethod *Method = MethodEntry.getValue();

		// Check if Method Visibility is not private and not static
		if (Method->getVisibility() > SemaVisibilityKind::PRIVATE && !Method->isStatic()) {

			// Check Attribute already exists and type conflicts in Super Vars
			auto It = Class->getMethods().find(MethodEntry.getKey());
			if (It == Class->getMethods().end()) { // Not Found
				Class->Methods.insert(std::make_pair(MethodEntry.getKey(), Method));
			} else { // Duplicate Found
				// Check Return Type conflicts
				if (It->second->getReturnType() != Method->getReturnType()) {
					S.Diag(It->second->getAST()->getLocation(), diag::err_syntax_error);
				}

				// Check Visibility conflicts
				if (It->second->getVisibility() < Method->getVisibility()) {
					S.Diag(It->second->getAST()->getLocation(), diag::err_syntax_error);
				}

				// Check Static conflicts
				if (It->second->isStatic()) {
					S.Diag(It->second->getAST()->getLocation(), diag::err_syntax_error);
				}
			}
		}
	}
}

void SemaResolverClass::InheritAttributes(SemaClassType * ClassType) {
	// Add Attributes from Super Class to Class
	for (auto &AttributeEntry : ClassType->getAttributes()) {
		SemaClassAttribute *Attribute = AttributeEntry.getValue();

		// Check if Attribute Visibility is not private and not static
		if (Attribute->getVisibility() > SemaVisibilityKind::PRIVATE && !Attribute->isStatic()) {

			// Check Attribute already exists and type conflicts in Super Vars
			auto It = Class->Attributes.find(AttributeEntry.getKey());
			if (It == Class->Attributes.end()) { // Not Found
				SemaClassAttribute *NewAttribute = S.getSemaBuilder().CreateClassAttribute(Class, Attribute->getAST(), Attribute->getComment());
				NewAttribute->Type = Attribute->getType();
				Class->Attributes.insert(std::make_pair(AttributeEntry.getKey(), NewAttribute));
			} else { // Duplicate Found

				// Check Type conflicts
				if (It->second->getType() != Attribute->getType()) {
					S.Diag(It->second->getAST()->getLocation(), diag::err_syntax_error);
				}

				// Check Visibility conflicts
				if (It->second->getVisibility() < Attribute->getVisibility()) {
					S.Diag(It->second->getAST()->getLocation(), diag::err_syntax_error);
				}

				// Check Constant conflicts
				if (!It->second->isConstant() && Attribute->isConstant()) {
					S.Diag(It->second->getAST()->getLocation(), diag::err_syntax_error);
				}

				// Check Static conflicts
				if (It->second->isStatic()) {
					S.Diag(It->second->getAST()->getLocation(), diag::err_syntax_error);
				}
			}
		}
	}
}

void SemaResolverClass::Definitions() {
	SemaComment *Comment = nullptr;

	// Resolve Class Definitions: Var, Function and Comment
	for (auto AST : Class->getAST()->getDefinitions()) {
		switch (AST->getKind()) {
			case ASTKind::AST_VAR: {
				ASTVar * Var = static_cast<ASTVar *>(AST);

				// Resolve Type
				R->ResolveTypeRef(Var->TypeRef);

				// Create Attribute
				S.getSemaBuilder().CreateClassAttribute(Class, Var, Comment);
				Var->Sema->Type = Var->TypeRef->getSema();
				Comment = nullptr;
			}
			break;
			case ASTKind::AST_FUNCTION: {
				ASTFunction *Function = static_cast<ASTFunction *>(AST);

				// Resolve Return Type
				R->ResolveTypeRef(Function->ReturnTypeRef);

				// Create Method
				S.getSemaBuilder().CreateClassMethod(Class, static_cast<ASTFunction *>(AST), Comment);

				// Resolve Return Type
				Function->Sema->ReturnType = Function->ReturnTypeRef->getSema();

				// Resolve Parameters Types
				for (auto Param : Function->getParams()) {
					// Check duplicated params
					// TODO
					//S.getValidator().CheckDuplicateParams(Function->Params, Param);

					// resolve parameter type
					if (R->ResolveTypeRef(Param->TypeRef)) {
						SemaParam *P = S.getSemaBuilder().CreateParam(Param);
						P->Type = Param->TypeRef->getSema();
						Function->Sema->Params.push_back(P);
					}
				}

				// Add to Body List for resolve in the next step
				R->Bodies.push_back(Function->Body);
				Comment = nullptr;
			}
			break;
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

void SemaResolverClass::CreateDefaultConstructor() {

	// Create Default Modifier
	llvm::SmallVector<ASTModifier *, 8> Modifiers;
	Modifiers.push_back(S.getASTBuilder().CreateModifier(SourceLocation(), ASTModifierKind::MOD_DEFAULT));

	llvm::SmallVector<ASTVar *, 8> Params;
	ASTBlockStmt *Body = S.getASTBuilder().CreateBlockStmt(SourceLocation());
	ASTFunction * AST = S.getASTBuilder().CreateFunction(R->Module->getAST(), Class->getAST()->getLocation(),
											   nullptr, Class->getAST()->getName(), Modifiers, Params, Body);

	// Call default constructor of the super classes (if exists)
	if (!Class->getSuperClasses().empty()) {
		for (auto &SuperClassEntry : Class->getSuperClasses()) {
			// Resolve Super Class Type
			if (SuperClassEntry.getValue()->DefaultConstructor) {
				// Create Call to Super Constructor
				SemaClassMethod * DefaultConstructor = SuperClassEntry.getValue()->DefaultConstructor;
				const SourceLocation Loc = SourceLocation();
				llvm::SmallVector<ASTExpr *, 8> Args;
				ASTCall *SuperCall = S.getASTBuilder().CreateCall(DefaultConstructor->getAST()->getName(), Args);
				S.getASTBuilder().CreateExprStmt(Body, SourceLocation());
				S.getASTBuilder().CreateExpr(SuperCall);
			} else {
                // Error: Super Class has no default constructor
                S.Diag(SuperClassEntry.getValue()->getAST()->getLocation(), diag::err_syntax_error);
            }

		}

	}

	// Create the default constructor
	S.getSemaBuilder().CreateClassMethod(Class, AST, nullptr);
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
