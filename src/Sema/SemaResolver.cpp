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
#include "Sym/SymTable.h"
#include "Sym/SymNameSpace.h"
#include "Sym/SymModule.h"
#include "Sym/SymClass.h"
#include "Sym/SymEnum.h"
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

#include <string>
#include <AST/ASTComment.h>
#include <llvm/Transforms/IPO/FunctionImport.h>
#include <Sema/SymBuilder.h>
#include <Sym/SymFunction.h>
#include <Sym/SymGlobalVar.h>

using namespace fly;

SemaResolver::SemaResolver(Sema &S, ASTModule *Module) :
        S(S), Module(S.getSymBuilder().CreateModule(Module)) {

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
    		Resolver->AddNameSpace();
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

void SemaResolver::AddNameSpace() {
	// Create the NameSpace if not exists yet in the Context
	std::string FullName = Module->getAST()->NameSpace->getFullName();
	SymNameSpace *NameSpace = S.getSymTable().getNameSpaces().lookup(FullName);
	if (NameSpace == nullptr) {
		SymNameSpace *Sym = S.getSymBuilder().CreateNameSpace(FullName);
		Module->NameSpace = NameSpace;
	}
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
		}
	}
}

void SemaResolver::AddImport(ASTImport *AST) {
	// Check Import
	if (S.getValidator().CheckImport(AST)) {

		if (!AST->getAlias().empty()) {

			// Check Alias
			llvm::StringRef AliasName = AST->getAlias();
			SymNameSpace *Duplicate = Module->getImports().lookup(AliasName);
			if (Duplicate) {
				S.Diag(AST->getLocation(), diag::err_conflict_import_alias) << AliasName;
				return;
			}

			// Add NameSpace to the Imports for next symbols resolution
			Module->Imports.insert(std::make_pair(AliasName, nullptr));
		}
	}
}

/**
 * ResolveModule GlobalVar Declarations
 */
void SemaResolver::AddGlobalVar(ASTVar *AST, SymComment *Comment) {
    // Check and set GlobalVar Scopes
    if (S.getValidator().CheckScopes(AST->getScopes())) {

    	// Lookup into namespace for public global var duplication
    	const llvm::StringMap<SymGlobalVar *> &GlobalVars = NameSpace->getGlobalVars();
    	if (S.getValidator().CheckDuplicateVars(GlobalVars, AST)) {
    		// Add into NameSpace for next resolve
    		SymGlobalVar *GlobalVar = S.getSymBuilder().CreateGlobalVar(Module, AST);
    		GlobalVar->Comment = Comment;
    	}
    }
}

/**
 * ResolveModule Function Declarations
 */
void SemaResolver::AddFunction(ASTFunction *AST, SymComment *Comment) {
    // Check Scopes
    if (S.getValidator().CheckScopes(AST->getScopes())) {
	    // Lookup into namespace for public global var duplication
    	const SmallVector<SymFunction *, 8> &Functions = NameSpace->getFunctions();
    	if (S.getValidator().CheckDuplicateFunctions(Functions, AST)) {
    		// Add into NameSpace for next resolve
    		SymFunction *Function = S.getSymBuilder().CreateFunction(Module, AST);
    		Function->Comment = Comment;
    	}
    }
}

/**
 * ResolveModule Identity Declarations
 */
void SemaResolver::AddClass(ASTClass *AST, SymComment *Comment) {
    // Check and set GlobalVar Scopes
    if (S.getValidator().CheckClass(AST) && S.getValidator().CheckScopes(AST->getScopes())) {
	    // Lookup into namespace for public Identity var duplication
    	if (S.getValidator().CheckDuplicateIdentities(Identities, AST)) {
    		S.getSymBuilder().CreateClass(Module, AST);
    	}
    }
}

void SemaResolver::AddEnum(ASTEnum *Enum, SymComment *Comment) {
	if (S.getValidator().CheckEnum(Enum)) {
		SymEnum *Enum = S.getSymBuilder().CreateEnum(Module, Enum);
		Enum->Comment = Comment;
	}
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
		S.getValidator().CheckCommentParams(Function->getParams());
		S.getValidator().CheckCommentReturn(Function->getReturnType());
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
		ResolveType(AST->getTypeRef());
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
		ResolveType(AST->getReturnType());

		// Resolve Parameters Types
		for (auto Param : AST->getParams()) {
			// Check duplicated params
			// TODO
			//S.getValidator().CheckDuplicateParams(Function->Params, Param);

			// resolve parameter type
			ResolveType(Param->getTypeRef());
		}

		// Resolve Function Body
		ResolveStmtBlock(AST->Body);

	}
}

