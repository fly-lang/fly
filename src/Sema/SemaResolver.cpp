//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaResolver.cpp - The Sema Resolver
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaResolver.h"
#include "Sema/Sema.h"
#include "Sema/ASTBuilder.h"
#include "Sema/SymBuilder.h"
#include "Sema/SemaValidator.h"
#include "Sym/SymTable.h"
#include "Sym/SymNameSpace.h"
#include "Sym/SymModule.h"
#include "Sym/SymClass.h"
#include "Sym/SymEnum.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
#include "AST/ASTTypeRef.h"
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
#include "AST/ASTVar.h"
#include "AST/ASTAssignmentStmt.h"
#include "AST/ASTFailStmt.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTOpExpr.h"
#include "CodeGen/CodeGen.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"

#include "llvm/ADT/StringMap.h"

#include <AST/ASTAlias.h>
#include <AST/ASTComment.h>
#include <llvm/Transforms/IPO/FunctionImport.h>
#include <Sema/SymBuilder.h>
#include <Sym/SymClassAttribute.h>
#include <Sym/SymClassMethod.h>
#include <Sym/SymFunction.h>
#include <Sym/SymGlobalVar.h>

using namespace fly;

SemaResolver::SemaResolver(Sema &S, ASTModule *Module) :
        S(S), Module(S.getSymBuilder().CreateModule(Module)), NameSpace(S.getSymBuilder().AddNameSpace(Module->getNameSpace()->getName())) {

}

/**
 * Resolve Modules by creating the right structure for resolving all symbols
 */
bool SemaResolver::Resolve(Sema &S) {
    llvm::SmallVector<SemaResolver *, 8> Resolvers;

    // First: Resolve Modules for populate NameSpaces
    for (auto Module : S.getModules()) {

    	// Validate Module name duplication
    	if (S.getValidator().CheckDuplicateModules(Module)) {

    		// Resolve Declarations
    		SemaResolver *Resolver = new SemaResolver(S, Module);
    		Resolver->AddSymbols();

    		// add to Resolvers list
    		Resolvers.push_back(Resolver);
    	}
    }

    // Second: Resolve Definitions
    for (auto &Resolver : Resolvers) {
    	Resolver->ResolveImports();
		Resolver->ResolveGlobalVars();
    	Resolver->ResolveFunctions();
    	Resolver->ResolveClasses();
    	Resolver->ResolveEnums();
    }

    return !S.Diags.hasErrorOccurred();
}

void SemaResolver::AddSymbols() {

	SymComment *Comment = nullptr;
	for (ASTBase *AST: Module->getAST()->getDefinitions()) {

		// Set Comment if the previous AST is a Comment
		switch (AST->getKind()) {
			case ASTKind::AST_IMPORT: {
				AddImport(static_cast<ASTImport *>(AST));
				Comment = nullptr;
			} break;
			case ASTKind::AST_VAR: {
				AddGlobalVar(static_cast<ASTVar *>(AST), Comment);
				Comment = nullptr;
			} break;
			case ASTKind::AST_FUNCTION: {
				AddFunction(static_cast<ASTFunction *>(AST), Comment);
				Comment = nullptr;
			} break;
			case ASTKind::AST_CLASS: {
				AddClass(static_cast<ASTClass *>(AST), Comment);
				Comment = nullptr;
			} break;
			case ASTKind::AST_ENUM: {
				AddEnum(static_cast<ASTEnum *>(AST), Comment);
				Comment = nullptr;
			} break;
			case ASTKind::AST_COMMENT:
				Comment = S.getSymBuilder().CreateComment(static_cast<ASTComment *>(AST));
				break;
			default:
                // Error: invalid declaration in module
                S.Diag(AST->getLocation(), diag::err_syntax_error);
                break;
		}

	}
}

void SemaResolver::AddImport(ASTImport *AST) {
	// Check Import
	S.getSymBuilder().CreateImport(Module, AST);
}

/**
 * ResolveModule GlobalVar Declarations
 */
void SemaResolver::AddGlobalVar(ASTVar *AST, SymComment *Comment) {
    // Create GlobalVar
    SymGlobalVar *GlobalVar = S.getSymBuilder().CreateGlobalVar(Module, AST);
    GlobalVar->Comment = Comment;
}

/**
 * ResolveModule Function Declarations
 */
void SemaResolver::AddFunction(ASTFunction *AST, SymComment *Comment) {
    SymFunction *Function = S.getSymBuilder().CreateFunction(Module, AST);
    Function->Comment = Comment;
}

/**
 * ResolveModule Identity Declarations
 */
void SemaResolver::AddClass(ASTClass *AST, SymComment *Comment) {
	SymClass * Class = S.getSymBuilder().CreateClass(Module, AST);
	Class->Comment = Comment;
}

void SemaResolver::AddEnum(ASTEnum *AST, SymComment *Comment) {
	SymEnum *Enum = S.getSymBuilder().CreateEnum(Module, AST);
	Enum->Comment = Comment;
}

/**
 * ResolveModule Import Definitions
 */
void SemaResolver::ResolveImports() {
	for (auto Entry : Module->getImports()) {

		SymNameSpace *Import = S.getSymTable().getNameSpaces().lookup(Entry.getKey());
		if (!Import) {
			S.Diag(diag::err_namespace_notfound) << Entry.getKey();
			return;
		}

		Entry.setValue(Import);
	}
}

void SemaResolver::ResolveComment(SymComment *Comment, ASTBase* AST) {
	if (AST->getKind() == ASTKind::AST_FUNCTION) {
		ASTFunction * Function = (ASTFunction *) AST;
		S.getValidator().CheckCommentParams(Comment, Function->getParams());
		S.getValidator().CheckCommentReturn(Comment, Function->getReturnTypeRef());
		S.getValidator().CheckCommentFail(Comment);
	}
}

