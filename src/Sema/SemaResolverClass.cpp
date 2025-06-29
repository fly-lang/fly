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
	Resolver->S.getSemaBuilder().CreateThisAttribute(Class);

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
					//                                            SmallVector<ASTModifier *, 8> Modifiers = SuperMethod->getModifiers();
					//                                            ASTFunction *M = S.Builder->CreateClassMethod(SuperMethod->getLocation(),
					//                                                                                             *Class,
					//                                                                                             SuperMethod->getReturnType(),
					//                                                                                             SuperMethod->getName(),
					//                                                                                             Modifiers);
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

				// Add to Body list for resolve in the next step
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
	// Create the default constructor if no constructors are defined
	if (Class->getConstructors().empty()) {
		SmallVector<ASTModifier *, 8> Modifiers = SemaBuilderModifiers::Build()
					 ->addVisibility(SourceLocation(), ASTVisibilityKind::V_DEFAULT)->getModifiers();
		llvm::SmallVector<ASTVar *, 8> Params;
		ASTBlockStmt *Body = S.getASTBuilder().CreateBlockStmt(SourceLocation());
		ASTFunction * AST = S.getASTBuilder().CreateFunction(R->Module->getAST(), Class->getAST()->getLocation(),
												   nullptr, Class->getAST()->getName(), Modifiers, Params, Body);
		SemaClassMethod * DefaultConstructor = S.getSemaBuilder().CreateClassMethod(Class, AST, nullptr);

		// Call default constructor of the super classes (if exists)
		// TODO
	}
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
		}
		S.getValidator().CheckIsValueExpr(Attribute->getAST()->getExpr());
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
