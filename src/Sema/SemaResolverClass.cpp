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
#include <AST/ASTScopes.h>
#include <AST/ASTVar.h>
#include <Sema/ASTBuilder.h>
#include <Sema/Sema.h>
#include <Sema/SemaBuilder.h>
#include <Sema/SemaBuilderScopes.h>
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

	// Resolve Super Classes
	Resolver->SuperClasses();

	// Resolve Class Definitions
	Resolver->Definitions();

	// Create Class Default Constructor
	Resolver->CreateDefaultConstructor();

	// Resolve Class Attributes
	Resolver->SetDefaultValueInAttributes();

	// Resolve Class Methods
	Resolver->AddBodies();
}

SemaResolverClass::SemaResolverClass(SemaResolver *R, SemaClassType *Class) : R(R), S(R->S), Class(Class) {

}

void SemaResolverClass::SuperClasses() {
	// Resolve Super Classes on first pass
	llvm::SmallVector<ASTTypeRef *, 4> SuperClassesTypeRef = Class->getAST()->getSuperClasses();
	while (SuperClassesTypeRef.empty() == false) {
		for (auto &ClassTypeRef : SuperClassesTypeRef) {

			// Search for the SuperClass in the Module, NameSpace or Imports
			if (R->ResolveTypeRef(ClassTypeRef)) {
				SemaType *SuperType = ClassTypeRef->getSema();

				if (SuperType->getKind() != SemaTypeKind::TYPE_CLASS) {
					// Error: invalid superclass type
					S.Diag(ClassTypeRef->getLocation(), diag::err_syntax_error);
					break;
				}

				SemaClassType *SuperClass = static_cast<SemaClassType *>(SuperType);

				// Struct: Resolve Var in Super Classes
				if (SuperClass->getClassKind() == SemaClassKind::STRUCT) {

					// Interface cannot extend a Struct
					if (Class->getClassKind() == SemaClassKind::INTERFACE) {
						S.Diag(ClassTypeRef->getLocation(), diag::err_sema_interface_ext_struct);
						return;
					}

					// Add Vars to the Struct
					for (auto &SuperAttributeEntry : SuperClass->getAttributes()) {
						SemaClassAttribute *SuperAttribute = SuperAttributeEntry.getValue();

						// Check Var already exists and type conflicts in Super Vars
						if (Class->Attributes.lookup(SuperAttributeEntry.getKey())) {
							S.Diag(SuperAttribute->getAST()->getLocation(), diag::err_sema_super_struct_var_conflict);
						} else {
							Class->Attributes.insert(std::make_pair(SuperAttributeEntry.getKey(), SuperAttribute));
						}
					}
				}

				// Interface cannot extend a Class
				if (Class->getClassKind() == SemaClassKind::INTERFACE &&
				    SuperClass->getClassKind() == SemaClassKind::CLASS) {
					S.Diag(SuperClass->getAST()->getLocation(), diag::err_sema_interface_ext_class);
					return;
				}

				// Class/Interface: take all Super Classes methods
				if (SuperClass->getClassKind() == SemaClassKind::CLASS ||
					SuperClass->getClassKind() == SemaClassKind::INTERFACE) {

					// FIXME
					// Collects Super Methods of the Super Classes
					//                                for (auto SuperMethod: SuperClass->getMethods()) {
					//                                    if (SuperClass->getClassKind() == ASTClassKind::INTERFACE) {
					//                                        S.Builder->InsertFunction(ISuperMethods, SuperMethod);
					//                                    } else {
					//                                        // Insert methods in the Super and if is ok also in the base Class
					//                                        if (S.Builder->InsertFunction(SuperMethods, SuperMethod)) {
					//                                            SmallVector<ASTScope *, 8> Scopes = SuperMethod->getScopes();
					//                                            ASTFunction *M = S.Builder->CreateClassMethod(SuperMethod->getLocation(),
					//                                                                                             *Class,
					//                                                                                             SuperMethod->getReturnType(),
					//                                                                                             SuperMethod->getName(),
					//                                                                                             Scopes);
					//                                            M->Params = SuperMethod->Params;
					//                                            M->Body = SuperMethod->Body;
					//                                            M->DerivedClass = Class;
					//                                            Class->Methods.push_back(M);
					//
					//                                        } else {
					//                                            // Multiple Methods Implementations in Super Class need to be re-defined in base class
					//                                            // Search if this method is re-defined in the base class
					//                                            if (SuperMethod->getVisibility() !=
					//                                                ASTVisibilityKind::V_PRIVATE &&
					//                                                !S.Builder->ContainsFunction(Class->Methods, SuperMethod)) {
					//                                                S.Diag(SuperMethod->getLocation(),
					//                                                       diag::err_sema_super_class_method_conflict);
					//                                                return;
					//                                            }
					//                                        }
					//                                    }
					//                                }
				}
			}
		}
	}
}