/**
 * Resolve Module GlobalVar Definitions
 */
void SemaResolver::ResolveGlobalVars() {
	for (auto Entry : Module->getGlobalVars()) {
		SymGlobalVar *Sym = Entry.getValue();
		ASTVar *AST = Sym->getAST();

		if (Sym->Comment) {
			ResolveComment(Sym->Comment, AST);
		}

		// Check Expr Value
		if (AST->Expr && AST->Expr->getExprKind() != ASTExprKind::EXPR_VALUE) {
			S.Diag(AST->Expr->getLocation(), diag::err_invalid_gvar_value);
		}

		// Resolve Type
		ResolveTypeRef(AST->TypeRef);
	}
}

/**
 * Resolve Module Function Definitions
 */
void SemaResolver::ResolveFunctions() {
	for (auto Entry : Module->getFunctions()) {
		SymFunction *Sym = Entry.getValue();
		ASTFunction *AST = Sym->getAST();

		if (Sym->Comment) {
			ResolveComment(Sym->Comment, AST);
		}

		// Resolve Return Type
		ResolveTypeRef(AST->ReturnTypeRef);

		// Resolve Parameters Types
		for (auto Param : AST->getParams()) {
			// Check duplicated params
			// TODO
			//S.getValidator().CheckDuplicateParams(Function->Params, Param);

			// resolve parameter type
			ResolveTypeRef(Param->TypeRef);
		}

		// Resolve Function Body
		ResolveStmtBlock(AST->Body);

	}
}

/**
 * Resolve Module Identity Definitions
 */
