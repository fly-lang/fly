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
#include "Sema/SemaBuilder.h"
#include "Sema/SemaSpaceSymbols.h"
#include "Sema/SemaIdentitySymbols.h"
#include "AST/ASTContext.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTClassMethod.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassAttribute.h"
#include "AST/ASTEnum.h"
#include "AST/ASTEnumType.h"
#include "AST/ASTEnumEntry.h"
#include "AST/ASTType.h"
#include "AST/ASTModule.h"
#include "AST/ASTArg.h"
#include "AST/ASTIfStmt.h"
#include "AST/ASTImport.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTParam.h"
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

using namespace fly;

SemaResolver::SemaResolver(Sema &S, ASTModule *Module, SemaSpaceSymbols *SpaceSymbols) :
        S(S), Module(Module), MySpaceSymbols(SpaceSymbols) {

}

/**
 * Resolve Modules by creating the right structure for resolving all symbols
 */
bool SemaResolver::Resolve(Sema &S) {
    llvm::SmallVector<SemaResolver *, 8> Resolvers;

    // First: Resolve Modules for populate all NameSpace MapSymbols
    for (auto &Module : S.Context->getModules()) {

        // Check Duplicate Module Names
        for (auto M : S.Context->Modules) {
            if (M->getId() != Module->getId() && M->getName() == Module->getName()) {
                S.Diag(diag::err_sema_module_duplicated) << M->getName();
                return false;
            }
        }

        // If no NameSpace is assigned, set Default NameSpace
        if (Module->getNameSpaces().empty())
            Module->NameSpaces.push_back(new ASTNameSpace(SourceLocation(), ASTContext::DEFAULT_NAMESPACE));

        // Error: Multiple NameSpaces declared
        if (Module->getNameSpaces().size() > 1)
            S.Diag(diag::err_sema_multiple_module_namespaces) << Module->getName();

        // Use NameSpace to create MapSymbols
        ASTNameSpace *NameSpace = Module->getNameSpace();
        SemaSpaceSymbols *Symbols = S.MapSymbols.lookup(NameSpace->getName());
        if (Symbols == nullptr) {
            Symbols = new SemaSpaceSymbols(S);
            Symbols->NameSpace = NameSpace->getName();
            S.MapSymbols.insert(std::make_pair(NameSpace->getName(), Symbols));
        }

        // Add Module in the Symbol Modules: you can retrieve all Modules from a NameSpace
        Symbols->Modules.push_back(Module);

        // Resolve Declarations
        SemaResolver *Resolver = new SemaResolver(S, Module, Symbols);
        Resolver->ResolveGlobalVarDeclarations(); // resolve Global Vars
        Resolver->ResolveFunctionDeclarations();  // resolve ASTBlock of Body Functions
        Resolver->ResolveIdentityDeclarations();  // resolve Identity attributes and methods

        // add to Resolvers list
        Resolvers.push_back(Resolver);
    }

    // Second: Resolve Definitions
    for (auto &Resolver : Resolvers) {
    	Resolver->ResolveNameSpaceDefinition();
    	Resolver->ResolveImportDefinitions();
    	Resolver->ResolveDefinitions();
    }

    return !S.Diags.hasErrorOccurred();
}

/**
 * ResolveModule GlobalVar Declarations
 */
void SemaResolver::ResolveGlobalVarDeclarations() {
    for (auto GlobalVar : Module->getGlobalVars()) {

        // Check and set GlobalVar Scopes
        S.Validator->CheckScopes(GlobalVar->getScopes());
        for (auto Scope : GlobalVar->getScopes()) {
            switch (Scope->getKind()) {

                case ASTScopeKind::SCOPE_VISIBILITY:
                    GlobalVar->Visibility = Scope->Visibility;
                    break;
                case ASTScopeKind::SCOPE_CONSTANT:
                    GlobalVar->Constant = Scope->Constant;
                    break;

                default:
                    S.Diag(GlobalVar->getLocation(), diag::err_sema_invalid_gvar_scope);
            }
        }

        // Lookup into namespace for public global var duplication
        ASTGlobalVar *DuplicateVar = MySpaceSymbols->getGlobalVars().lookup(GlobalVar->getName());

        if (DuplicateVar) { // This NameSpace already contains this GlobalVar
            S.Diag(DuplicateVar->getLocation(), diag::err_duplicate_gvar) << DuplicateVar->getName();
            return;
        }

        // Add into NameSpace for next resolve
        MySpaceSymbols->GlobalVars.insert(std::make_pair(GlobalVar->getName(), GlobalVar));
    }
}