/**
 * Resolve Module Identity Definitions
 */
void SemaResolver::ResolveClasses() {
	for (auto Entry : Module->getClasses()) {
		SymClass *Sym = Entry.getValue();
		ASTClass *AST = Sym->getAST();

		if (Sym->Comment) {
			ResolveComment(Sym->Comment, AST);
		}

		// Class and Structures
		// Attributes
		for (auto &Attribute: Class->Attributes) {
			IdentitySymbols->Attributes.insert(std::make_pair(Attribute->getName(), Attribute));
		}

		// Constructors
		for (auto &Constructor: Class->Constructors) {
			SemaSpaceSymbols::InsertFunction(IdentitySymbols->Methods, Constructor);
		}

		// Resolve Super Classes
		if (!AST->SuperClasses.empty()) {
			llvm::StringMap<std::map<uint64_t, llvm::SmallVector<ASTFunction *, 4>>> SuperMethods;
			llvm::StringMap<std::map<uint64_t, llvm::SmallVector<ASTFunction *, 4>>> ISuperMethods;
			for (ASTRefType *SuperClassType: AST->SuperClasses) {
				ResolveType(SuperClassType);
				ASTClass *SuperClass = static_cast<ASTClass *>(SuperClassType->getDef());

				// Struct: Resolve Var in Super Classes
				if (SuperClass->getClassKind() == ASTClassKind::STRUCT) {

					// Interface cannot extend a Struct
					if (AST->getClassKind() == ASTClassKind::INTERFACE) {
						S.Diag(SuperClassType->getLocation(), diag::err_sema_interface_ext_struct);
						return;
					}

					// Add Vars to the Struct
					for (auto &SuperAttribute: SuperClass->getAttributes()) {

						// Check Var already exists and type conflicts in Super Vars
						for (auto &Attribute : AST->Attributes) {
							if (Attribute->getName() == SuperAttribute->getName()) {
								S.Diag(Attribute->getLocation(), diag::err_sema_super_struct_var_conflict);
								return;
							}
							AST->Attributes.push_back(SuperAttribute);
						}
					}
				}

				// Interface cannot extend a Class
				if (AST->getClassKind() == ASTClassKind::INTERFACE &&
					SuperClass->getClassKind() == ASTClassKind::CLASS) {
					S.Diag(SuperClassType->getLocation(), diag::err_sema_interface_ext_class);
					return;
					}

				// Class/Interface: take all Super Classes methods
				if (SuperClass->getClassKind() == ASTClassKind::CLASS ||
					SuperClass->getClassKind() == ASTClassKind::INTERFACE) {

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

			// FIXME
			// Check if all abstract methods are implemented
			//                    for (const auto &EntryMap: ISuperMethods) {
			//                        const auto &Map = EntryMap.getValue();
			//                        auto MapIt = Map.begin();
			//                        while (MapIt != Map.end()) {
			//                            for (ASTFunction *ISuperMethod: MapIt->second) {
			//                                if (!S.Builder->ContainsFunction(Class->Methods, ISuperMethod)) {
			//                                    S.Diag(ISuperMethod->getLocation(),
			//                                           diag::err_sema_method_not_implemented);
			//                                    return;
			//                                }
			//                            }
			//                            MapIt++;
			//                        }
			//                    }
		}


		// Set default values in attributes
		if (AST->getKind() == ASTKind::AST_INTERFACE || AST->getKind() == ASTKind::AST_STRUCT) {

			// Init null value attributes with default values
			for (auto &Attribute : AST->Attributes) {
				// Generate default values
				if (Attribute->getExpr() == nullptr) {
					ASTValue *DefaultValue = S.Builder->CreateDefaultValue(Attribute->getType());
					ASTValueExpr *ValueExpr = S.Builder->CreateExpr(DefaultValue);
					Attribute->setExpr(ValueExpr);
				}
				S.getValidator().CheckValueExpr(Attribute->getExpr());
				Attribute->getExpr()->Type = Attribute->getType(); // Maintain expr type
			}
		}

		// Create default constructor if there aren't any other constructors
		// FIXME this code remove default constructor
		//                if (!Class->Constructors.empty()) {
		//                    delete Class->DefaultConstructor;
		//                }

		// Constructors
		for (auto Constructor: AST->Constructors) {

			// Resolve Attribute types
			for (auto &Attribute: AST->Attributes) {
				// TODO
			}
			ResolveStmtBlock(Constructor->Body);
		}

		// Methods
		for (auto Method : AST->Methods) {

			// Add Class vars for each Method
			for (auto &Attribute: AST->Attributes) {

				// Check if Method already contains this var name as LocalVar
				if (!S.getValidator().CheckDuplicateLocalVars(Method->Body, Attribute->getName())) {
					return;
				}
			}

			if (!Method->isAbstract()) {
				ResolveStmtBlock(Method->Body); // FIXME check if already resolved
			}
		}
	}
}

void SemaResolver::ResolveEnums() {
	for (auto Entry : Module->getEnums()) {
		SymEnum *Sym = Entry.getValue();
		ASTEnum *AST = Sym->getAST();

		if (Sym->Comment) {
			ResolveComment(Sym->Comment, AST);
		}

		for (auto Def : AST->getDefinitions()) {
			switch (Def->getKind()) {
			case ASTKind::AST_VAR: {
				AddEnum(static_cast<ASTEnum *>(AST), Comment);
			} break;
			case ASTKind::AST_COMMENT:
				Comment = S.getSymBuilder().CreateComment(static_cast<ASTComment *>(AST));
				break;
			}
			Sym->Entries.insert(std::make_pair(Entry->getName(), Entry));
		}
	}
}

bool SemaResolver::ResolveType(ASTType *Type) {
    if (Type->isIdentity()) {
        return ResolveIdentityType(static_cast<ASTRefType *>(Type));
    } else if (Type->isArray()) {
        ASTArrayType *ArrayType = static_cast<ASTArrayType *>(Type);
        return ResolveType(ArrayType->getType());
    }
    // TODO Error: Type not resolved
    return false;
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
        	ASTType * ReturnType = ReturnStmt->Parent->getFunction()->getReturnType(); // Force Return Expr to be of Return Type
			bool Success = true;
        	if (ReturnStmt->Expr != nullptr) {
            	Success = ResolveExpr(ReturnStmt->Parent, ReturnStmt->Expr, ReturnType) &&
        		S.getValidator().CheckConvertibleTypes(ReturnStmt->Expr->getTypeRef(), ReturnStmt->Parent->getFunction()->getReturnType());
            } else {
            	if (!ReturnStmt->Parent->getFunction()->getReturnType()->isVoid()) {
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
        ResolveType(LocalVar.getValue()->getType());
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
    ASTBoolType * IfBoolType = S.Builder->CreateBoolType(IfStmt->Rule->getLocation());
    bool Success = ResolveExpr(IfStmt->getParent(), IfStmt->Rule, IfBoolType) &&
                   ResolveStmt(IfStmt->Stmt);
    for (ASTRuleStmt *Elsif : IfStmt->Elsif) {
        ASTBoolType * ElsifBoolType = S.Builder->CreateBoolType(Elsif->Rule->getLocation());
        Success &= ResolveExpr(IfStmt->getParent(), Elsif->Rule, ElsifBoolType) &&
                   ResolveStmt(Elsif->Stmt);
    }
    if (Success && IfStmt->Else) {
        Success = ResolveStmt(IfStmt->Else);
    }
    return Success;
}

bool SemaResolver::ResolveStmtSwitch(ASTSwitchStmt *SwitchStmt) {
    assert(SwitchStmt && "Switch Block cannot be null");

    bool Success = ResolveVarRef(SwitchStmt->getParent(), SwitchStmt->getVarRef()) &&
                   S.getValidator().CheckEqualTypes(SwitchStmt->getVarRef()->getDef()->getType(), ASTValueKind::TYPE_INTEGER);
    for (ASTRuleStmt *Case : SwitchStmt->Cases) {
    	Success &= ResolveExpr(SwitchStmt, Case->getRule());
        Success &= S.getValidator().CheckEqualTypes(Case->getRule()->getTypeRef(), ASTValueKind::TYPE_INTEGER) &&
                ResolveStmt(Case);
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
	ASTBoolType * ConditionBoolType = S.Builder->CreateBoolType(LoopStmt->Rule->getLocation());
    if (LoopStmt->Init) {
        LoopStmt->Stmt->Parent = LoopStmt->Init;
        Success = ResolveStmt(LoopStmt->Init) &&
        	ResolveExpr(LoopStmt->Init, LoopStmt->Rule, ConditionBoolType);
    } else {
        Success = ResolveExpr(LoopStmt->Parent, LoopStmt->Rule, ConditionBoolType);
    }
    Success = S.getValidator().CheckConvertibleTypes(LoopStmt->Rule->Type, S.Builder->CreateBoolType(SourceLocation()));
    Success = ResolveStmt(LoopStmt->Stmt);
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

bool SemaResolver::ResolveIdentityType(ASTRefType *IdentityType) {
    assert(IdentityType && "IdentityType cannot be null");

    // Resolve only if not resolved yet
    if (!IdentityType->isResolved()) {

        // Identity can have only NameSpace as Parent: NameSpace.Type
        ASTIdentity *Identity;
        if (IdentityType->getParent()) {

            // Resolve by Imports
        	ASTIdentifier *Parent = nullptr;
        	SymNameSpace *IdentityNameSpace = FindNameSpace(IdentityType, Parent);
            Identity = FindIdentity(IdentityType->getName(), IdentityNameSpace);
        } else {
            // Resolve in current NameSpace
            Identity = FindIdentity(IdentityType->getName(), NameSpace);

            if (Identity == nullptr)
                // Resolve in Default NameSpace
                Identity = FindIdentity(IdentityType->getName(), S.getSymTable().getDefaultNameSpace());
        }

        // Take Identity from NameSpace
        IdentityType->Resolved = true; // Evict Cycle Loop: can be resolved only now

        if (Identity) {
            IdentityType->Def = Identity;
            IdentityType->IdentityTypeKind = Identity->getType()->getIdentityTypeKind();
        } else {
            S.Diag(IdentityType->getLocation(), diag::err_sema_type_notfound);
        }
    }

    if (!IdentityType->Def) {
        S.Diag(IdentityType->getLocation(), diag::err_unref_type);
        return false;
    }

    return IdentityType->Resolved;
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
                ASTType *Type = Call->getDef()->getReturnType();
                if (Type->isIdentity()) {
                    if (ResolveIdentityType(static_cast<ASTRefType *>(Type)))
                        // Can be only a Call or a Var
                        return ResolveIdentifier(static_cast<ASTRefType *>(Type)->getDef(), Stmt, Call->getChild());
                } else {
                    // Error: cannot access to not identity var
                    // TODO
                }
            }
        } else {
            // Identity is Type
            // NameSpace.Type...Call() or NameSpace.Type...Var
            ASTIdentity *Identity = FindIdentity(Identifier->getName(), NameSpace);
            if (Identity) {
                if (Identity->getIdentityKind() == ASTIdentityKind::ID_CLASS)
                    Identifier = S.Builder->CreateClassType(Identifier);
                else if (Identity->getIdentityKind() == ASTIdentityKind::ID_ENUM)
                    Identifier = S.Builder->CreateEnumType(Identifier);
                static_cast<ASTRefType *>(Identifier)->Def = Identity;
                Identifier->Resolved = true;

                // Resolve Child
                if (Identifier->getChild()) {
                    if (Identifier->getChild()->isCall()) {
                        return ResolveStaticCall(Identity, Stmt, static_cast<ASTCall *>(Identifier->getChild()));
                    } else {
                        return ResolveStaticVarRef(Identity, Stmt, static_cast<ASTVarRef *>(Identifier->getChild()));
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

bool SemaResolver::ResolveIdentifier(ASTStmt *Stmt, ASTIdentifier *Identifier) {
    assert(Stmt && "Stmt cannot be null");
    assert(Identifier && "Identifier cannot be null");

    if (Identifier->isResolved() == false) {
        if (Identifier->isCall()) { // Call cannot be undefined
            ASTCall *Call = static_cast<ASTCall*>(Identifier);

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
                ASTType *Type = Call->getDef()->getReturnType();
                if (!Type->isIdentity()) {
                    // Error: cannot access to not identity var
                }
                if (ResolveIdentityType(static_cast<ASTRefType*>(Type)))
                    // Can be only a Call or a Var
                    return ResolveIdentifier(static_cast<ASTRefType*>(Type)->getDef(), Stmt, Call->getChild());
            }

        } else if (Identifier->isVarRef()) {
            auto *VarRef = static_cast<ASTVarRef*>(Identifier);

            // Search in LocalVars
            // LocalVar.Var or ClassAttribute.Var
            ASTVar *Var = FindLocalVar(Stmt, Identifier->getName());

            // Check if Function is a class Method
            if (Var == nullptr) {
                ASTFunction *Function = Stmt->getFunction();

                // Search for Class Vars if Var is Class Method
                if (Function->getKind() == ASTFunctionKind::CLASS_METHOD) {
                    for (auto &Attribute: static_cast<ASTFunction*>(Function)->getClass()->Attributes) {
                        if (Attribute->getName() == Identifier->getName()) {
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
                    ASTRefType *IdentityType = static_cast<ASTRefType*>(Var->getTypeRef());
                    return ResolveIdentifier(IdentityType->getDef(), Stmt, VarRef->getChild());
                }
            }
        } else {
            ASTRefType *IdentityType = S.Builder->CreateTypeRef(Identifier);
            if (ResolveIdentityType(IdentityType) && IdentityType->getChild()) {
                return ResolveIdentifier(IdentityType->getDef(), Stmt, IdentityType->getChild());
            }
        }
    }

    return Identifier->Resolved;
}

bool SemaResolver::ResolveGlobalVarRef(SymNameSpace *NameSpace, ASTStmt *Stmt, ASTVarRef *VarRef) {
    assert(NameSpace && "NameSpace cannot be null");
    assert(VarRef && "VarRef cannot be null");

    if (VarRef->isResolved() == false) {
        ASTVar *GlobalVar = FindGlobalVar(VarRef->getName(), NameSpace);
        if (GlobalVar) {
            VarRef->Def = GlobalVar;
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
bool SemaResolver::ResolveExpr(ASTStmt *Stmt, ASTExpr *Expr, ASTType *Type) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveExpr", Logger()
            .Attr("Expr", Expr)
            .Attr("Type", Expr).End());

    bool Success = false;
    switch (Expr->getExprKind()) {
        case ASTExprKind::EXPR_VALUE: {
            // Select the best option for this Value
            ASTValueExpr *ValueExpr = static_cast<ASTValueExpr*>(Expr);
            if (Type != nullptr)
                ValueExpr->Type = Type;
            return S.getValidator().CheckValue(ValueExpr->getValue());
        }
        case ASTExprKind::EXPR_VAR_REF: {
            ASTVarRef *VarRef = static_cast<ASTVarRefExpr*>(Expr)->getVarRef();
            if (ResolveVarRef(Stmt, VarRef)) {
                Expr->Type = VarRef->getDef()->getType();
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
                        Expr->Type = Call->Def->ReturnType;
                        break;
                    case ASTCallKind::CALL_NEW: {
                        ASTFunction *Def = static_cast<ASTFunction*>(Call->Def);
                        assert(Def && "Undefined Call");
                        Expr->Type = Def->getClass()->getType();
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
                    Expr->Type = Unary->Expr->Type;
                    break;
                }
                case ASTOpExprKind::OP_BINARY: {
                    ASTBinaryOpExpr *Binary = static_cast<ASTBinaryOpExpr*>(Expr);

                    Success = ResolveExpr(Stmt, Binary->LeftExpr) && ResolveExpr(Stmt, Binary->RightExpr);
                    if (Success) {
                        if (Binary->getTypeKind() == ASTBinaryOpTypeExprKind::OP_BINARY_ARITH ||
                                Binary->getTypeKind() == ASTBinaryOpTypeExprKind::OP_BINARY_COMPARISON) {

                        	// Check Compatible Types Bool/Bool, Float/Float, Integer/Integer
                            Success = S.getValidator().CheckArithTypes(Binary->getLocation(), Binary->LeftExpr->Type,
                                                                  Binary->RightExpr->Type);

                            if (Success) {
                            	// Set respectively the Left or Right Expr Type by chose the Expr which is not a Value Type
                            	// Ex.
                            	// int a = 0
                            	// int b = a + 1
                            	// 1 will have type int
                            	if (Binary->LeftExpr->getExprKind() == ASTExprKind::EXPR_VALUE &&
									Binary->RightExpr->getExprKind() != ASTExprKind::EXPR_VALUE) {
                            		Binary->LeftExpr->Type = Binary->RightExpr->Type;
								} else if (Binary->RightExpr->getExprKind() == ASTExprKind::EXPR_VALUE &&
                            		Binary->LeftExpr->getExprKind() != ASTExprKind::EXPR_VALUE) {
                            		Binary->RightExpr->Type = Binary->LeftExpr->Type;
                            	}

                                // Promotes First or Second Expr Types in order to be equal
                                if (Binary->LeftExpr->Type->isInteger()) {
                                    if (static_cast<ASTIntegerType*>(Binary->LeftExpr->Type)->getSize() > static_cast<ASTIntegerType*>(Binary->RightExpr->Type)->getSize())
                                        Binary->Type = Binary->LeftExpr->Type;
                                    else
                                        Binary->Type = Binary->RightExpr->Type;
                                } else if (Binary->LeftExpr->Type->isFloatingPoint()) {
                                    if (static_cast<ASTFloatingPointType*>(Binary->LeftExpr->Type)->getSize() > static_cast<ASTFloatingPointType*>(Binary->RightExpr->Type)->getSize())
                                		Binary->Type = Binary->LeftExpr->Type;
                                    else
                                		Binary->Type = Binary->RightExpr->Type;
                                }

                                Binary->Type = Binary->getTypeKind() == ASTBinaryOpTypeExprKind::OP_BINARY_ARITH ?
                                               Binary->LeftExpr->Type : S.Builder->CreateBoolType(Binary->getLocation());
                            }
                        } else if (Binary->getTypeKind() == ASTBinaryOpTypeExprKind::OP_BINARY_LOGIC) {
                            Success = S.getValidator().CheckLogicalTypes(Binary->getLocation(),
                                                                     Binary->LeftExpr->Type, Binary->RightExpr->Type);
                            Binary->Type = S.Builder->CreateBoolType(Binary->getLocation());
                        }
                    }
                    break;
                }
                case ASTOpExprKind::OP_TERNARY: {
                    ASTTernaryOpExpr *Ternary = static_cast<ASTTernaryOpExpr*>(Expr);
                    Success = ResolveExpr(Stmt, Ternary->ConditionExpr) &&
                              S.getValidator().CheckConvertibleTypes(Ternary->ConditionExpr->Type, S.Builder->CreateBoolType(SourceLocation())) &&
                              ResolveExpr(Stmt, Ternary->TrueExpr) &&
                              ResolveExpr(Stmt, Ternary->FalseExpr);
                    Ternary->Type = Ternary->TrueExpr->Type; // The group type is equals to the second type
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

SymGlobalVar *SemaResolver::FindGlobalVar(llvm::StringRef Name, SymNameSpace *NameSpace) const {
    FLY_DEBUG_MESSAGE("Sema", "FindGlobalVar", Logger().Attr("Name", Name).End());
    return NameSpace->getGlobalVars().lookup(Name);
}

SymIdentity *SemaResolver::FindIdentity(llvm::StringRef Name, SymNameSpace *NameSpace) const {
    FLY_DEBUG_MESSAGE("Sema", "FindIdentity", Logger().Attr("Name", Name).End());
    return NameSpace->getIdentities().lookup(Name);
}

SymFunction *SemaResolver::FindFunction(ASTCall *Call, SymNameSpace *NameSpace) const {
    FLY_DEBUG_MESSAGE("Sema", "FindFunction", Logger().Attr("Call", (ASTIdentifier *) Call).End());
    return FindFunction(Call, NameSpace->getFunctions());
}

ASTFunction *SemaResolver::FindClassMethod(ASTCall *Call, ASTClass *Class) const {
    FLY_DEBUG_MESSAGE("Sema", "FindClassMethod", Logger().Attr("Call", (ASTIdentifier *) Call).End());
    return FindFunction(Call, Class->Methods);
}

template <typename T>
T *SemaResolver::FindFunction(ASTCall *Call, llvm::SmallVector<T *, 8> Functions) const {
	// TODO
}

template <typename T>
T *SemaResolver::FindFunction(ASTCall *Call, llvm::StringMap<std::map<uint64_t, llvm::SmallVector<T *, 4>>> Functions) const {
    for (auto &StrMapIt : Functions) {
        llvm::StringRef FunctionName = StrMapIt.getKey();
        if (FunctionName == Call->getName()) {
            const std::map<uint64_t, llvm::SmallVector<T *, 4>> &IntMap = StrMapIt.getValue();
            const auto &IntMapIt = IntMap.find(Call->getArgs().size());
            if (IntMapIt != IntMap.end()) {
                for (T *Function: IntMapIt->second) {
                    if (Function->getParams().size() == Call->getArgs().size()) {

                        bool Success = true; // if Params = Args = 0 skip for cycle
                        for (unsigned long i = 0; i < Function->getParams().size(); i++) {
                            // Resolve Arg Expr on first
                            ASTArg *Arg = Call->getArgs()[i];
                            ASTVar *Param = Function->getParams()[i];
                            Success &= S.getValidator().CheckConvertibleTypes(Arg->getExpr()->getTypeRef(), Param->getTypeRef());
                        }

                        if (Success)
                            return Function;
                    }
                }
            }
        }
    }
    return nullptr;
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