void SemaResolver::ResolveClasses() {
	// FIXME resolve inherity by resolving base class on first
	for (auto ClassEntry : Module->getClasses()) {
		SymClass *Sym = ClassEntry.getValue();

		// Create default Constructor if it needs
		// TODO
		// llvm::SmallVector<ASTVar *, 8> Params;
		// ASTBlockStmt *Body = CreateBlockStmt(SourceLocation());
		// Class->DefaultConstructor = S.Builder->CreateClassConstructor(SourceLocation(), *Class, Scopes, Params, Body);

		if (Sym->Comment) {
			ResolveComment(Sym->Comment, Sym->getAST());
		}

		SymComment *Comment = nullptr;
		for (auto AST: Sym->getAST()->getDefinitions()) {
			switch (AST->getKind()) {
				case ASTKind::AST_VAR: {
					S.getSymBuilder().CreateClassAttribute(Sym, static_cast<ASTVar *>(AST), Comment);
					Comment = nullptr;
				}	break;
				case ASTKind::AST_FUNCTION: {
					S.getSymBuilder().CreateClassFunction(Sym, static_cast<ASTFunction *>(AST), Comment);
					Comment = nullptr;
				}	break;
				case ASTKind::AST_COMMENT:
					Comment = S.getSymBuilder().CreateComment(static_cast<ASTComment *>(AST));
					break;
				default:
					// Error: invalid declaration in class
					S.Diag(AST->getLocation(), diag::err_syntax_error);
					break;
			}
		}

		// Resolve Super Classes
		for (auto ClassTypeRef : Sym->getAST()->getSuperClasses()) {
			if (ResolveTypeRef(ClassTypeRef)) {
				SymType *SuperType = ClassTypeRef->getDef();

				if (SuperType->getKind() != SymTypeKind::TYPE_CLASS) {
					// Error: invalid superclass type
					S.Diag(ClassTypeRef->getLocation(), diag::err_syntax_error);
					break;
				}

				SymClass *SuperClass = static_cast<SymClass *>(SuperType);

				// Struct: Resolve Var in Super Classes
				if (SuperClass->getClassKind() == SymClassKind::STRUCT) {

					// Interface cannot extend a Struct
					if (Sym->getClassKind() == SymClassKind::INTERFACE) {
						S.Diag(ClassTypeRef->getLocation(), diag::err_sema_interface_ext_struct);
						return;
					}

					// Add Vars to the Struct
					for (auto SuperAttributeEntry: SuperClass->getAttributes()) {
						SymClassAttribute * SuperAttribute = SuperAttributeEntry.getValue();

						// Check Var already exists and type conflicts in Super Vars
						if (Sym->Attributes.lookup(SuperAttributeEntry.getKey())) {
							S.Diag(SuperAttribute->getAST()->getLocation(), diag::err_sema_super_struct_var_conflict);
						} else {
							Sym->Attributes.insert(std::make_pair(SuperAttributeEntry.getKey(), SuperAttribute));
						}
					}
				}

				// Interface cannot extend a Class
				if (Sym->getClassKind() == SymClassKind::INTERFACE &&
					SuperClass->getClassKind() == SymClassKind::CLASS) {
						S.Diag(SuperClass->getAST()->getLocation(), diag::err_sema_interface_ext_class);
						return;
					}

				// Class/Interface: take all Super Classes methods
				if (SuperClass->getClassKind() == SymClassKind::CLASS ||
					SuperClass->getClassKind() == SymClassKind::INTERFACE) {

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

		// FIXME
		// Check if all abstract methods are implemented


		// Set default values in attributes
		if (Sym->getClassKind() == SymClassKind::INTERFACE || Sym->getClassKind() == SymClassKind::STRUCT) {

			// Init null value attributes with default values
			for (auto AttributeEntry : Sym->Attributes) {
				SymClassAttribute *Attribute = AttributeEntry.getValue();
				// Generate default values
				if (Attribute->getAST()->getExpr() == nullptr) {
					ASTValue *DefaultValue = S.Builder->CreateDefaultValue(Attribute->getAST()->getTypeRef()->getDef());
					ASTValueExpr *ValueExpr = S.Builder->CreateExpr(DefaultValue);
					Attribute->AST->Expr = ValueExpr;
				}
				S.getValidator().CheckIsValueExpr(Attribute->getAST()->getExpr());
			}
		}

		// Create default constructor if there aren't any other constructors
		// FIXME this code remove default constructor
		//                if (!Class->Constructors.empty()) {
		//                    delete Class->DefaultConstructor;
		//                }

		// Constructors
		for (auto ConstructorEntry: Sym->Constructors) {

			// Resolve Attribute types
			for (auto &Attribute: Sym->Attributes) {
				// TODO
			}
			ResolveStmtBlock(ConstructorEntry.getSecond()->getAST()->Body);
		}

		// Methods
		for (auto MethodEntry : Sym->Methods) {
			SymClassMethod * Method = MethodEntry.getSecond();

			// Add Class vars for each Method
			for (auto &Attribute: Sym->Attributes) {

				// Check if Method already contains this var name as LocalVar
				if (!S.getValidator().CheckDuplicateLocalVars(Method->getAST()->Body, Attribute.getKey())) {
					return;
				}
			}

			if (Method->getAST()->getBody()) {
				ResolveStmtBlock(Method->getAST()->Body); // FIXME check if already resolved
			}
		}
	}
}

void SemaResolver::ResolveEnums() {
	for (auto Entry : Module->getEnums()) {
		SymEnum *Sym = Entry.getValue();

		if (Sym->Comment) {
			ResolveComment(Sym->Comment, Sym->getAST());
		}

		SymComment *Comment = nullptr;
		for (auto &AST: Sym->getAST()->getDefinitions()) {
			switch (AST->getKind()) {
			case ASTKind::AST_VAR: {
				S.getSymBuilder().CreateEnumEntry(Sym, static_cast<ASTVar *>(AST), Comment);
				Comment = nullptr;
			}	break;
			case ASTKind::AST_COMMENT:
				Comment = S.getSymBuilder().CreateComment(static_cast<ASTComment *>(AST));
				break;
			default:
				// Error: invalid declaration in class
					S.Diag(AST->getLocation(), diag::err_syntax_error);
				break;
			}
		}
	}
}

bool SemaResolver::ResolveTypeRef(ASTTypeRef *&TypeRef) {
	if (!TypeRef->isResolved()) {

		// TypeRef is an Array
		if (TypeRef->isArray()) {
			auto ArrayTypeRef = static_cast<ASTArrayTypeRef *>(TypeRef);
			return ResolveTypeRef(ArrayTypeRef->TypeRef);
		}

		// Type is Class or Enum
		SymType *Type = nullptr;
		if (TypeRef->getParent()) {

			// Resolve by Imports
			ASTIdentifier *Parent = nullptr;
			for (auto ImportEntry : Module->getImports()) {
				SymNameSpace * Import = ImportEntry.getValue();
				if (!TypeRef->Def)
					TypeRef->Def = Import->getTypes().lookup(TypeRef->getParent()->getName());
			}
		} else {
			// Resolve in current NameSpace
			TypeRef->Def = NameSpace->getTypes().lookup(TypeRef->getName());

			// Resolve in Default NameSpace
			if (!TypeRef->Def)
				TypeRef->Def = S.getSymTable().getDefaultNameSpace()->getTypes().lookup(TypeRef->getName());
		}

		// Take Identity from NameSpace
		TypeRef->Resolved = TypeRef->Def != nullptr; // Evict Cycle Loop: can be resolved only now
	}

	if (!TypeRef->Def) {
		S.Diag(TypeRef->getLocation(), diag::err_unref_type);
		return false;
	}

    return true;
}

bool SemaResolver::ResolveStmt(ASTStmt *Stmt) {
    switch (Stmt->getStmtKind()) {

        case ASTStmtKind::STMT_BLOCK:
            return ResolveStmtBlock(static_cast<ASTBlockStmt *>(Stmt));
		case ASTStmtKind::STMT_RULE: {
			ASTRuleStmt *Rule = static_cast<ASTRuleStmt *>(Stmt);
			return ResolveExpr(Rule->getParent(), Rule->getRule()) && ResolveStmt(Rule->getStmt());
		}
        case ASTStmtKind::STMT_IF:
            return ResolveStmtIf(static_cast<ASTIfStmt *>(Stmt));
        case ASTStmtKind::STMT_SWITCH:
            return ResolveStmtSwitch(static_cast<ASTSwitchStmt *>(Stmt));
        case ASTStmtKind::STMT_LOOP:
            return ResolveStmtLoop(static_cast<ASTLoopStmt *>(Stmt));
        case ASTStmtKind::STMT_LOOP_IN:
            return ResolveStmtLoopIn(static_cast<ASTLoopInStmt *>(Stmt));
        case ASTStmtKind::STMT_ASSIGN:
            return ResolveStmtVar(static_cast<ASTAssignmentStmt *>(Stmt));
        case ASTStmtKind::STMT_EXPR:
            return ResolveExpr(Stmt->Parent, static_cast<ASTExprStmt *>(Stmt)->Expr);
        case ASTStmtKind::STMT_FAIL:
            return ResolveStmtFail(static_cast<ASTFailStmt *>(Stmt));
        case ASTStmtKind::STMT_HANDLE:
            return ResolveStmtHandle(static_cast<ASTHandleStmt *>(Stmt));
        case ASTStmtKind::STMT_DELETE:
            return ResolveVarRef(Stmt->Parent, static_cast<ASTDeleteStmt *>(Stmt)->VarRef);
        case ASTStmtKind::STMT_RETURN: {
        	ASTReturnStmt *ReturnStmt = static_cast<ASTReturnStmt *>(Stmt);
        	ASTTypeRef * ReturnType = ReturnStmt->Parent->getFunction()->getReturnTypeRef(); // Force Return Expr to be of Return Type
			bool Success = true;
        	if (ReturnStmt->Expr != nullptr) {
            	Success = ResolveExpr(ReturnStmt->Parent, ReturnStmt->Expr, ReturnType->getDef()) &&
        		S.getValidator().CheckConvertibleTypes(ReturnStmt->getExpr()->getTypeRef()->getDef(),
        			ReturnStmt->getParent()->getFunction()->getReturnTypeRef()->getDef());
            } else {
            	if (!ReturnStmt->Parent->getFunction()->getReturnTypeRef()->getDef()->isVoid()) {
            		S.Diag(ReturnStmt->getLocation(), diag::err_invalid_return_type);
            	}
            }
            return Success;
        }
        case ASTStmtKind::STMT_BREAK:
        case ASTStmtKind::STMT_CONTINUE:
            return true;
    }

    assert(false && "Invalid ASTStmtKind");
}

bool SemaResolver::ResolveStmtBlock(ASTBlockStmt *Block) {
    // Resolve LocalVar Type
    for (auto &LocalVar : Block->LocalVars) {
        ResolveTypeRef(LocalVar.getValue()->TypeRef);
    }

    // Resolve Statements
    for (ASTStmt *Stmt : Block->Content) {
        ResolveStmt(Stmt);
    }

    // Check LocalVar initialization
    // TODO
//    for (auto &LocalVar : Block->LocalVars) {
//        if (!LocalVar.second->isInitialized())
//            S.Diag(LocalVar.getValue()->getLocation(), diag::err_sema_uninit_var) << LocalVar.getValue()->getName();
//    }

    return true;
}

bool SemaResolver::ResolveStmtIf(ASTIfStmt *IfStmt) {
    bool Success = ResolveExpr(IfStmt->getParent(), IfStmt->Rule, S.getSymTable().getBoolType()) &&
                   ResolveStmt(IfStmt->Stmt);
    for (ASTRuleStmt *Elsif : IfStmt->Elsif) {
        Success &= ResolveExpr(IfStmt->getParent(), Elsif->Rule, S.getSymTable().getBoolType()) &&
                   ResolveStmt(Elsif->Stmt);
    }
    if (Success && IfStmt->Else) {
        Success = ResolveStmt(IfStmt->Else);
    }
    return Success;
}

bool SemaResolver::ResolveStmtSwitch(ASTSwitchStmt *SwitchStmt) {
    assert(SwitchStmt && "Switch Block cannot be null");

    bool Success = ResolveVarRef(SwitchStmt->getParent(), SwitchStmt->VarRef) &&
    	SwitchStmt->getVarRef()->getDef()->getAST()->getTypeRef()->getDef()->isInteger();
    for (ASTRuleStmt *Case : SwitchStmt->Cases) {
    	Success &= ResolveExpr(SwitchStmt, Case->getRule());
        Success &= Case->getRule()->getTypeRef()->getDef()->isInteger() && ResolveStmt(Case);
    }
    return Success && ResolveStmt(SwitchStmt->Default);
}

bool SemaResolver::ResolveStmtLoop(ASTLoopStmt *LoopStmt) {
    // Check Loop is not null or empty
    bool Success = LoopStmt->Stmt != nullptr;

	if (LoopStmt->getRule() == nullptr) { // Error: empty condition expr
		S.Diag(diag::err_parse_empty_while_expr);
		Success = false;
	}

    // Check Init
    if (LoopStmt->Init) {
        LoopStmt->Stmt->Parent = LoopStmt->Init;
        Success = ResolveStmt(LoopStmt->Init) &&
        	ResolveExpr(LoopStmt->Init, LoopStmt->Rule, S.getSymTable().getBoolType());
    } else {
        Success = ResolveExpr(LoopStmt->Parent, LoopStmt->Rule, S.getSymTable().getBoolType());
    }
    Success = S.getValidator().CheckConvertibleTypes(LoopStmt->getRule()->getTypeRef()->Def, S.getSymTable().getBoolType());
    Success &= ResolveStmt(LoopStmt->Stmt);
    Success &= LoopStmt->Post ? ResolveStmt(LoopStmt->Post) : true;
    return Success;
}

bool SemaResolver::ResolveStmtLoopIn(ASTLoopInStmt *LoopInStmt) {
    return ResolveVarRef(LoopInStmt->Parent, LoopInStmt->VarRef) && ResolveStmtBlock(LoopInStmt->Block);
}

bool SemaResolver::ResolveStmtVar(ASTAssignmentStmt *VarStmt) {
    ResolveVarRef(VarStmt->Parent, VarStmt->VarRef);
    if (VarStmt->getVarRef()->getDef() && VarStmt->getExpr() != nullptr && !VarStmt->getVarRef()->getDef()->isInitialized())
        VarStmt->getVarRef()->getDef()->setInitialization(VarStmt); // FIXME ? with if - else
    return VarStmt->getVarRef()->getDef() &&
        ResolveExpr(VarStmt->Parent, VarStmt->Expr, VarStmt->getVarRef()->getDef()->getType());
}

bool SemaResolver::ResolveStmtFail(ASTFailStmt *FailStmt) {
	// Resolve Fail Expr
	if (FailStmt->Expr)
		return ResolveExpr(FailStmt->Parent, FailStmt->Expr);

    return true;
}

bool SemaResolver::ResolveStmtHandle(ASTHandleStmt *HandleStmt) {
    if (HandleStmt->ErrorHandlerRef)
        ResolveVarRef(HandleStmt->getParent(), HandleStmt->ErrorHandlerRef);
    return ResolveStmt(HandleStmt->Handle);
}

bool SemaResolver::ResolveIdentifier(SymNameSpace *NameSpace, ASTStmt *Stmt, ASTIdentifier *Identifier) {
    assert(NameSpace && "NameSpace cannot be null");
    assert(Identifier && "Identifier cannot be null");

    if (Identifier->isResolved() == false) {

        if (Identifier->isCall()) { // Call cannot be undefined
            ASTCall *Call = static_cast<ASTCall *>(Identifier);

            // NameSpace.ConstructorCall()... or NameSpace.Call()...
            Identifier->Resolved = ResolveFunctionCall(NameSpace, Stmt, Call);
            if (Identifier->isResolved() == false) {

                // Constructor
                if (Call->getCallKind() == ASTCallKind::CALL_NEW ||
                    Call->getCallKind() == ASTCallKind::CALL_NEW_UNIQUE ||
                    Call->getCallKind() == ASTCallKind::CALL_NEW_SHARED ||
                    Call->getCallKind() == ASTCallKind::CALL_NEW_WEAK) {
                    ASTIdentity *Identity = FindIdentity(Call->getName(), NameSpace);
                    if (Identity)
                        Call->Resolved = ResolveStaticCall(Identity, Stmt, Call);
                } else {
                    // Function
                    ASTFunction *Function = FindFunction(Call, NameSpace);
                    if (Function) {
                        Call->Def = Function;
                        Call->Resolved = true;
                    }
                }
            }

            // Resolve Child
            if (Identifier->isResolved() && Call->getChild()) {
                ASTTypeRef *TypeRef = Call->getDef()->getAST()->getReturnTypeRef();
                if (!TypeRef->isResolved() && ResolveTypeRef(TypeRef)) {
                    // Can be only a Call or a Var
                	return ResolveIdentifier(TypeRef->getDef(), Stmt, Call->getChild());
                } else {
                    // Error: cannot access to not identity var
                    // TODO
                }
            }
        } else {
        	// Ref is a Type
            // NameSpace.Type...Call() or NameSpace.Type...Var
            SymType *Type = NameSpace->getTypes().lookup(Identifier->getName());
            if (Type) {
                if (Type->isClass())
                    Identifier = S.Builder->CreateClassType(Identifier);
                else if (Type->isEnum())
                    Identifier = S.Builder->CreateEnumType(Identifier);
                static_cast<ASTTypeRef *>(Identifier)->Def = Type;
                Identifier->Resolved = true;

                // Resolve Child
                if (Identifier->getChild()) {
                    if (Identifier->getChild()->isCall()) {
                        return ResolveStaticCall(Type, Stmt, static_cast<ASTCall *>(Identifier->getChild()));
                    } else {
                        return ResolveStaticVarRef(Type, Stmt, static_cast<ASTVarRef *>(Identifier->getChild()));
                    }
                }

                return Identifier->Resolved;
            }

            // Identity is GlobalVar
            // NameSpace.GlobalVar...Call() or NameSpace.GlobalVar...Var
            return ResolveGlobalVarRef(NameSpace, Stmt, static_cast<ASTVarRef *>(Identifier));
        }
    }

    return Identifier->isResolved();
}

bool SemaResolver::ResolveIdentifier(ASTIdentity *Identity, ASTStmt *Stmt, ASTIdentifier *Identifier) {
    assert(Identity && "Identity cannot be null");
    assert(Identifier && "Identifier cannot be null");

    if (Identifier->isResolved() == false) {
        if (Identifier->isCall()) { // Call cannot be undefined
            return ResolveStaticCall(Identity, Stmt, static_cast<ASTCall*>(Identifier));
        } else {
            return ResolveStaticVarRef(Identity, Stmt, static_cast<ASTVarRef*>(Identifier));
        }
    }

    return Identifier->Resolved;
}

bool SemaResolver::ResolveIdentifier(ASTStmt *Stmt, ASTIdentifier *Ref) {
    assert(Stmt && "Stmt cannot be null");
    assert(Ref && "Identifier cannot be null");

    if (Ref->isResolved() == false) {
        if (Ref->isCall()) { // Call cannot be undefined
            ASTCall *Call = static_cast<ASTCall*>(Ref);

            if (Call->getCallKind() == ASTCallKind::CALL_NEW) {
                ASTIdentity *Identity = FindIdentity(Call->getName(), NameSpace);
                if (Identity)
                    Call->Resolved = ResolveStaticCall(Identity, Stmt, Call);
            } else {

                // call function()
                ASTFunction *Function = FindFunction(Call, NameSpace);
                if (Function == nullptr) {
                    Function = FindFunction(Call, S.getSymTable().getDefaultNameSpace());
                    Call->Def = Function;
                    Call->Resolved = true;
                }

                // call method()
                if (Function == nullptr) {
                    if (Stmt->getFunction()->getKind() == ASTFunctionKind::CLASS_METHOD) {
                        ASTClass *Class = static_cast<ASTFunction*>(Stmt->getFunction())->getClass();
                        Function = FindClassMethod(Call, Class);
                        Call->Def = Function;
                        Call->Resolved = true;
                    }
                }
            }

            // Resolve child
            if (Call->Def && Call->getChild()) {
                ASTTypeRef *TypeRef = Call->getDef()->getAST()->getReturnTypeRef();
                if (!TypeRef->isIdentity()) {
                    // Error: cannot access to not identity var
                }
                if (ResolveTypeRef(TypeRef))
                    // Can be only a Call or a Var
                    return ResolveIdentifier(static_cast<ASTTypeRef*>(TypeRef)->getDef(), Stmt, Call->getChild());
            }

        } else if (Ref->isVarRef()) {
            auto *VarRef = static_cast<ASTVarRef*>(Ref);

            // Search in LocalVars
            // LocalVar.Var or ClassAttribute.Var
            ASTVar *Var = FindLocalVar(Stmt, Ref->getName());

            // Check if Function is a class Method
            if (Var == nullptr) {
                ASTFunction *Function = Stmt->getFunction();

                // Search for Class Vars if Var is Class Method
                if (Function->getKind() == ASTFunctionKind::CLASS_METHOD) {
                    for (auto &Attribute: static_cast<ASTFunction*>(Function)->getClass()->Attributes) {
                        if (Attribute->getName() == Ref->getName()) {
                            Var = Attribute;
                        }
                    }
                }
            }

            if (Var) {
                VarRef->Def = Var;
                VarRef->Resolved = true;

                // Resolve Child
                if (VarRef->getChild() && Var->getTypeRef()->isIdentity()) {
                    ASTTypeRef *IdentityType = static_cast<ASTTypeRef*>(Var->getTypeRef());
                    return ResolveIdentifier(IdentityType->getDef(), Stmt, VarRef->getChild());
                }
            }
        } else {
            ASTTypeRef *TypeRef = S.Builder->CreateTypeRef(Ref);
            if (ResolveTypeRef(TypeRef) && TypeRef->getChild()) {
                return ResolveIdentifier(TypeRef->getDef(), Stmt, TypeRef->getChild());
            }
        }
    }

    return Ref->Resolved;
}

bool SemaResolver::ResolveGlobalVarRef(SymNameSpace *NameSpace, ASTStmt *Stmt, ASTVarRef *VarRef) {
    assert(NameSpace && "NameSpace cannot be null");
    assert(VarRef && "VarRef cannot be null");

    if (VarRef->isResolved() == false) {
    	SymVar *GlobalVar = nullptr;
    	if (NameSpace) {
    		GlobalVar = NameSpace->GlobalVars.lookup(VarRef->getName());
    	} else {
    		// Search in Module
    		GlobalVar = Module->getGlobalVars().lookup(VarRef->getName());

    		// Search in Default NameSpace
    		if (!GlobalVar)
    			GlobalVar = S.getSymTable().getDefaultNameSpace()->getGlobalVars().lookup(VarRef->getName());
    	}

    	// GlobalVar found
    	if (GlobalVar) {
            VarRef->Def = &GlobalVar->getAST()->Def;
            VarRef->Resolved = true;
        }
    }
    return VarRef->Resolved;
}

bool SemaResolver::ResolveStaticVarRef(SymIdentity *Identity, ASTStmt *Stmt, ASTVarRef *VarRef) {
    assert(Identity && "Identity cannot be null");
    assert(VarRef && "VarRef cannot be null");

    if (VarRef->isResolved() == false) {
        if (Identity->getIdentity()->getIdentityKind() == ASTIdentityKind::ID_CLASS) {
            ASTVar *Attribute = static_cast<ASTClass *>(Identity)->getAttributes().lookup(VarRef->getName());
            if (Attribute) {
                VarRef->Def = Attribute;
                VarRef->Resolved = true;
            }
        } else if (Identity->getIdentityKind() == ASTIdentityKind::ID_ENUM) {
            ASTEnumEntry *Entry = static_cast<ASTEnum *>(Identity)->getEntries().lookup(VarRef->getName());
            if (Entry) {
                VarRef->Def = Entry;
                VarRef->Resolved = true;
            }
        }
    }

    return VarRef->Resolved;
}

/**
 * ResolveModule a VarRef with its declaration
 * @param VarRef
 * @return true if no error occurs, otherwise false
 */
bool SemaResolver::ResolveVarRef(ASTStmt *Stmt, ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveVarRefWithParent", Logger().Attr("VarRef", VarRef).End());
    assert(Stmt && "Stmt cannot be null");
    assert(VarRef && "VarRef cannot be null");

    if (VarRef->isResolved() == false) {

        ASTIdentifier *Current = nullptr;
        SymNameSpace *VarNameSpace = FindNameSpace(VarRef, Current);
        if (VarNameSpace)
            Current->Parent = S.Builder->CreateNameSpace(VarNameSpace);

        VarRef->Resolved = (VarNameSpace && ResolveIdentifier(VarNameSpace, Stmt, Current)) || // Resolve in NameSpace: Type, Function, GlobalVar
               ResolveIdentifier(Stmt, Current) || // Resolve in statements as LocalVar
               ResolveIdentifier(NameSpace, Stmt, Current) ||
               ResolveIdentifier(S.getSymTable().getDefaultNameSpace(), Stmt, Current); // Default NameSpace, Class Method or Attribute, LocalVar
    }

    // VarRef not found in Module, namespace and Module imports
    if (VarRef->isResolved() == false) {
        S.Diag(VarRef->getLocation(), diag::err_unref_var) << VarRef->Name;
        return false;
    }

    return true;
}

bool SemaResolver::ResolveFunctionCall(SymNameSpace *NameSpace, ASTStmt *Stmt, ASTCall *Call) {
    assert(NameSpace && "NameSpace cannot be null");
    assert(Call && "Call cannot be null");

    if (ResolveCallArgs(Stmt, Call)) {
        ASTFunction *Function = FindFunction(Call, NameSpace);
        if (Function) {
            Call->Def = Function;
            Call->Resolved = true;
        }
    }
    return Call->isResolved();
}

bool SemaResolver::ResolveStaticCall(ASTIdentity *Identity, ASTStmt *Stmt, ASTCall *Call) {
    assert(Identity && "Identity cannot be null");
    assert(Call && "Call cannot be null");

    if (ResolveCallArgs(Stmt, Call)) {
        if (Identity->getIdentityKind() == ASTIdentityKind::ID_CLASS) {
            ASTFunction *Method = FindClassMethod(Call, static_cast<ASTClass *>(Identity));
            Call->Def = Method;
            Call->Resolved = true;
        } else {
            S.Diag(Call->getLocation(), diag::err_sema_call_enum);
        }
    }

    return Call->Resolved;
}

bool SemaResolver::ResolveCall(ASTStmt *Stmt, ASTCall *Call) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveCall", Logger().Attr("Call", Call).End());
    assert(Stmt && "Stmt cannot be null");
    assert(Call && "Call cannot be null");

    if (Call->isResolved() == false) {

        ASTIdentifier *Current = nullptr;
        SymNameSpace *CallNameSpace = FindNameSpace(Call, Current);
        if (CallNameSpace)
            Current->Parent = S.Builder->CreateNameSpace(CallNameSpace);

        // Resolve Expression in Arguments
        bool ResolvedArgs = true;
        for (ASTArg *Arg : Call->getArgs()) {
            ResolvedArgs &= ResolveExpr(Stmt, Arg->getExpr());
        }

        // if Arguments are not resolved is not possible go ahead with call reference resolution
        // cannot resolve with the function parameters types
        if (!ResolvedArgs) {
            return false;
        }

        // resolve identifier from most significant to less
        Call->Resolved = (CallNameSpace && ResolveIdentifier(CallNameSpace, Stmt, Current)) || // Resolve in NameSpace: Type, Function, GlobalVar
           ResolveIdentifier(Stmt, Current) || // Resolve in statements as LocalVar
           ResolveIdentifier(NameSpace, Stmt, Current) || // Module NameSpace
           ResolveIdentifier(S.getSymTable().getDefaultNameSpace(), Stmt, Current); // Default NameSpace, Class Method or Attribute, LocalVar
    }

    // VarRef not found in Module, namespace and Module imports
    if (Call->isResolved() == false) {
        S.Diag(Call->getLocation(), diag::err_unref_call) << Call->getName();
    }

    // Search from until parent is null or parent is a Handle Stmt
    ASTStmt *Parent = Stmt;
    while (Call->ErrorHandler == nullptr) {
        Parent = Parent->getParent();
        if (Parent == nullptr) {
            Call->ErrorHandler = Stmt->getFunction()->getErrorHandler();
        } else if (Parent->getStmtKind() == ASTStmtKind::STMT_HANDLE) {
            ASTHandleStmt *HandleStmt = static_cast<ASTHandleStmt*>(Parent);
            if (HandleStmt->ErrorHandlerRef != nullptr) {
                Call->ErrorHandler = HandleStmt->ErrorHandlerRef->Def;
            }
        }
    }

    return Call->isResolved();
}

bool SemaResolver::ResolveCallArgs(ASTStmt *Stmt, ASTCall *Call) {
    bool Resolved =  true;
    for (auto &Arg : Call->getArgs()) {
        Resolved &= ResolveExpr(Stmt, Arg->getExpr());
    }
    return Resolved;
}

/**
 * ResolveModule Expr contents
 * @param Expr
 * @return true if no error occurs, otherwise false
 */
bool SemaResolver::ResolveExpr(ASTStmt *Stmt, ASTExpr *Expr, SymType *Type) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveExpr", Logger()
            .Attr("Expr", Expr)
            .Attr("Type", Expr).End());

    bool Success = false;
    switch (Expr->getExprKind()) {
        case ASTExprKind::EXPR_VALUE: {
            // Select the best option for this Value
            ASTValueExpr *ValueExpr = static_cast<ASTValueExpr*>(Expr);
            if (Type != nullptr)
                ValueExpr->getTypeRef()->Def = Type;
            return S.getValidator().CheckValue(ValueExpr->getValue());
        }
        case ASTExprKind::EXPR_VAR_REF: {
            ASTVarRef *VarRef = static_cast<ASTVarRefExpr*>(Expr)->getVarRef();
            if (ResolveVarRef(Stmt, VarRef)) {
                Expr->getTypeRef()->Def = VarRef->getDef()->getAST()->getTypeRef()->getDef();
                Success = true;
                break;
            } else {
                return false;
            }
        }
        case ASTExprKind::EXPR_CALL: {
            ASTCall *Call = static_cast<ASTCallExpr*>(Expr)->getCall();
            if (ResolveCall(Stmt, Call)) {
                switch (Call->getCallKind()) {

                    case ASTCallKind::CALL_FUNCTION:
                        Expr->getTypeRef()->Def = Call->getDef()->getAST()->getReturnTypeRef()->getDef();
                        break;
                    case ASTCallKind::CALL_NEW: {
                        ASTFunction *AST = Call->getDef()->getAST();
                    	// FIXME NameSpace exists?
                    	SymType *Type = S.getSymTable().getDefaultNameSpace()->getTypes().lookup(AST->getName());
                        Expr->TypeRef->Def = Def->getClass()->getType();
                    }
                    break;
                }
                Success = true;
                break;
            }
            return false;
        }
        case ASTExprKind::EXPR_OP: {
            switch (static_cast<ASTOpExpr*>(Expr)->getOpExprKind()) {
                case ASTOpExprKind::OP_UNARY: {
                    ASTUnaryOpExpr *Unary = static_cast<ASTUnaryOpExpr*>(Expr);
                    Success = ResolveExpr(Stmt, const_cast<ASTExpr*>(Unary->Expr));
                    Expr->getTypeRef()->Def = Unary->getExpr()->getTypeRef()->getDef();
                    break;
                }
                case ASTOpExprKind::OP_BINARY: {
                    ASTBinaryOpExpr *Binary = static_cast<ASTBinaryOpExpr*>(Expr);

                    Success = ResolveExpr(Stmt, Binary->LeftExpr) && ResolveExpr(Stmt, Binary->RightExpr);
                    if (Success) {

                    	// Check if Left and Right Expr are resolved
                    	SymType * LeftType = Binary->getLeftExpr()->getTypeRef()->getDef();
                    	SymType * RightType = Binary->getRightExpr()->getTypeRef()->getDef();

                        if (Binary->getTypeKind() == ASTBinaryOpTypeExprKind::OP_BINARY_ARITH ||
                                Binary->getTypeKind() == ASTBinaryOpTypeExprKind::OP_BINARY_COMPARISON) {

                        	// Check Compatible Types Bool/Bool, Float/Float, Integer/Integer
                            Success = S.getValidator().CheckArithTypes(Binary->getLeftExpr()->getTypeRef()->getDef(),
                                                                  Binary->getRightExpr()->getTypeRef()->getDef());

                            if (Success) {
                            	// Set respectively the Left or Right Expr Type by chose the Expr which is not a Value Type
                            	// Ex.
                            	// int a = 0
                            	// int b = a + 1
                            	// 1 will have type int
                            	if (Binary->getLeftExpr()->getExprKind() == ASTExprKind::EXPR_VALUE &&
									Binary->getRightExpr()->getExprKind() != ASTExprKind::EXPR_VALUE) {
                            		Binary->getLeftExpr()->getTypeRef()->Def = RightType;
								} else if (Binary->getRightExpr()->getExprKind() == ASTExprKind::EXPR_VALUE &&
                            		Binary->getLeftExpr()->getExprKind() != ASTExprKind::EXPR_VALUE) {
                            		Binary->getRightExpr()->getTypeRef()->Def = LeftType;
                            	}

                                // Promotes First or Second Expr Types in order to be equal
                                if (LeftType->isInteger()) {
                                    if (static_cast<SymTypeInt *>(LeftType)->getIntKind() >
                                    	static_cast<SymTypeInt*>(RightType)->getIntKind())
                                        Binary->getTypeRef()->Def = LeftType;
                                    else
                                        Binary->getTypeRef()->Def = RightType;
                                } else if (LeftType->isFloatingPoint()) {
                                    if (static_cast<SymTypeFP*>(LeftType)->getFPKind() >
                                    	static_cast<SymTypeFP*>(RightType)->getFPKind())
                                		Binary->getTypeRef()->Def = LeftType;
                                    else
                                		Binary->getTypeRef()->Def = RightType;
                                }

                                Binary->getTypeRef()->Def = Binary->getTypeKind() == ASTBinaryOpTypeExprKind::OP_BINARY_ARITH ?
                                               LeftType : S.getSymTable().getBoolType();
                            } else {
                            	S.Diag(Binary->getLocation(), diag::err_sema_types_operation)
											<< LeftType->getName()
											<< RightType->getName();
                            }
                        } else if (Binary->getTypeKind() == ASTBinaryOpTypeExprKind::OP_BINARY_LOGIC) {
                            Success = S.getValidator().CheckLogicalTypes(LeftType, RightType);
                        	if (Success) {
                        		Binary->getTypeRef()->Def = S.getSymTable().getBoolType();
                        	} else {
                        		S.Diag(Binary->getLocation(), diag::err_sema_types_logical)
									<< LeftType->getName()
									<< RightType->getName();
                        	}
                        }
                    }
                    break;
                }
                case ASTOpExprKind::OP_TERNARY: {
                    ASTTernaryOpExpr *Ternary = static_cast<ASTTernaryOpExpr*>(Expr);
                    Success = ResolveExpr(Stmt, Ternary->ConditionExpr) &&
                              S.getValidator().CheckConvertibleTypes(Ternary->getConditionExpr()->getTypeRef()->getDef(), S.getSymTable().getBoolType()) &&
                              ResolveExpr(Stmt, Ternary->TrueExpr) &&
                              ResolveExpr(Stmt, Ternary->FalseExpr);
                    Ternary->getTypeRef()->Def = Ternary->getTrueExpr()->getTypeRef()->getDef(); // The group type is equals to the second type
                    break;
                }
            }
            break;
        }
        default:
            assert(0 && "Invalid ASTExprKind");
    }

    return Success;
}