/**
 * ResolveModule Function Declarations
 */
void SemaResolver::ResolveFunctionDeclarations() {
    for (auto &Function : Module->Functions) {

        // Check Scopes
        S.Validator->CheckScopes(Function->getScopes());
        for (auto Scope : Function->getScopes()) {
            switch (Scope->getKind()) {

                case ASTScopeKind::SCOPE_VISIBILITY:
                    Function->Visibility = Scope->Visibility;
                    break;

                default:
                    S.Diag(Function->getLocation(), diag::err_sema_invalid_func_scope);
            }
        }

        // Add into NameSpace for next resolve
        SemaSpaceSymbols::InsertFunction(MySpaceSymbols->Functions, Function);
    }
}

/**
 * ResolveModule Identity Declarations
 */
void SemaResolver::ResolveIdentityDeclarations() {
    for (auto &Identity : Module->Identities) {

        // Check and set GlobalVar Scopes
        S.Validator->CheckScopes(Identity->getScopes());
        for (auto Scope : Identity->getScopes()) {
            switch (Scope->getKind()) {

                case ASTScopeKind::SCOPE_VISIBILITY:
                    Identity->Visibility = Scope->Visibility;
                    break;

                default:
                    S.Diag(Identity->getLocation(), diag::err_sema_invalid_identity_scope);
            }
        }

        // Lookup into namespace for public global var duplication
        SemaIdentitySymbols *DuplicateIdentity = MySpaceSymbols->getIdentities().lookup(Identity->getName());
        if (DuplicateIdentity) { // This NameSpace already contains this Identity
            S.Diag(Identity->getLocation(), diag::err_duplicate_identity) << Identity->getName();
            return;
        }

        // Add into NameSpace for next resolve
        SemaIdentitySymbols *IdentitySymbols = new SemaIdentitySymbols(Identity);
        MySpaceSymbols->Identities.insert(std::make_pair(Identity->getName(), IdentitySymbols));
        Identity->Type->IdentitySymbols = IdentitySymbols;

        // Resolve Attributes and Methods
        if (Identity->getIdentityKind() == ASTIdentityKind::ID_CLASS) {
            ASTClass *Class = static_cast<ASTClass *>(Identity);
            if (Class->getClassKind() == ASTClassKind::CLASS || Class->getClassKind() == ASTClassKind::STRUCT) {

                for (auto &Attribute: Class->Attributes) {
                    IdentitySymbols->Attributes.insert(std::make_pair(Attribute->getName(), Attribute));
                }

                for (auto &Constructor: Class->Constructors) {
                    SemaSpaceSymbols::InsertFunction(IdentitySymbols->Methods, Constructor);
                }

                for (auto &Method: Class->Methods) {
                    SemaSpaceSymbols::InsertFunction(IdentitySymbols->Methods, Method);
                }
            }
        } else if (Identity->getIdentityKind() == ASTIdentityKind::ID_ENUM) {
            ASTEnum *Enum = static_cast<ASTEnum *>(Identity);
            for (auto &Entry: Enum->Entries) {
                IdentitySymbols->Entries.insert(std::make_pair(Entry->getName(), Entry));
            }
        }
    }
}

void SemaResolver::ResolveDefinitions() {
	unsigned i = 0;
	for (ASTBase *Def: Module->Definitions) {

		// Set Comment if the previous AST is a Comment
		if (i > 0 && Def->getKind() != ASTKind::AST_COMMENT) { // Cannot set a comment of a comment
			ASTBase *ProbablyComment = Module->Definitions[i-1];
			if (ProbablyComment->getKind() == ASTKind::AST_COMMENT) {
				Def->Comment = static_cast<ASTComment *>(ProbablyComment);
			}
		}

		switch (Def->getKind()) {
			case ASTKind::AST_VAR:
				ResolveGlobalVarDefinition(static_cast<ASTGlobalVar *>(Def));
				break;
			case ASTKind::AST_FUNCTION:
				ResolveFunctionDefinition(static_cast<ASTFunction *>(Def));
				break;
			case ASTKind::AST_IDENTITY: {
				ASTIdentity *Identity = static_cast<ASTIdentity *>(Def);
				switch (Identity->getIdentityKind()) {
				case ASTIdentityKind::ID_CLASS:
					ResolveClassDefinition(static_cast<ASTClass *>(Def));
					break;
				case ASTIdentityKind::ID_ENUM:
					ResolveEnumDefinition(static_cast<ASTEnum *>(Def));
					break;
				}
			} break;
			case ASTKind::AST_COMMENT: {
				ASTBase *NextDef = Module->Definitions.size() > i ? Module->Definitions[i+1] : nullptr;
				ResolveCommentDefinition(static_cast<ASTComment *>(Def), NextDef);
			} break;
		}

		i++;
	}
}

