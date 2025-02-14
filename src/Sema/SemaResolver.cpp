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
#include "AST/ASTVarRef.h"
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
#include "AST/ASTFailStmt.h"
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
#include <Sym/SymEnumEntry.h>
#include <Sym/SymFunction.h>
#include <Sym/SymGlobalVar.h>
#include <Sym/SymLocalVar.h>

using namespace fly;

SemaResolver::SemaResolver(Sema &S, ASTModule *Module) :
    S(S), Default(S.getSymTable().DefaultNameSpace), NameSpace(S.getSymBuilder().CreateOrGetNameSpace(Module->getNameSpace())),
	Module(S.getSymBuilder().CreateModule(Module)) {

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
	for (auto &Entry : Module->Imports) {

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
	for (auto &Entry : Module->getGlobalVars()) {
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
	for (auto &Entry : Module->getFunctions()) {
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
	for (auto &ClassEntry : Module->getClasses()) {
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
				SymType *SuperType = ClassTypeRef->getType();

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
					for (auto &SuperAttributeEntry: SuperClass->getAttributes()) {
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
			for (auto &AttributeEntry : Sym->getAttributes()) {
				SymClassAttribute *Attribute = AttributeEntry.getValue();
				// Generate default values
				if (Attribute->getAST()->getExpr() == nullptr) {
					ASTValue *DefaultValue = S.getASTBuilder().CreateDefaultValue(Attribute->getType());
					ASTValueExpr *ValueExpr = S.getASTBuilder().CreateExpr(DefaultValue);
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
		for (auto &ConstructorEntry: Sym->getConstructors()) {

			// Resolve Attribute types
			for (auto &Attribute: Sym->Attributes) {
				// TODO
			}
			ResolveStmtBlock(ConstructorEntry.getValue()->getAST()->Body);
		}

		// Methods
		for (auto &MethodEntry : Sym->getMethods()) {
			SymClassMethod * Method = MethodEntry.getValue();

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
	for (auto &Entry : Module->getEnums()) {
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
			ASTRef *Parent = nullptr;
			for (auto &ImportEntry : Module->getImports()) {
				SymNameSpace * Import = ImportEntry.getValue();
				if (!TypeRef->Type)
					TypeRef->Type = Import->getTypes().lookup(TypeRef->getParent()->getName());
			}
		} else {
			// Resolve in current NameSpace
			TypeRef->Type = NameSpace->getTypes().lookup(TypeRef->getName());

			// Resolve in Default NameSpace
			if (!TypeRef->Type)
				TypeRef->Type = S.getSymTable().getDefaultNameSpace()->getTypes().lookup(TypeRef->getName());
		}

		// Take Identity from NameSpace
		TypeRef->Resolved = TypeRef->Type != nullptr; // Evict Cycle Loop: can be resolved only now
	}

	if (!TypeRef->Type) {
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
        case ASTStmtKind::STMT_VAR:
            return ResolveStmtVar(static_cast<ASTVarStmt *>(Stmt));
        case ASTStmtKind::STMT_EXPR:
            return ResolveExpr(Stmt->Parent, static_cast<ASTExprStmt *>(Stmt)->Expr);
        case ASTStmtKind::STMT_FAIL:
            return ResolveStmtFail(static_cast<ASTFailStmt *>(Stmt));
        case ASTStmtKind::STMT_HANDLE:
            return ResolveStmtHandle(static_cast<ASTHandleStmt *>(Stmt));
        case ASTStmtKind::STMT_DELETE:
            return ResolveRef(Stmt->Parent, static_cast<ASTDeleteStmt *>(Stmt)->VarRef);
        case ASTStmtKind::STMT_RETURN: {
        	ASTReturnStmt *ReturnStmt = static_cast<ASTReturnStmt *>(Stmt);
        	ASTTypeRef * ReturnType = ReturnStmt->Parent->getFunction()->getReturnTypeRef(); // Force Return Expr to be of Return Type
			bool Success = true;
        	if (ReturnStmt->Expr != nullptr) {
            	Success = ResolveExpr(ReturnStmt->Parent, ReturnStmt->Expr, ReturnType->getType()) &&
        		S.getValidator().CheckConvertibleTypes(ReturnStmt->getExpr()->getTypeRef()->getType(),
        			ReturnStmt->getParent()->getFunction()->getReturnTypeRef()->getType());
            } else {
            	if (!ReturnStmt->Parent->getFunction()->getReturnTypeRef()->getType()->isVoid()) {
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

    bool Success = ResolveRef(SwitchStmt->getParent(), SwitchStmt->VarRef) &&
    	SwitchStmt->getVarRef()->getVar()->getType()->isInteger();
    for (ASTRuleStmt *Case : SwitchStmt->Cases) {
    	Success &= ResolveExpr(SwitchStmt, Case->getRule());
        Success &= Case->getRule()->getTypeRef()->getType()->isInteger() && ResolveStmt(Case);
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
    Success = S.getValidator().CheckConvertibleTypes(LoopStmt->getRule()->getTypeRef()->Type, S.getSymTable().getBoolType());
    Success &= ResolveStmt(LoopStmt->Stmt);
    Success &= LoopStmt->Post ? ResolveStmt(LoopStmt->Post) : true;
    return Success;
}

bool SemaResolver::ResolveStmtLoopIn(ASTLoopInStmt *LoopInStmt) {
    return ResolveRef(LoopInStmt->Parent, LoopInStmt->VarRef) && ResolveStmtBlock(LoopInStmt->Block);
}

bool SemaResolver::ResolveStmtVar(ASTVarStmt *VarStmt) {
    ResolveRef(VarStmt->Parent, VarStmt->VarRef);
	SymVar *Var = VarStmt->getVarRef()->getVar();
    if (Var && VarStmt->getExpr() != nullptr) {
    	return ResolveExpr(VarStmt->Parent, VarStmt->Expr, Var->getType());
    }
	return false;
}

bool SemaResolver::ResolveStmtFail(ASTFailStmt *FailStmt) {
	// Resolve Fail Expr
	if (FailStmt->Expr)
		return ResolveExpr(FailStmt->Parent, FailStmt->Expr);

    return true;
}

bool SemaResolver::ResolveStmtHandle(ASTHandleStmt *HandleStmt) {
    if (HandleStmt->ErrorHandlerRef)
        ResolveRef(HandleStmt->getParent(), HandleStmt->ErrorHandlerRef);
    return ResolveStmt(HandleStmt->Handle);
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
                ValueExpr->getTypeRef()->Type = Type;
            return S.getValidator().CheckValue(ValueExpr->getValue());
        }
        case ASTExprKind::EXPR_VAR_REF: {
            ASTVarRef *VarRef = static_cast<ASTVarRefExpr*>(Expr)->getVarRef();
            if (ResolveRef(Stmt, VarRef)) {
                Expr->getTypeRef()->Type = VarRef->getVar()->getType();
                Success = true;
                break;
            } else {
                return false;
            }
        }
        case ASTExprKind::EXPR_CALL: {
            ASTCall *Call = static_cast<ASTCallExpr*>(Expr)->getCall();
            if (ResolveRef(Stmt, Call)) {
                switch (Call->getCallKind()) {

                    case ASTCallKind::CALL_FUNCTION:
                        Expr->getTypeRef()->Type = Call->getFunction()->getReturnType();
                        break;
                    case ASTCallKind::CALL_NEW: {
                        ASTFunction *AST = Call->getFunction()->getAST();
                    	// FIXME NameSpace exists?
                    	SymType *Type = S.getSymTable().getDefaultNameSpace()->getTypes().lookup(AST->getName());
                        Expr->getTypeRef()->Type = Type;
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
                    Expr->getTypeRef()->Type = Unary->getExpr()->getTypeRef()->getType();
                    break;
                }
                case ASTOpExprKind::OP_BINARY: {
                    ASTBinaryOpExpr *Binary = static_cast<ASTBinaryOpExpr*>(Expr);

                    Success = ResolveExpr(Stmt, Binary->LeftExpr) && ResolveExpr(Stmt, Binary->RightExpr);
                    if (Success) {

                    	// Check if Left and Right Expr are resolved
                    	SymType * LeftType = Binary->getLeftExpr()->getTypeRef()->getType();
                    	SymType * RightType = Binary->getRightExpr()->getTypeRef()->getType();

                        if (Binary->getTypeKind() == ASTBinaryOpTypeExprKind::OP_BINARY_ARITH ||
                                Binary->getTypeKind() == ASTBinaryOpTypeExprKind::OP_BINARY_COMPARISON) {

                        	// Check Compatible Types Bool/Bool, Float/Float, Integer/Integer
                            Success = S.getValidator().CheckArithTypes(Binary->getLeftExpr()->getTypeRef()->getType(),
                                                                  Binary->getRightExpr()->getTypeRef()->getType());

                            if (Success) {
                            	// Set respectively the Left or Right Expr Type by chose the Expr which is not a Value Type
                            	// Ex.
                            	// int a = 0
                            	// int b = a + 1
                            	// 1 will have type int
                            	if (Binary->getLeftExpr()->getExprKind() == ASTExprKind::EXPR_VALUE &&
									Binary->getRightExpr()->getExprKind() != ASTExprKind::EXPR_VALUE) {
                            		Binary->getLeftExpr()->getTypeRef()->Type = RightType;
								} else if (Binary->getRightExpr()->getExprKind() == ASTExprKind::EXPR_VALUE &&
                            		Binary->getLeftExpr()->getExprKind() != ASTExprKind::EXPR_VALUE) {
                            		Binary->getRightExpr()->getTypeRef()->Type = LeftType;
                            	}

                                // Promotes First or Second Expr Types in order to be equal
                                if (LeftType->isInteger()) {
                                    if (static_cast<SymTypeInt *>(LeftType)->getIntKind() >
                                    	static_cast<SymTypeInt*>(RightType)->getIntKind())
                                        Binary->getTypeRef()->Type = LeftType;
                                    else
                                        Binary->getTypeRef()->Type = RightType;
                                } else if (LeftType->isFloatingPoint()) {
                                    if (static_cast<SymTypeFP*>(LeftType)->getFPKind() >
                                    	static_cast<SymTypeFP*>(RightType)->getFPKind())
                                		Binary->getTypeRef()->Type = LeftType;
                                    else
                                		Binary->getTypeRef()->Type = RightType;
                                }

                                Binary->getTypeRef()->Type = Binary->getTypeKind() == ASTBinaryOpTypeExprKind::OP_BINARY_ARITH ?
                                               LeftType : S.getSymTable().getBoolType();
                            } else {
                            	S.Diag(Binary->getLocation(), diag::err_sema_types_operation)
											<< LeftType->getName()
											<< RightType->getName();
                            }
                        } else if (Binary->getTypeKind() == ASTBinaryOpTypeExprKind::OP_BINARY_LOGIC) {
                            Success = S.getValidator().CheckLogicalTypes(LeftType, RightType);
                        	if (Success) {
                        		Binary->getTypeRef()->Type = S.getSymTable().getBoolType();
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
                              S.getValidator().CheckConvertibleTypes(Ternary->getConditionExpr()->getTypeRef()->getType(), S.getSymTable().getBoolType()) &&
                              ResolveExpr(Stmt, Ternary->TrueExpr) &&
                              ResolveExpr(Stmt, Ternary->FalseExpr);
                    Ternary->getTypeRef()->Type = Ternary->getTrueExpr()->getTypeRef()->getType(); // The group type is equals to the second type
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

/**
 * Resolve a Call Reference
 * @param Stmt
 * @param Call
 * @param NameSpaces
 * @param ...
 * @return
 */
ASTRef *SemaResolver::ResolveCall(ASTStmt *Stmt, ASTCall *Call, llvm::SmallVector<SymNameSpace *, 4> &NameSpaces) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveCall", Logger().Attr("Call", Call).End());
    assert(Stmt && "Stmt cannot be null");
    assert(Call && "Call cannot be null");

    if (Call->isResolved() == false) {

    	// Resolve Expression in Arguments
    	llvm::SmallVector<SymType *, 8> CallTypes = ResolveCallArgTypes(Stmt, Call);

    	// if Arguments are not resolved is not possible go ahead with call reference resolution
    	// cannot resolve with the function parameters types
    	std::string Mangled = SymFunctionBase::MangleFunction(Call->Name, CallTypes);

    	// Constructor
        if (Call->getCallKind() == ASTCallKind::CALL_NEW ||
            Call->getCallKind() == ASTCallKind::CALL_NEW_UNIQUE ||
            Call->getCallKind() == ASTCallKind::CALL_NEW_SHARED ||
            Call->getCallKind() == ASTCallKind::CALL_NEW_WEAK) {

            // Take the Type from the NameSpace
            SymType *Type = FindType(Call->getName(), NameSpaces);
            if (Type && Type->isClass()) {
				SymClass *Class = static_cast<SymClass *>(Type);
				SymFunctionBase *Constructor = Class->getConstructors().lookup(Mangled);
                Call->Function = &Constructor;
                Call->Resolved = true;
                if (Call->Child)
                	return ResolveRef(Stmt, Type, Call->Child);
            }
        } else {

        	for (auto &NS : NameSpaces) {
        		// Take the Function
        		SymFunctionBase *Func = NS->getFunctions().lookup(Mangled);
        		if (Func) {
        			Call->Function = &Func;
        			Call->Resolved = true;
        			if (Call->Child)
        				return ResolveRef(Stmt, Func->getReturnType(), Call->Child);
        		}
        	}
        }

    	return Call;
    }

    // VarRef not found in Module, namespace and Module imports
    if (Call->isResolved() == false) {
        S.Diag(Call->getLocation(), diag::err_unref_call) << Call->getName();
    }

    // Search from until parent is null or parent is a Handle Stmt
	// When Parent Stmt is nullptr assign Function ErrorHandler to Call ErrorHandler
    ASTStmt *Parent = Stmt;
    while (Call->getErrorHandler() == nullptr) {
        Parent = Parent->getParent();
        if (Parent == nullptr) {
            Call->ErrorHandler->Sym = Stmt->getFunction()->getSym()->getErrorHandler();
        } else if (Parent->getStmtKind() == ASTStmtKind::STMT_HANDLE) {
            ASTHandleStmt *HandleStmt = static_cast<ASTHandleStmt*>(Parent);
            if (HandleStmt->getErrorHandlerRef() != nullptr) {
                Call->ErrorHandler->Sym = *HandleStmt->getErrorHandlerRef()->Var;
            }
        }
    }

    return Call;
}

llvm::SmallVector<SymType *, 8> SemaResolver::ResolveCallArgTypes(ASTStmt *Stmt, ASTCall *Call) {
	// Resolve Expression in Arguments
	llvm::SmallVector<SymType *, 8> CallTypes;
	for (auto Arg : Call->getArgs()) {
		ResolveExpr(Stmt, Arg->Expr);
		CallTypes.push_back(Arg->getExpr()->getTypeRef()->getType());
	}
	return CallTypes;
}

/**
 * Resolve a Reference from starting from the Stmt
 * @param Stmt
 * @param Ref
 * @return
 */
ASTRef *SemaResolver:: ResolveRef(ASTStmt *Stmt, ASTRef *Ref) {
	if (!Ref->Resolved) {

		// Resolve to the Top Parent
		ASTRef *Current = Ref;
		while (Current->getParent() != nullptr) {
			Current->getParent()->Child = Current;
			Current = Current->getParent();
		}

		// Resolve from top-bottom
		SmallVector<SymNameSpace *, 4> NameSpaces = llvm::SmallVector<SymNameSpace *, 4>{NameSpace, Default};
		return ResolveRef(Stmt, Current, NameSpaces);
	}
	return Ref;
}

/**
 * Resolve a Reference, continue to resolve until the Ref is completely resolved
 * @param Stmt
 * @param Ref
 * @param NameSpaces
 * @param ...
 * @return
 */
ASTRef *SemaResolver:: ResolveRef(ASTStmt *Stmt, ASTRef *Ref, llvm::SmallVector<SymNameSpace *, 4> &NameSpaces) {
	if (!Ref->Resolved) {

		// Ref is a Function defined in Default NameSpace?
		if (Ref->isCall()) {
			return ResolveCall(Stmt, static_cast<ASTCall *>(Ref), NameSpaces);
		}

		// Ref is a NameSpace ?
		SymNameSpace *CurrentNameSpace = S.getSymTable().getNameSpaces().lookup(Ref->getName());
		if (CurrentNameSpace) {
			// Check Import
			NameSpaces.push_back(CurrentNameSpace);
			return ResolveRef(Stmt, Ref->Child, NameSpaces);
		}

		// Ref is a Class or an Enum defined in Module or Default NameSpace?
		SymType *Type = FindType(Ref->getName(), NameSpaces);
		if (Type)
			return ResolveRef(Stmt, Type, Ref->Child);

		// Ref is a LocalVar defined in Stmt?
		SymVar *Var = nullptr;
		if (Stmt->getStmtKind() == ASTStmtKind::STMT_BLOCK) {
			ASTBlockStmt *Block = static_cast<ASTBlockStmt*>(Stmt);
			const auto &It = Block->getLocalVars().find(Ref->getName());
			if (It != Block->getLocalVars().end()) { // Search into this Block
				Var = It->getValue()->getSym();
			}
		}

		// Search in parent Stmt
		if (!Var) {
			if (Stmt->getParent()) { // search recursively into Parent Stmt to find the right Var definition
				ASTRef * SearchRef = ResolveRef(Stmt->getParent(), Ref);
				if (SearchRef->isResolved()) {
					Var = static_cast<ASTVarRef *>(SearchRef)->getVar();
				}
			}
		}

		// Search into Function Parameter list
		if (!Var) {
			llvm::SmallVector<ASTVar *, 8> Params = Stmt->getFunction()->getParams();
			for (auto &Param : Params) {
				if (Param->getName() == Ref->getName()) {
					Var = Param->getSym();
				}
			}
		}

		// Search into Global Vars
		if (!Var) {
			for (auto NS : NameSpaces) {
				Var = NS->getGlobalVars().lookup(Ref->getName());
				if (Var)
					break;
			}
		}

		if (Var) {
			Ref = S.getASTBuilder().CreateVarRef(Ref);
			static_cast<ASTVarRef *>(Ref)->Var = &Var;
			Ref->Resolved = true;

			// Add Var to LocalVars of the SymFunctionBase
			Stmt->getFunction()->getSym()->getLocalVars().push_back(Var); // Function Local var to be allocated

			if (Ref->Child)
				return ResolveRef(Stmt, Var, Ref->Child);

			return Ref;
		}
	}

	// Error: symbol not found
	S.Diag(Ref->getLocation(), diag::err_syntax_error);
	Ref->Resolved = true;
	return Ref;
}

/**
 * Resolve static Ref to a Class or Enum
 * @param Type
 * @param Ref
 * @return
 */
ASTRef *SemaResolver:: ResolveRef(ASTStmt *Stmt, SymType *Type, ASTRef *Ref) {
	if (!Ref->Resolved) {
		if (Type->isClass()) {
            SymClass *Class = static_cast<SymClass *>(Type);

			// static call
			if (Ref->isCall()) {
				ASTCall *Call = static_cast<ASTCall *>(Ref);
				SmallVector<SymType *, 8> CallTypes = ResolveCallArgTypes(Stmt, Call);
				std::string Mangled = SymFunctionBase::MangleFunction(Call->getName(), CallTypes);
				SymFunctionBase* Method = Class->getMethods().lookup(Mangled);
				if (Method) {
					Call->Function = &Method;
					Call->Resolved = true;
					if (Ref->Child)
						return ResolveRef(Stmt, Method->getReturnType(), Ref->Child);
                }
			} else { // static var
				SymVar *Var = Class->getAttributes().lookup(Ref->getName());
				if (Var && static_cast<SymClassAttribute *>(Var)->isStatic()) {
					Ref = S.getASTBuilder().CreateVarRef(Ref);
					static_cast<ASTVarRef *>(Ref)->Var = &Var;
					Ref->Resolved = true;
					if (Ref->Child)
						return ResolveRef(Stmt, Var, Ref->Child);
				}
			}
        } else if (Type->isEnum()) {

        	// Enum Entry
            SymEnum *Enum = static_cast<SymEnum *>(Type);
            SymVar *Var = Enum->getEntries().lookup(Ref->getName());
            if (Var) {
            	Ref = S.getASTBuilder().CreateVarRef(Ref);
            	static_cast<ASTVarRef *>(Ref)->Var = &Var;
            	Ref->Resolved = true;
            	if (Ref->Child)
                    return ResolveRef(Stmt, Var, Ref->Child);
            }
        }
	}
	return Ref;
}

/**
 * Resolve a Reference from parent instance
 * @param Var
 * @param Ref
 * @return
 */
ASTRef *SemaResolver:: ResolveRef(ASTStmt *Stmt, SymVar *Var, ASTRef *Ref) {
	if (!Ref->Resolved) {

		SymType *Type = Var->getType();
		if (Type->isClass()) {
			SymClass * Class = static_cast<SymClass *>(Type);
			// instance call
			if (Ref->isCall()) {
				ASTCall *Call = static_cast<ASTCall *>(Ref);
				SmallVector<SymType *, 8> CallTypes = ResolveCallArgTypes(Stmt, Call);
				std::string Mangled = SymFunctionBase::MangleFunction(Call->getName(), CallTypes);
				SymFunctionBase* Method = Class->getMethods().lookup(Mangled);
				Call->Function = &Method;
				Ref->Resolved = true;
				if (Ref->Child)
					return ResolveRef(Stmt, Method->getReturnType(), Ref->Child);
			} else {
				// instance var
				SymVar *Attribute = Class->getAttributes().lookup(Ref->getName());
				Ref = S.getASTBuilder().CreateVarRef(Ref);
				static_cast<ASTVarRef *>(Ref)->Var = &Attribute;
				Ref->Resolved = true;
				if (Ref->Child)
					return ResolveRef(Stmt, Var, Ref->Child);
			}
		} else {
			// Error: no child found from Ref
			S.Diag(Ref->getLocation(), diag::err_syntax_error);
			return nullptr;
		}
	}
	return Ref;
}

SymType * SemaResolver::FindType(llvm::StringRef Name, llvm::SmallVector<SymNameSpace *, 4> &NameSpaces) const {
	SymType *Type = nullptr;
	for (auto &NS : NameSpaces) {
		Type = NS->getTypes().lookup(Name);
		if (Type)
			return Type;
	}
	return Type;
}