void SemaResolverClass::Definitions() {
	SemaComment *Comment = nullptr;

	// Resolve Class Definitions: Var, Function and Comment
	for (auto AST: Class->getAST()->getDefinitions()) {
		switch (AST->getKind()) {
			case ASTKind::AST_VAR: {
				ASTVar * Var = static_cast<ASTVar *>(AST);
				if (R->ResolveTypeRef(Var->TypeRef)) {
					S.getSemaBuilder().CreateClassAttribute(Class, Var, Comment);
					Var->Sema->Type = Var->TypeRef->getSema();
				}
				Comment = nullptr;
			}
			break;
			case ASTKind::AST_FUNCTION: {
				S.getSemaBuilder().CreateClassMethod(Class, static_cast<ASTFunction *>(AST), Comment);
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
	// Create the default constructor if no constructors are defined
	if (Class->getConstructors().empty()) {
		SmallVector<ASTScope *, 8> Scopes = SemaBuilderScopes::Build()
					 ->addVisibility(SourceLocation(), ASTVisibilityKind::V_DEFAULT)->getScopes();
		llvm::SmallVector<ASTVar *, 8> Params;
		ASTBlockStmt *Body = S.getASTBuilder().CreateBlockStmt(SourceLocation());
		ASTFunction * AST = S.getASTBuilder().CreateFunction(R->Module->getAST(), Class->getAST()->getLocation(),
												   nullptr, Class->getAST()->getName(), Scopes, Params, Body);
		SemaClassMethod * DefaultConstructor = S.getSemaBuilder().CreateClassMethod(Class, AST, nullptr);
	}
}

void SemaResolverClass::SetDefaultValueInAttributes() {
	// Set default values in attributes
	if (Class->getClassKind() == SemaClassKind::INTERFACE || Class->getClassKind() == SemaClassKind::STRUCT) {

		// Init null value attributes with default values
		for (auto &AttributeEntry : Class->getAttributes()) {
			SemaClassAttribute *Attribute = AttributeEntry.getValue();
			// Generate default values
			if (Attribute->getAST()->getExpr() == nullptr) {
				ASTValue *DefaultValue = S.getASTBuilder().CreateDefaultValue(Attribute->getType());
				ASTValueExpr *ValueExpr = S.getASTBuilder().CreateExpr(DefaultValue);
				Attribute->AST->Expr = ValueExpr;
			}
			S.getValidator().CheckIsValueExpr(Attribute->getAST()->getExpr());
		}
	}
}

void SemaResolverClass::AddBodies() {
	// Constructors
	for (auto &ConstructorEntry: Class->getConstructors()) {
		ASTBlockStmt * Body = ConstructorEntry.getValue()->getAST()->getBody();

		// Add Attribute to Constructor LocalVars
		for (auto &Attribute: Class->Attributes) {
			ConstructorEntry.second->LocalVars.push_back(Attribute.getValue());
		}

		R->Bodies.push_back(ConstructorEntry.getValue()->getAST()->getBody());
	}

	// Methods
	for (auto &MethodEntry : Class->getMethods()) {
		SemaClassMethod * Method = MethodEntry.getValue();

		// Add Attribute to Method LocalVars
		for (auto &Attribute: Class->Attributes) {
			MethodEntry.second->LocalVars.push_back(Attribute.getValue());
		}

		if (Method->getAST()->getBody()) {
			R->Bodies.push_back(Method->getAST()->getBody());
		}
	}
}