void SemaResolver::ResolveCommentDefinition(ASTComment *Comment, ASTBase *NextDef) {
	switch (NextDef->getKind()) {

	case ASTKind::AST_VAR:
		NextDef->Comment = Comment;
		break;
	case ASTKind::AST_FUNCTION: {
		ASTFunction *Function = static_cast<ASTFunction *>(NextDef);
		S.Validator->CheckCommentParams(Function->getParams());
		S.Validator->CheckCommentReturn(Function->getReturnType());
		NextDef->Comment = Comment;
	} break;
	case ASTKind::AST_IDENTITY: {
		// Set Class Comment
		ASTIdentity *Identity = static_cast<ASTIdentity *>(NextDef);
		Identity->Comment = Comment;

		// Set Class Attributes and Methods Comment
		unsigned i = 0;
		for (auto ClassDef: Identity->Definitions) {

			// Set Comment in Attributes and Methods
			if (ClassDef->getKind() == ASTKind::AST_COMMENT) {

				// Set the Next Definition if exists
				ASTBase *NextAST = Identity->Definitions.size() > i ? Identity->Definitions[i+1] : nullptr;
				if (NextAST) {
					if (NextAST->getKind() == ASTKind::AST_VAR) { // Attribute
						NextAST->Comment = Comment;

					} else if (NextAST->getKind() == ASTKind::AST_FUNCTION) { // Method
						ASTClassMethod *Method = static_cast<ASTClassMethod *>(NextAST);
						S.Validator->CheckCommentParams(Method->getParams());
						S.Validator->CheckCommentReturn(Method->getReturnType());
						NextAST->Comment = Comment;
					}
				}
			}

			i++;
		}
	} break;
	case ASTKind::AST_COMMENT:
		// Do nothing
		break;
	}
}

void SemaResolver::ResolveNameSpaceDefinition() {
	return;
}

/**
 * ResolveModule Import Definitions
 */
void SemaResolver::ResolveImportDefinitions() {

	for (ASTImport *Import : Module->Imports) {
		// Search Namespace of the Import
		SemaSpaceSymbols *ImportNameSpace = S.MapSymbols.lookup(Import->getName());

		if (!ImportNameSpace) {
			// Error: NameSpace not found
			S.Diag(Import->getLocation(), diag::err_namespace_notfound) << Import->getName();
			return;
		}

		// Check import
		S.Validator->CheckImport(Module, Import);

		// Check import duplications
		llvm::StringRef ImportName = Import->getName();
		auto Duplicate = ImportSymbols.lookup(Import->getName());
		if (Duplicate) {
			S.Diag(Import->getLocation(), diag::err_conflict_import) << Import->getName();
			return;
		}

		if (Import->getAlias()) {

			// Check Alias
			llvm::StringRef AliasName = Import->getAlias()->getName();
			Duplicate = ImportSymbols.lookup(AliasName);
			if (Duplicate) {
				S.Diag(Import->getLocation(), diag::err_conflict_import_alias) << AliasName;
				return;
			}

			// Add NameSpace Symbols with Alias name
			AddImportSymbols(AliasName);
		} else {

			// Add NameSpace Symbols Import name
			AddImportSymbols(ImportName);
		}
	}
}

/**
 * Resolve Module GlobalVar Definitions
 */
void SemaResolver::ResolveGlobalVarDefinition(ASTGlobalVar *GlobalVar) {
    // Check Expr Value
    if (GlobalVar->Expr && GlobalVar->Expr->getExprKind() != ASTExprKind::EXPR_VALUE) {
        S.Diag(GlobalVar->Expr->getLocation(), diag::err_invalid_gvar_value);
    }

    // Resolve Type
    ResolveType(GlobalVar->getType());
}