SymNameSpace *SemaResolver:: FindNameSpace(ASTIdentifier *Identifier, ASTIdentifier *&Current) const {
    // Find NameSpace by iterating parents
    // one.two.three.four
    // four.Parent, three.Parent, two.Parent, one.Parent
    SymNameSpace *NameSpace = nullptr;
    Current = Identifier;
    while (Current->getParent()) {
        // Check from Imports
        NameSpace = S.getSymTable().getNameSpaces().lookup(Current->getParent()->getFullName());
        if (NameSpace)
            break;
        Current = Current->getParent();
    }

    return NameSpace;
}

/**
 * Search a VarRef into declared Block's vars
 * If found set LocalVar
 * @param Stmt
 * @param Name
 * @return the found LocalVar
 */
ASTVar *SemaResolver::FindLocalVar(ASTStmt *Stmt, llvm::StringRef Name) const {
    FLY_DEBUG_MESSAGE("Sema", "FindLocalVar", Logger().Attr("Parent", Stmt).Attr("Name", Name).End());

	if (Stmt->getStmtKind() == ASTStmtKind::STMT_BLOCK) {
		ASTBlockStmt *Block = static_cast<ASTBlockStmt*>(Stmt);
		const auto &It = Block->getLocalVars().find(Name);
		if (It != Block->getLocalVars().end()) { // Search into this Block
			return It->getValue();
		}
	}

	if (Stmt->getParent()) { // search recursively into Parent Blocks to find the right Var definition
        return FindLocalVar(Stmt->getParent(), Name);
    } else { // Search into Parameter list
        llvm::SmallVector<ASTVar *, 8> Params = Stmt->getFunction()->getParams();
        for (auto &Param : Params) {
            if (Param->getName() == Name) {
                return Param;
            }
        }
    }
    return nullptr;
}