/**
 * Resolve Module Function Definitions
 */
void SemaResolver::ResolveFunctionDefinition(ASTFunction *Function) {
	// Resolve Return Type
	ResolveType(Function->getReturnType());

	// Resolve Parameters Types
	for (auto Param : Function->getParams()) {
		// Check duplicated params
		// TODO
		//S.Validator->CheckDuplicateParams(Function->Params, Param);

		// resolve parameter type
		ResolveType(Param->getType());
	}

	// Resolve Function Body
	ResolveStmtBlock(Function->Body);
}

void SemaResolver::ResolveEnumDefinition(ASTEnum *Enum) {
	for (auto &Entry : Enum->Entries) {
		// TODO check Entry value
		// S.Validator->CheckValueExpr(Entry->getExpr());
	}
}

/**
 * Resolve Module Identity Definitions
 */
void SemaResolver::ResolveClassDefinition(ASTClass *Class) {
    SemaIdentitySymbols *IdentitySymbols = Class->getType()->IdentitySymbols;

    if (Class->getIdentityKind() == ASTIdentityKind::ID_CLASS) {
        // Resolve Super Classes
        if (!Class->SuperClasses.empty()) {
            llvm::StringMap<std::map<uint64_t, llvm::SmallVector<ASTClassMethod *, 4>>> SuperMethods;
            llvm::StringMap<std::map<uint64_t, llvm::SmallVector<ASTClassMethod *, 4>>> ISuperMethods;
            for (ASTIdentityType *SuperClassType: Class->SuperClasses) {
                ResolveType(SuperClassType);
                ASTClass *SuperClass = static_cast<ASTClass *>(SuperClassType->getDef());

                // Struct: Resolve Var in Super Classes
                if (SuperClass->getClassKind() == ASTClassKind::STRUCT) {

                    // Interface cannot extend a Struct
                    if (Class->getClassKind() == ASTClassKind::INTERFACE) {
                        S.Diag(SuperClassType->getLocation(), diag::err_sema_interface_ext_struct);
                        return;
                    }

                    // Add Vars to the Struct
                    for (auto &SuperAttribute: SuperClass->getAttributes()) {

                        // Check Var already exists and type conflicts in Super Vars
                        for (auto &Attribute : Class->Attributes) {
                            if (Attribute->getName() == SuperAttribute->getName()) {
                                S.Diag(Attribute->getLocation(), diag::err_sema_super_struct_var_conflict);
                                return;
                            }
                            Class->Attributes.push_back(SuperAttribute);
                        }
                    }
                }

                // Interface cannot extend a Class
                if (Class->getClassKind() == ASTClassKind::INTERFACE &&
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
//                                            ASTClassMethod *M = S.Builder->CreateClassMethod(SuperMethod->getLocation(),
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
//                            for (ASTClassMethod *ISuperMethod: MapIt->second) {
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
        if (Class->getClassKind() == ASTClassKind::CLASS || Class->getClassKind() == ASTClassKind::STRUCT) {

            // Init null value attributes with default values
            for (auto &Attribute : Class->Attributes) {
                // Generate default values
                if (Attribute->getExpr() == nullptr) {
                    ASTValue *DefaultValue = S.Builder->CreateDefaultValue(Attribute->getType());
                    ASTValueExpr *ValueExpr = S.Builder->CreateExpr(DefaultValue);
                    Attribute->setExpr(ValueExpr);
                }
                S.Validator->CheckValueExpr(Attribute->getExpr());
                Attribute->getExpr()->Type = Attribute->getType(); // Maintain expr type
            }
        }

        // Create default constructor if there aren't any other constructors
        // FIXME this code remove default constructor
//                if (!Class->Constructors.empty()) {
//                    delete Class->DefaultConstructor;
//                }

        // Constructors
        for (auto Constructor: Class->Constructors) {

            // Resolve Attribute types
            for (auto &Attribute: Class->Attributes) {
                // TODO
            }
            SemaSpaceSymbols::InsertFunction(IdentitySymbols->Methods, Constructor);
            ResolveStmtBlock(Constructor->Body);
        }

        // Methods
        for (auto Method: Class->Methods) {

            // Add Class vars for each Method
            for (auto &Attribute: Class->Attributes) {

                // Check if Method already contains this var name as LocalVar
                if (!S.Validator->CheckDuplicateLocalVars(Method->Body, Attribute->getName())) {
                    return;
                }
            }

            if (!Method->isAbstract()) {
                ResolveStmtBlock(Method->Body); // FIXME check if already resolved
            }
        }
    }
}

bool SemaResolver::ResolveType(ASTType *Type) {
    if (Type->isIdentity()) {
        return ResolveIdentityType(static_cast<ASTIdentityType *>(Type));
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
        		S.Validator->CheckConvertibleTypes(ReturnStmt->Expr->getType(), ReturnStmt->Parent->getFunction()->getReturnType());
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
                   S.Validator->CheckEqualTypes(SwitchStmt->getVarRef()->getDef()->getType(), ASTTypeKind::TYPE_INTEGER);
    for (ASTRuleStmt *Case : SwitchStmt->Cases) {
    	Success &= ResolveExpr(SwitchStmt, Case->getRule());
        Success &= S.Validator->CheckEqualTypes(Case->getRule()->getType(), ASTTypeKind::TYPE_INTEGER) &&
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
    Success = S.Validator->CheckConvertibleTypes(LoopStmt->Rule->Type, S.Builder->CreateBoolType(SourceLocation()));
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

bool SemaResolver::ResolveIdentityType(ASTIdentityType *IdentityType) {
    assert(IdentityType && "IdentityType cannot be null");

    // Resolve only if not resolved yet
    if (!IdentityType->isResolved()) {

        // Identity can have only NameSpace as Parent: NameSpace.Type
        SemaIdentitySymbols *IdentitySymbols;
        if (IdentityType->getParent()) {

            // Resolve by Imports
            SemaSpaceSymbols *SpaceSymbols = ImportSymbols.lookup(IdentityType->getParent()->getFullName());
            IdentitySymbols = FindIdentity(IdentityType->getName(), SpaceSymbols);
        } else {
            // Resolve in current NameSpace
            IdentitySymbols = FindIdentity(IdentityType->getName(), MySpaceSymbols);

            if (IdentitySymbols == nullptr)
                // Resolve in Default NameSpace
                IdentitySymbols = FindIdentity(IdentityType->getName(), S.DefaultSymbols);
        }

        // Take Identity from NameSpace
        IdentityType->Resolved = true; // Evict Cycle Loop: can be resolved only now

        if (IdentitySymbols) {
            IdentityType->IdentitySymbols = IdentitySymbols;
            IdentityType->Def = IdentitySymbols->getIdentity();
            IdentityType->IdentityTypeKind = IdentitySymbols->getIdentity()->getType()->getIdentityTypeKind();
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

bool SemaResolver::ResolveIdentifier(SemaSpaceSymbols *SpaceSymbols, ASTStmt *Stmt, ASTIdentifier *Identifier) {
    assert(SpaceSymbols && "SpaceSymbols cannot be null");
    assert(Identifier && "Identifier cannot be null");

    if (Identifier->isResolved() == false) {

        if (Identifier->isCall()) { // Call cannot be undefined
            ASTCall *Call = static_cast<ASTCall *>(Identifier);

            // NameSpace.ConstructorCall()... or NameSpace.Call()... 
            Identifier->Resolved = ResolveFunctionCall(SpaceSymbols, Stmt, Call);
            if (Identifier->isResolved() == false) {

                // Constructor
                if (Call->getCallKind() == ASTCallKind::CALL_NEW ||
                    Call->getCallKind() == ASTCallKind::CALL_NEW_UNIQUE ||
                    Call->getCallKind() == ASTCallKind::CALL_NEW_SHARED ||
                    Call->getCallKind() == ASTCallKind::CALL_NEW_WEAK) {
                    SemaIdentitySymbols *IdentitySymbols = FindIdentity(Call->getName(), SpaceSymbols);
                    if (IdentitySymbols)
                        Call->Resolved = ResolveStaticCall(IdentitySymbols, Stmt, Call);
                } else {
                    // Function
                    ASTFunction *Function = FindFunction(Call, SpaceSymbols);
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
                    if (ResolveIdentityType(static_cast<ASTIdentityType *>(Type)))
                        // Can be only a Call or a Var
                        return ResolveIdentifier(static_cast<ASTIdentityType *>(Type)->IdentitySymbols, Stmt, Call->getChild());
                } else {
                    // Error: cannot access to not identity var
                    // TODO
                }
            }
        } else {
            // Identity is Type
            // NameSpace.Type...Call() or NameSpace.Type...Var
            SemaIdentitySymbols *IdentitySymbols = FindIdentity(Identifier->getName(), SpaceSymbols);
            if (IdentitySymbols) {
                if (IdentitySymbols->getIdentity()->getIdentityKind() == ASTIdentityKind::ID_CLASS)
                    Identifier = S.Builder->CreateClassType(Identifier);
                else if (IdentitySymbols->getIdentity()->getIdentityKind() == ASTIdentityKind::ID_ENUM)
                    Identifier = S.Builder->CreateEnumType(Identifier);
                static_cast<ASTIdentityType *>(Identifier)->Def = IdentitySymbols->getIdentity();
                Identifier->Resolved = true;

                // Resolve Child
                if (Identifier->getChild()) {
                    if (Identifier->getChild()->isCall()) {
                        return ResolveStaticCall(IdentitySymbols, Stmt, static_cast<ASTCall *>(Identifier->getChild()));
                    } else {
                        return ResolveStaticVarRef(IdentitySymbols, Stmt, static_cast<ASTVarRef *>(Identifier->getChild()));
                    }
                }

                return Identifier->Resolved;
            }

            // Identity is GlobalVar
            // NameSpace.GlobalVar...Call() or NameSpace.GlobalVar...Var
            return ResolveGlobalVarRef(SpaceSymbols, Stmt, static_cast<ASTVarRef *>(Identifier));
        }
    }

    return Identifier->isResolved();
}

bool SemaResolver::ResolveIdentifier(SemaIdentitySymbols *IdentitySymbols, ASTStmt *Stmt, ASTIdentifier *Identifier) {
    assert(IdentitySymbols && "IdentitySymbols cannot be null");
    assert(Identifier && "Identifier cannot be null");

    if (Identifier->isResolved() == false) {
        if (Identifier->isCall()) { // Call cannot be undefined
            return ResolveStaticCall(IdentitySymbols, Stmt, static_cast<ASTCall*>(Identifier));
        } else {
            return ResolveStaticVarRef(IdentitySymbols, Stmt, static_cast<ASTVarRef*>(Identifier));
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
                SemaIdentitySymbols *IdentitySymbols = FindIdentity(Call->getName(), MySpaceSymbols);
                if (IdentitySymbols)
                    Call->Resolved = ResolveStaticCall(IdentitySymbols, Stmt, Call);
            } else {

                // call function()
                ASTFunctionBase *Function = FindFunction(Call, MySpaceSymbols);
                if (Function == nullptr) {
                    Function = FindFunction(Call, S.DefaultSymbols);
                    Call->Def = Function;
                    Call->Resolved = true;
                }

                // call method()
                if (Function == nullptr) {
                    if (Stmt->getFunction()->getKind() == ASTFunctionKind::CLASS_METHOD) {
                        ASTClass *Class = static_cast<ASTClassMethod*>(Stmt->getFunction())->getClass();
                        Function = FindClassMethod(Call, Class->getType()->IdentitySymbols);
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
                if (ResolveIdentityType(static_cast<ASTIdentityType*>(Type)))
                    // Can be only a Call or a Var
                    return ResolveIdentifier(static_cast<ASTIdentityType*>(Type)->IdentitySymbols, Stmt, Call->getChild());
            }

        } else if (Identifier->isVarRef()) {
            auto *VarRef = static_cast<ASTVarRef*>(Identifier);

            // Search in LocalVars
            // LocalVar.Var or ClassAttribute.Var
            ASTVar *Var = FindLocalVar(Stmt, Identifier->getName());

            // Check if Function is a class Method
            if (Var == nullptr) {
                ASTFunctionBase *Function = Stmt->getFunction();

                // Search for Class Vars if Var is Class Method
                if (Function->getKind() == ASTFunctionKind::CLASS_METHOD) {
                    for (auto &Attribute: static_cast<ASTClassMethod*>(Function)->getClass()->Attributes) {
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
                if (VarRef->getChild() && Var->getType()->isIdentity()) {
                    ASTIdentityType *IdentityType = static_cast<ASTIdentityType*>(Var->getType());
                    return ResolveIdentifier(IdentityType->IdentitySymbols, Stmt, VarRef->getChild());
                }
            }
        } else {
            ASTIdentityType *IdentityType = S.Builder->CreateIdentityType(Identifier);
            if (ResolveIdentityType(IdentityType) && IdentityType->getChild()) {
                return ResolveIdentifier(IdentityType->IdentitySymbols, Stmt, IdentityType->getChild());
            }
        }
    }

    return Identifier->Resolved;
}

bool SemaResolver::ResolveGlobalVarRef(SemaSpaceSymbols *SpaceSymbols, ASTStmt *Stmt, ASTVarRef *VarRef) {
    assert(SpaceSymbols && "NameSpace cannot be null");
    assert(VarRef && "VarRef cannot be null");

    if (VarRef->isResolved() == false) {
        ASTGlobalVar *GlobalVar = FindGlobalVar(VarRef->getName(), SpaceSymbols);
        if (GlobalVar) {
            VarRef->Def = GlobalVar;
            VarRef->Resolved = true;
        }
    }
    return VarRef->Resolved;
}

bool SemaResolver::ResolveStaticVarRef(SemaIdentitySymbols *IdentitySymbols, ASTStmt *Stmt, ASTVarRef *VarRef) {
    assert(IdentitySymbols && "Identity cannot be null");
    assert(VarRef && "VarRef cannot be null");

    if (VarRef->isResolved() == false) {
        if (IdentitySymbols->getIdentity()->getIdentityKind() == ASTIdentityKind::ID_CLASS) {
            ASTClassAttribute *Attribute = IdentitySymbols->getAttributes().lookup(VarRef->getName());
            if (Attribute) {
                VarRef->Def = Attribute;
                VarRef->Resolved = true;
            }
        } else if (IdentitySymbols->getIdentity()->getIdentityKind() == ASTIdentityKind::ID_ENUM) {
            ASTEnumEntry *Entry = IdentitySymbols->getEntries().lookup(VarRef->getName());
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
        SemaSpaceSymbols *SpaceSymbols = FindSpaceSymbols(VarRef, Current);
        if (SpaceSymbols)
            Current->Parent = S.Builder->CreateNameSpace(Current->getParent());

        VarRef->Resolved = (SpaceSymbols && ResolveIdentifier(SpaceSymbols, Stmt, Current)) || // Resolve in NameSpace: Type, Function, GlobalVar
               ResolveIdentifier(Stmt, Current) || // Resolve in statements as LocalVar
               ResolveIdentifier(MySpaceSymbols, Stmt, Current) ||
               ResolveIdentifier(S.DefaultSymbols, Stmt, Current); // Default NameSpace, Class Method or Attribute, LocalVar
    }

    // VarRef not found in Module, namespace and Module imports
    if (VarRef->isResolved() == false) {
        S.Diag(VarRef->getLocation(), diag::err_unref_var) << VarRef->Name;
        return false;
    }

    return true;
}

bool SemaResolver::ResolveFunctionCall(SemaSpaceSymbols *SpaceSymbols, ASTStmt *Stmt, ASTCall *Call) {
    assert(SpaceSymbols && "NameSpace cannot be null");
    assert(Call && "Call cannot be null");

    if (ResolveCallArgs(Stmt, Call)) {
        ASTFunction *Function = FindFunction(Call, SpaceSymbols);
        if (Function) {
            Call->Def = Function;
            Call->Resolved = true;
        }
    }
    return Call->isResolved();
}

bool SemaResolver::ResolveStaticCall(SemaIdentitySymbols *IdentitySymbols, ASTStmt *Stmt, ASTCall *Call) {
    assert(IdentitySymbols && "IdentitySymbols cannot be null");
    assert(Call && "Call cannot be null");

    if (ResolveCallArgs(Stmt, Call)) {
        if (IdentitySymbols->getIdentity()->getIdentityKind() == ASTIdentityKind::ID_CLASS) {
            ASTClassMethod *Method = FindClassMethod(Call, IdentitySymbols);
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
        SemaSpaceSymbols *SpaceSymbols = FindSpaceSymbols(Call, Current);
        if (SpaceSymbols)
            Current->Parent = S.Builder->CreateNameSpace(Current->getParent());

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
        Call->Resolved = (SpaceSymbols && ResolveIdentifier(SpaceSymbols, Stmt, Current)) || // Resolve in NameSpace: Type, Function, GlobalVar
           ResolveIdentifier(Stmt, Current) || // Resolve in statements as LocalVar
           ResolveIdentifier(MySpaceSymbols, Stmt, Current) || // Module NameSpace
           ResolveIdentifier(S.DefaultSymbols, Stmt, Current); // Default NameSpace, Class Method or Attribute, LocalVar
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
                        ASTClassMethod *Def = static_cast<ASTClassMethod*>(Call->Def);
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
                            Success = S.Validator->CheckArithTypes(Binary->getLocation(), Binary->LeftExpr->Type,
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
                            Success = S.Validator->CheckLogicalTypes(Binary->getLocation(),
                                                                     Binary->LeftExpr->Type, Binary->RightExpr->Type);
                            Binary->Type = S.Builder->CreateBoolType(Binary->getLocation());
                        }
                    }
                    break;
                }
                case ASTOpExprKind::OP_TERNARY: {
                    ASTTernaryOpExpr *Ternary = static_cast<ASTTernaryOpExpr*>(Expr);
                    Success = ResolveExpr(Stmt, Ternary->ConditionExpr) &&
                              S.Validator->CheckConvertibleTypes(Ternary->ConditionExpr->Type, S.Builder->CreateBoolType(SourceLocation())) &&
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

SemaSpaceSymbols *SemaResolver:: FindSpaceSymbols(ASTIdentifier *Identifier, ASTIdentifier *&Current) const {
    // Find NameSpace by iterating parents
    // one.two.three.four
    // four.Parent, three.Parent, two.Parent, one.Parent
    SemaSpaceSymbols *SpaceSymbols = nullptr;
    Current = Identifier;
    while (Current->getParent()) {
        // Check from Imports
        SpaceSymbols = ImportSymbols.lookup(Current->getParent()->getFullName());
        if (SpaceSymbols)
            break;
        else
            Current = Current->getParent();
    }
    
    return SpaceSymbols;
}

ASTGlobalVar *SemaResolver::FindGlobalVar(llvm::StringRef Name, SemaSpaceSymbols *SpaceSymbols) const {
    FLY_DEBUG_MESSAGE("Sema", "FindGlobalVar", Logger().Attr("Name", Name).End());
    return SpaceSymbols->GlobalVars.lookup(Name);
}

SemaIdentitySymbols *SemaResolver::FindIdentity(llvm::StringRef Name, SemaSpaceSymbols *SpaceSymbols) const {
    FLY_DEBUG_MESSAGE("Sema", "FindIdentity", Logger().Attr("Name", Name).End());
    return SpaceSymbols->getIdentities().lookup(Name);
}

ASTFunction *SemaResolver::FindFunction(ASTCall *Call, SemaSpaceSymbols *SpaceSymbols) const {
    FLY_DEBUG_MESSAGE("Sema", "FindFunction", Logger().Attr("Call", (ASTIdentifier *) Call).End());
    return FindFunction(Call, SpaceSymbols->getFunctions());
}

ASTClassMethod *SemaResolver::FindClassMethod(ASTCall *Call, SemaIdentitySymbols *IdentitySymbols) const {
    FLY_DEBUG_MESSAGE("Sema", "FindClassMethod", Logger().Attr("Call", (ASTIdentifier *) Call).End());
    return FindFunction(Call, IdentitySymbols->Methods);
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
                            ASTParam *Param = Function->getParams()[i];
                            Success &= S.Validator->CheckConvertibleTypes(Arg->getExpr()->getType(), Param->getType());
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
    } else { // Search into ASTParam list
        llvm::SmallVector<ASTParam *, 8> Params = Stmt->getFunction()->getParams();
        for (auto &Param : Params) {
            if (Param->getName() == Name) {
                return Param;
            }
        }
    }
    return nullptr;
}

SemaSpaceSymbols *SemaResolver::AddImportSymbols(llvm::StringRef Name) {
    SemaSpaceSymbols *SpaceSymbols = S.MapSymbols.lookup(Name);
    ImportSymbols.insert(std::make_pair(Name, SpaceSymbols));
    return SpaceSymbols;
}
