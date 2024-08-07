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
#include "Sema/SemaSymbols.h"
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
#include "AST/ASTHandleStmt.h"
#include "AST/ASTDeleteStmt.h"
#include "AST/ASTVar.h"
#include "AST/ASTVarStmt.h"
#include "AST/ASTFailStmt.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTGroupExpr.h"
#include "AST/ASTOperatorExpr.h"
#include "CodeGen/CodeGen.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"

#include "llvm/ADT/StringMap.h"

#include <string>

using namespace fly;

SemaResolver::SemaResolver(Sema &S, ASTModule *Module, SemaSymbols *Symbols) :
        S(S), Module(Module), NameSpace(Symbols) {

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

        // If no NameSpace is assign set Default NameSpace
        if (Module->getNameSpaces().empty())
            Module->NameSpaces.push_back(new ASTNameSpace(SourceLocation(), ASTContext::DEFAULT_NAMESPACE));

        // Error: Multiple NameSpaces declared
        if (Module->getNameSpaces().size() > 1)
            S.Diag(diag::err_sema_multiple_module_namespaces) << Module->getName();

        // Use NameSpace to create MapSymbols
        ASTNameSpace *NameSpace = Module->getNameSpace();
        SemaSymbols *Symbols = S.MapSymbols.lookup(NameSpace->getName());
        if (Symbols == nullptr) {
            Symbols = new SemaSymbols(S);
            Symbols->NameSpace = NameSpace->getName();
            S.MapSymbols.insert(std::make_pair(NameSpace->getName(), Symbols));
        }

        // Add Module in the Symbol Modules: you can retrieve all Modules from a NameSpace
        Symbols->Modules.push_back(Module);

        // Resolve declarations
        SemaResolver *Resolver = new SemaResolver(S, Module, Symbols);
        Resolver->ResolveGlobalVarDeclarations(); // resolve Global Vars
        Resolver->ResolveIdentityDeclarations();  // resolve Identity attributes and methods
        Resolver->ResolveFunctionDeclarations();  // resolve ASTBlock of Body Functions

        // add to Resolvers list
        Resolvers.push_back(Resolver);
    }

    // Second: Resolve Definitions
    for (auto &Resolver : Resolvers) {
        Resolver->ResolveImportDefinitions();
        Resolver->ResolveGlobalVarDefinitions();
        Resolver->ResolveIdentityDefinitions();
        Resolver->ResolveFunctionDefinitions();

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
        ASTGlobalVar *LookupVar = NameSpace->getGlobalVars().lookup(GlobalVar->getName());

        if (LookupVar) { // This NameSpace already contains this GlobalVar
            S.Diag(LookupVar->getLocation(), diag::err_duplicate_gvar) << LookupVar->getName();
            return;
        }

        // Add into NameSpace for next resolve
        NameSpace->GlobalVars.insert(std::make_pair(GlobalVar->getName(), GlobalVar));
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
        SemaSymbols::InsertFunction(NameSpace->Functions, Function);
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
        ASTIdentity *LookupIdentity = NameSpace->getIdentities().lookup(Identity->getName());

        if (LookupIdentity) { // This NameSpace already contains this Identity
            S.Diag(LookupIdentity->getLocation(), diag::err_duplicate_identity) << LookupIdentity->getName();
            return;
        }

        // Add into NameSpace for next resolve
        NameSpace->Identities.insert(std::make_pair(Identity->getName(), Identity));
    }
}

/**
 * ResolveModule Import Definitions
 */
void SemaResolver::ResolveImportDefinitions() {
    for (auto &Import : Module->getImports()) {

        // Search Namespace of the Import
        SemaSymbols *ImportNameSpace = S.MapSymbols.lookup(Import->getName());

        if (!ImportNameSpace) {
            // Error: NameSpace not found
            S.Diag(Import->getLocation(), diag::err_namespace_notfound) << Import->getName();
            return;
        }

        // Check import
        S.Validator->CheckImport(Module, Import);

        // Check import duplications
        llvm::StringRef ImportName = Import->getName();
        auto Duplicate = Imports.lookup(Import->getName());
        if (Duplicate) {
            S.Diag(Import->getLocation(), diag::err_conflict_import) << Import->getName();
            return;
        }


        if (Import->getAlias()) {

            // Check Alias
            llvm::StringRef AliasName = Import->getAlias()->getName();
            Duplicate = Imports.lookup(AliasName);
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
void SemaResolver::ResolveGlobalVarDefinitions() {
    for (auto GlobalVar : Module->getGlobalVars()) {

        // Check Expr Value
        if (GlobalVar->Expr->getExprKind() != ASTExprKind::EXPR_VALUE) {
            S.Diag(GlobalVar->Expr->getLocation(), diag::err_invalid_gvar_value);
        }

        // Resolve Type
        ResolveType(GlobalVar->getType());
    }
}

/**
 * Resolve Module Identity Definitions
 */
void SemaResolver::ResolveIdentityDefinitions() {
    for (auto Identity : Module->Identities) {

        if (Identity->getTopDefKind() == ASTTopDefKind::DEF_CLASS) {
            ASTClass *Class = (ASTClass *) Identity;

            // Resolve Super Classes
            if (!Class->SuperClasses.empty()) {
                llvm::StringMap<std::map<uint64_t, llvm::SmallVector<ASTClassMethod *, 4>>> SuperMethods;
                llvm::StringMap<std::map<uint64_t, llvm::SmallVector<ASTClassMethod *, 4>>> ISuperMethods;
                for (ASTIdentityType *SuperClassType: Class->SuperClasses) {
                    ResolveType(SuperClassType);
                    ASTClass *SuperClass = (ASTClass *) SuperClassType->getDef();

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
            for (auto Function: Class->Constructors) {

                // Resolve Attribute types
                for (auto &Attribute: Class->Attributes) {
                    // TODO
                }

                ResolveStmtBlock(Function->Body);
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

        } else if (Identity->getTopDefKind() == ASTTopDefKind::DEF_ENUM) {
            // TODO
        }
    }
}

/**
 * Resolve Module Function Definitions
 */
void SemaResolver::ResolveFunctionDefinitions() {
    for (auto Function : Module->getFunctions()) {

        // Resolve Return Type
        ResolveType(Function->getReturnType());

        // Resolve Parameters Types
        for (auto &Param : Function->getParams()) {
            ResolveType(Param->getType());
        }

        // Resolve Function Body
        ResolveStmtBlock(Function->Body);
    }
}

void SemaResolver::ResolveType(ASTType *Type) {
    if (Type->isIdentity()) {
        ResolveIdentityType((ASTIdentityType *) Type);
    } else if (Type->isArray()) {
        ASTArrayType *ArrayType = (ASTArrayType *) Type;
        ResolveType(ArrayType->getType());
    }
}

void SemaResolver::ResolveIdentityType(ASTIdentityType *IdentityType) {
    // Resolve only if not resolved yet
    if (IdentityType->getDef() == nullptr) {

        // Identity without parent
        ASTIdentity *Identity = FindIdentity(IdentityType);
        if (Identity) {
            IdentityType->Def = Identity;
            IdentityType->IdentityTypeKind = Identity->getType()->getIdentityTypeKind();
        } else {
            S.Diag(IdentityType->getLocation(), diag::err_sema_resolve_identifier);
            return;
        }
    }

    if (!IdentityType->Def) {
        S.Diag(IdentityType->getLocation(), diag::err_unref_type);
        return;
    }
}

bool SemaResolver::ResolveStmt(ASTStmt *Stmt) {
    switch (Stmt->getKind()) {

        case ASTStmtKind::STMT_BLOCK:
            return ResolveStmtBlock((ASTBlockStmt *) Stmt);
        case ASTStmtKind::STMT_IF:
            return ResolveStmtIf((ASTIfStmt *) Stmt);
        case ASTStmtKind::STMT_SWITCH:
            return ResolveStmtSwitch((ASTSwitchStmt *) Stmt);
        case ASTStmtKind::STMT_LOOP:
            return ResolveStmtLoop((ASTLoopStmt *) Stmt);
        case ASTStmtKind::STMT_LOOP_IN:
            return ResolveStmtLoopIn((ASTLoopInStmt *) Stmt);
        case ASTStmtKind::STMT_VAR: {
            ASTVarStmt *VarStmt = (ASTVarStmt *) Stmt;
            ResolveVarRef(Stmt->Parent, VarStmt->getVarRef());
            if (VarStmt->getVarRef()->Def && VarStmt->getExpr() != nullptr && !VarStmt->getVarRef()->getDef()->isInitialized())
                VarStmt->getVarRef()->getDef()->setInitialization(VarStmt); // FIXME ? with if - else
            return VarStmt->getVarRef()->Def && ResolveExpr(VarStmt->Parent, VarStmt->Expr, VarStmt->getVarRef()->getDef()->getType());
        }
        case ASTStmtKind::STMT_EXPR:
            return ResolveExpr(Stmt->Parent, ((ASTExprStmt *) Stmt)->Expr);
        case ASTStmtKind::STMT_FAIL:
            return ResolveExpr(Stmt->Parent, ((ASTFailStmt *) Stmt)->Expr);
        case ASTStmtKind::STMT_HANDLE: {
            ASTHandleStmt *HandlStmt = (ASTHandleStmt *) Stmt;
            bool Success = true;
            if (HandlStmt->ErrorHandlerRef != nullptr)
                Success = ResolveVarRef(Stmt->Parent, HandlStmt->ErrorHandlerRef);
            return Success && ResolveStmt(HandlStmt->Handle);
        }
        case ASTStmtKind::STMT_DELETE: {
            ASTDeleteStmt *DeleteStmt = (ASTDeleteStmt *) Stmt;
            return ResolveVarRef(Stmt->Parent, DeleteStmt->VarRef);
        }
        case ASTStmtKind::STMT_RETURN: {
            ASTReturnStmt *ReturnStmt = (ASTReturnStmt *) Stmt;
            return ResolveExpr(Stmt->Parent, ReturnStmt->Expr) &&
                   S.Validator->CheckConvertibleTypes(ReturnStmt->Expr->getType(), Stmt->getFunction()->getReturnType());
        }
        case ASTStmtKind::STMT_BREAK:
        case ASTStmtKind::STMT_CONTINUE:
            return true;
    }

    assert(false && "Invalid ASTStmtKind");
}

bool SemaResolver::ResolveStmtBlock(ASTBlockStmt *Block) {
    for (ASTStmt *Stmt : Block->Content) {
        ResolveStmt(Stmt);
    }
    for (auto &LocalVar : Block->LocalVars) {
        if (!LocalVar.second->isInitialized())
            S.Diag(LocalVar.getValue()->getLocation(), diag::err_sema_uninit_var) << LocalVar.getValue()->getName();
    }
    return true;
}

bool SemaResolver::ResolveStmtIf(ASTIfStmt *IfStmt) {
    IfStmt->Condition->Type = S.Builder->CreateBoolType(IfStmt->Condition->getLocation());
    bool Success = ResolveExpr(IfStmt->getParent(), IfStmt->Condition) &&
                   S.Validator->CheckConvertibleTypes(IfStmt->Condition->Type, S.Builder->CreateBoolType(SourceLocation())) &&
                   ResolveStmt(IfStmt->Stmt);
    for (ASTElsif *Elsif : IfStmt->Elsif) {
        Elsif->Condition->Type = S.Builder->CreateBoolType(Elsif->Condition->getLocation());
        Success &= ResolveExpr(IfStmt->getParent(), Elsif->Condition) &&
                   S.Validator->CheckConvertibleTypes(Elsif->Condition->Type, S.Builder->CreateBoolType(SourceLocation())) &&
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
    for (ASTSwitchCase *Case : SwitchStmt->Cases) {
        Success &= S.Validator->CheckEqualTypes(Case->getValueExpr()->getType(), ASTTypeKind::TYPE_INTEGER) &&
                ResolveStmt(Case->Stmt);
    }
    return Success && ResolveStmt(SwitchStmt->Default);
}

bool SemaResolver::ResolveStmtLoop(ASTLoopStmt *LoopStmt) {
    LoopStmt->Init->Parent = LoopStmt;
    LoopStmt->Init->Function = LoopStmt->Function;
    LoopStmt->Loop->Parent = LoopStmt->Init;
    bool Success = ResolveStmt(LoopStmt->Init) && ResolveExpr(LoopStmt->Init, LoopStmt->Condition);
    Success = S.Validator->CheckConvertibleTypes(LoopStmt->Condition->Type, S.Builder->CreateBoolType(LoopStmt->Condition->getLocation()));
    Success &= LoopStmt->Loop ? ResolveStmt(LoopStmt->Loop) : true;
    Success &= LoopStmt->Post ? ResolveStmt(LoopStmt->Post) : true;
    return Success;
}

bool SemaResolver::ResolveStmtLoopIn(ASTLoopInStmt *LoopInStmt) {
    return ResolveVarRef(LoopInStmt->Parent, LoopInStmt->VarRef) && ResolveStmtBlock(LoopInStmt->Block);
}

void SemaResolver::ResolveIdentifier(ASTIdentifier *&Identifier, ASTStmt *Stmt) {
    // Call().Call() or Call().Var
    // LocalVar.Call() or LocalVar.Var
    // NameSpace.Type
    // NameSpace.Call() or NameSpace.Var
    // GlobalVar.Call() or GlobalVar.Var
    // Type.Call() or Type.Var

    bool Result = false;
    switch (Identifier->getIdKind()) {

        case ASTIdentifierKind::REF_UNDEFINED:
            ResolveUndefinedIdentifier(Identifier);
            break;

        case ASTIdentifierKind::REF_NAMESPACE:
            break;
        case ASTIdentifierKind::REF_IMPORT:
            break;
        case ASTIdentifierKind::REF_ALIAS:
            break;
        case ASTIdentifierKind::REF_TYPE:
            ResolveIdentityType((ASTIdentityType *) Identifier);
            break;
        case ASTIdentifierKind::REF_CALL:
            ResolveCall(Stmt, (ASTCall *) Identifier);
            break;
        case ASTIdentifierKind::REF_VAR:
            ResolveVarRef(Stmt, (ASTVarRef *) Identifier);
            break;
    }

    if (Identifier->getParent()) {
        ResolveIdentifier(Identifier->Parent, Stmt);
    }
}

bool SemaResolver::ResolveUndefinedIdentifier(ASTIdentifier *&Identifier) {


    // NameSpace.Type
    // NameSpace.Call() or NameSpace.Var
    // GlobalVar.Call() or GlobalVar.Var
    // Type.Call() or Type.Var
    if (Identifier->getChild()) {
        switch (Identifier->getChild()->getIdKind()) {

            case ASTIdentifierKind::REF_UNDEFINED:
            case ASTIdentifierKind::REF_NAMESPACE:
            case ASTIdentifierKind::REF_IMPORT:
            case ASTIdentifierKind::REF_ALIAS:
                S.Diag(Identifier->getLocation(), diag::err_sema_wrong_child_type);
                break;

            case ASTIdentifierKind::REF_TYPE:
                // NameSpace.Type
                if (Imports.find(Identifier->getFullName()) != Imports.end()) {
                    Identifier = S.Builder->CreateNameSpace(Identifier);
                    return true;
                }
                break;

            case ASTIdentifierKind::REF_CALL:
            case ASTIdentifierKind::REF_VAR:
                // NameSpace.Call() or NameSpace.Type.Var
                if (Imports.find(Identifier->getFullName()) != Imports.end()) {
                    Identifier = S.Builder->CreateNameSpace(Identifier);
                    return true;
                }

                // GlobalVar.Call() or GlobalVar.Var
                ASTGlobalVar *GlobalVar = NameSpace->GlobalVars.lookup(Identifier->getName());
                if (GlobalVar) {
                    ((ASTVarRef *) Identifier)->Def = GlobalVar;
                    return true;
                }

                // Type.Call() or Type.Var
                ASTIdentity *Identity = NameSpace->getIdentities().lookup(Identifier->getName());
                if (Identity) {
                    ((ASTIdentityType *) Identifier)->Def = Identity;
                    return true;
                }
                break;
        }
    }

    // Error:
    S.Diag(Identifier->getLocation(), diag::err_sema_resolve_identifier);
    return false;
}

/**
 * ResolveModule a VarRef with its declaration
 * @param VarRef
 * @return true if no error occurs, otherwise false
 */
bool SemaResolver::ResolveVarRef(ASTStmt *Stmt, ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveVarRefWithParent", Logger().Attr("VarRef", VarRef).End());
    
    if (!VarRef->Def) {

        // With Parent
        if (VarRef->getParent()) {
            ResolveIdentifier(VarRef->Parent, Stmt);

            switch (VarRef->getParent()->getIdKind()) {

                case ASTIdentifierKind::REF_NAMESPACE: { // Namespace.globalVar
                    ASTNameSpace *NS = (ASTNameSpace *) VarRef->getParent();
                    SemaSymbols *FoundNS = FindNameSpace(NS);
                    VarRef->Def = FoundNS->GlobalVars.lookup(VarRef->getName());
                    break;
                }

                case ASTIdentifierKind::REF_TYPE: {
                    ASTIdentityType *IdentityType = (ASTIdentityType *) VarRef->getParent();
                    // ClassType.STATIC_VAR
                    // EnumType.ENUM_VAR
                    // StructType.fieldVar
                    VarRef->Def = ResolveVarRef(VarRef->getName(), IdentityType);
                    break;
                }

                    // Instance
                case ASTIdentifierKind::REF_CALL: // NameSpace.call().var or call().var
                {
                    ASTCall *ParentCall = (ASTCall*) VarRef->getParent();
                    ASTType * ParentType = ParentCall->getDef()->getReturnType();

                    // Parent is an Identity instance
                    if (ParentType->isIdentity())
                        VarRef->Def = ResolveVarRef(VarRef->getName(), (ASTIdentityType *) ParentType);
                    break;
                }

                case ASTIdentifierKind::REF_VAR: // NameSpace.globalVarInstance.var or instanceVar.var
                {
                    ASTVarRef *ParentVarRef = (ASTVarRef *) VarRef->getParent();
                    ASTType * ParentType = ParentVarRef->getDef()->getType();

                    // Parent is an Identity instance
                    if (ParentType->isIdentity())
                        VarRef->Def = ResolveVarRef(VarRef->getName(), (ASTIdentityType *) ParentType);
                    break;
                }

                    // Error: identifier not resolved
                case ASTIdentifierKind::REF_UNDEFINED:
                    assert(false && "Unexpected Identifier Kind");
            }
        }

        // Search for LocalVar
        ASTVar *Var = FindLocalVar(Stmt, VarRef->getName());

        if (Var == nullptr) {
            ASTFunctionBase *Function = Stmt->getFunction();

            // Search for Class Vars if Var is Class Method
            if (Function->getKind() == ASTFunctionKind::CLASS_METHOD)
                for (auto &Attribute : ((ASTClassMethod *) Function)->getClass()->Attributes) {
                    if (Attribute->getName() == VarRef->getName()) {
                        Var = Attribute;
                    }
                }

            // Search for GlobalVars in Module
            if (Var == nullptr)
                for (auto &GlobalVar : Module->GlobalVars)
                    if (GlobalVar->getName() == VarRef->getName())
                        Var = GlobalVar;

            // Search for GlobalVars in NameSpace
            if (Var == nullptr)
                Var = NameSpace->GlobalVars.lookup(VarRef->getName());
        }
    }

    // VarRef not found in Module, namespace and Module imports
    if (VarRef->getDef() == nullptr) {
        S.Diag(VarRef->getLocation(), diag::err_unref_var) << VarRef->Name;
        return false;
    }

    return true;
}

ASTVar *SemaResolver::ResolveVarRef(llvm::StringRef Name, ASTIdentityType *IdentityType) {
    if (IdentityType->isClass()) {
        for (auto &Attribute : ((ASTClass *) ((ASTClassType *) IdentityType)->getDef())->Attributes) {
            if (Attribute->getName() == Name) {
                return Attribute;
            }
        }
    } else if (IdentityType->isEnum()) {
        for (auto &EnumEntry : ((ASTEnum *) ((ASTEnumType *) IdentityType)->getDef())->Entries) {
            if (EnumEntry->getName() == Name) {
                return EnumEntry;
            }
        }
    } else {
        assert(false && "IdentityType unknown");
    }
    return nullptr;
}

bool SemaResolver::ResolveCall(ASTStmt *Stmt, ASTCall *Call) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveCall", Logger().Attr("Call", Call).End());

    if (!Call->Def) {

        if (Call->getParent()) {
            ResolveIdentifier(Call->Parent, Stmt);

            switch (Call->getParent()->getIdKind()) {

                case ASTIdentifierKind::REF_NAMESPACE: {
                    ASTNameSpace *Parent = (ASTNameSpace *) Call->getParent();

                    // NameSpace.func()
                    bool Success = ResolveCall(Stmt, Call, Parent->Functions);

                    // NameSpace.constructor()
                    if (!Success) {
                        ASTIdentity *Identity = FindIdentity(Call->getName(), Parent);
                        Identity != nullptr && Identity->getTopDefKind() == ASTTopDefKind::DEF_CLASS &&
                        ResolveCall(Stmt, Call, ((ASTClass *) Identity)->Constructors);
                    }
                }

                case ASTIdentifierKind::REF_TYPE: {
                    ASTIdentityType *IdentityType = (ASTIdentityType *) Call->getParent();
                    // NameSpace.IdentityType.call() or IdentityType.call()
                    ResolveCall(Stmt, Call, IdentityType);
                    break;
                }

                    // Instance
                case ASTIdentifierKind::REF_CALL: // NameSpace.call().call() call().call()
                {
                    ASTCall *ParentCall = (ASTCall*) Call->getParent();

                    // Parent is an Identity instance
                    ASTType * ParentType = ParentCall->getDef()->getReturnType();
                    return ParentType->isIdentity() && ResolveCall(Stmt, Call, (ASTIdentityType *) ParentType);
                }
                case ASTIdentifierKind::REF_VAR: // NameSpace.globalVarInstance.call() or instance.call()
                {
                    ASTVarRef *ParentVarRef = (ASTVarRef *) Call->getParent();

                    // Parent is an Identity instance
                    ASTType * ParentType = ParentVarRef->getDef()->getType();
                    return ParentType->isIdentity() && ResolveCall(Stmt, Call, (ASTIdentityType *) ParentType);
                }

                    // Error: identifier not resolved
                case ASTIdentifierKind::REF_UNDEFINED:
                    assert(false && "Unexpected Identifier Kind");
            }
        }

        // func()
        bool Success = ResolveCall(Stmt, Call, S.DefaultSymbols->Functions) ||
                       ResolveCall(Stmt, Call, NameSpace->Functions);

        // constructor()
        if (!Success) {
            ASTIdentity *Identity = FindIdentity(Call->getName(), Module->getNameSpace());
            Identity != nullptr && Identity->getTopDefKind() == ASTTopDefKind::DEF_CLASS &&
            ResolveCall(Stmt, Call, ((ASTClass *) Identity)->Constructors);
        }
    }

    // VarRef not found in Module, namespace and Module imports
    if (Call->getDef() == nullptr) {
        S.Diag(Call->getLocation(), diag::err_unref_call);
        return false;
    }

    Call->ErrorHandler = Stmt->ErrorHandler;
    return true;
}

bool SemaResolver::ResolveCall(ASTStmt *Stmt, ASTCall *Call, ASTIdentityType *IdentityType) {

    if (IdentityType->isClass()) {
        auto &ClassMethods = ((ASTClass *) ((ASTClassType *) IdentityType)->getDef())->Methods;
        return ResolveCall(Stmt, Call, ClassMethods);
    } else if (IdentityType->isEnum()) {
        S.Diag(Call->getLocation(), diag::err_sema_call_enum);
    } else {
        assert(false && "IdentityType unknown");
    }

    return false;
}

template <class T>
bool SemaResolver::ResolveCall(ASTStmt *Stmt, ASTCall *Call,
                               llvm::StringMap<std::map <uint64_t,llvm::SmallVector <T *, 4>>> &Functions) {

    // Search by Call Name
    auto StrMapIt = Functions.find(Call->getName());
    if (StrMapIt != Functions.end()) {
        std::map<uint64_t, llvm::SmallVector<T *, 4>> &IntMap = StrMapIt->getValue();
        return ResolveCall(Stmt, Call, IntMap);
    }

    return Call->Def;
}

template <class T>
bool SemaResolver::ResolveCall(ASTStmt *Stmt, ASTCall *Call,
                               std::map <uint64_t,llvm::SmallVector <T *, 4>> &Functions) {
    // Search by number of arguments
    const auto &IntMapIt = Functions.find(Call->getArgs().size());
    if (IntMapIt != Functions.end()) { // Map contains Function with this size of args
        S.Validator->DiagEnabled = false;
        for (T *Function : IntMapIt->second) {
            if (Function->getParams().size() == Call->getArgs().size()) {
                bool Success = true; // if Params = Args = 0 skip for cycle
                if (Call->getArgs().empty()) { // call function with no parameters
                    Success = true;
                } else {
                    for (unsigned long i = 0; i < Function->getParams().size(); i++) {
                        // Resolve Arg Expr on first
                        ASTArg *Arg = Call->getArgs()[i];
                        ASTParam *Param = Function->getParams()[i];
                        Success &= ResolveArg(Stmt, Arg, Param);
                    }
                }

                if (Success) {
                    Call->Def = Function;
                    break;
                }
            }
        }
        S.Validator->DiagEnabled = true;
    }

    return Call->Def;
}

bool SemaResolver::ResolveArg(ASTStmt *Stmt, ASTArg *Arg, ASTParam *Param) {
    Arg->Def = Param;
    if (ResolveExpr(Stmt, Arg->Expr)) {
        return S.Validator->CheckConvertibleTypes(Arg->Expr->Type, Param->getType());
    }

    return false;
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
        case ASTExprKind::EXPR_EMPTY:
            return true;
        case ASTExprKind::EXPR_VALUE: {
            // Select the best option for this Value
            ASTValueExpr *ValueExpr = (ASTValueExpr *) Expr;
            if (Type != nullptr)
                ValueExpr->Type = Type;
            return S.getValidator().CheckValue(ValueExpr->getValue());
        }
        case ASTExprKind::EXPR_VAR_REF: {
            ASTVarRef *VarRef = ((ASTVarRefExpr *)Expr)->getVarRef();
            if (ResolveVarRef(Stmt, VarRef)) {
                Expr->Type = VarRef->getDef()->getType();
                Success = true;
                break;
            } else {
                return false;
            }
        }
        case ASTExprKind::EXPR_CALL: {
            ASTCall *Call = ((ASTCallExpr *)Expr)->getCall();
            if (ResolveCall(Stmt, Call)) {
                switch (Call->getCallKind()) {

                    case ASTCallKind::CALL_FUNCTION:
                        Expr->Type = Call->Def->ReturnType;
                        break;
                    case ASTCallKind::CALL_CONSTRUCTOR:
                        Expr->Type = ((ASTClassMethod *) Call->Def)->getClass()->getType();
                        break;
                }
                Success = true;
                break;
            } else {
                return false;
            }
        }
        case ASTExprKind::EXPR_GROUP: {
            switch (((ASTGroupExpr *) Expr)->getGroupKind()) {
                case ASTExprGroupKind::GROUP_UNARY: {
                    ASTUnaryGroupExpr *Unary = (ASTUnaryGroupExpr *) Expr;
                    Success = ResolveExpr(Stmt, (ASTExpr *) Unary->First);
                    Expr->Type = Unary->First->Type;
                    break;
                }
                case ASTExprGroupKind::GROUP_BINARY: {
                    ASTBinaryGroupExpr *Binary = (ASTBinaryGroupExpr *) Expr;

                    if (Binary->First->Kind == ASTExprKind::EXPR_EMPTY) {
                        // Error: Binary cannot contain ASTEmptyExpr
                        S.Diag(Binary->First->getLocation(), diag::err_sema_empty_expr);
                        return false;
                    }

                    if (Binary->Second->Kind == ASTExprKind::EXPR_EMPTY) {
                        // Error: Binary cannot contain ASTEmptyExpr
                        S.Diag(Binary->Second->getLocation(), diag::err_sema_empty_expr);
                        return false;
                    }

                    Success = ResolveExpr(Stmt, Binary->First) && ResolveExpr(Stmt, Binary->Second);
                    if (Success) {
                        if (Binary->getOperator()->getOptionKind() == ASTBinaryOptionKind::BINARY_ARITH ||
                                Binary->getOperator()->getOptionKind() == ASTBinaryOptionKind::BINARY_COMPARISON) {
                            Success = S.Validator->CheckArithTypes(Binary->getLocation(), Binary->First->Type,
                                                                  Binary->Second->Type);

                            if (Success) {
                                // Selects the largest data Type
                                // Promotes First or Second Expr Types in order to be equal
                                if (Binary->First->Type->isInteger()) {
                                    if (((ASTIntegerType *)Binary->First->Type)->getSize() > ((ASTIntegerType *)Binary->Second->Type)->getSize())
                                        Binary->Second->Type = Binary->First->Type;
                                    else
                                        Binary->First->Type = Binary->Second->Type;
                                } else if (Binary->First->Type->isFloatingPoint()) {
                                    if (((ASTFloatingPointType *)Binary->First->Type)->getSize() > ((ASTFloatingPointType *)Binary->Second->Type)->getSize())
                                        Binary->Second->Type = Binary->First->Type;
                                    else
                                        Binary->First->Type = Binary->Second->Type;
                                }

                                Binary->Type = Binary->getOperator()->getOptionKind() == ASTBinaryOptionKind::BINARY_ARITH ?
                                        Binary->First->Type : S.Builder->CreateBoolType(Expr->getLocation());
                            }
                        } else if (Binary->getOperator()->getOptionKind() ==  ASTBinaryOptionKind::BINARY_LOGIC) {
                            Success = S.Validator->CheckLogicalTypes(Binary->getLocation(),
                                                                     Binary->First->Type, Binary->Second->Type);
                            Binary->Type = S.Builder->CreateBoolType(Expr->getLocation());
                        }
                    }
                    break;
                }
                case ASTExprGroupKind::GROUP_TERNARY: {
                    ASTTernaryGroupExpr *Ternary = (ASTTernaryGroupExpr *) Expr;
                    Success = ResolveExpr(Stmt, Ternary->First) &&
                              S.Validator->CheckConvertibleTypes(Ternary->First->Type, S.Builder->CreateBoolType(SourceLocation())) &&
                              ResolveExpr(Stmt, Ternary->Second) &&
                              ResolveExpr(Stmt, Ternary->Third);
                    Ternary->Type = Ternary->Second->Type; // The group type is equals to the second type
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

SemaSymbols *SemaResolver:: FindNameSpace(ASTIdentifier *Identifier) {
    switch (Identifier->getIdKind()) {
        case ASTIdentifierKind::REF_NAMESPACE:
            return S.MapSymbols.lookup(Identifier->getFullName());
        case ASTIdentifierKind::REF_IMPORT:
            return S.MapSymbols.lookup(Identifier->getFullName());
        case ASTIdentifierKind::REF_ALIAS:
            return S.MapSymbols.lookup(((ASTAlias *) Identifier)->getImport()->getFullName());
        case ASTIdentifierKind::REF_TYPE:
        case ASTIdentifierKind::REF_CALL:
        case ASTIdentifierKind::REF_VAR:
            if (Identifier->getParent()) {
                return FindNameSpace(Identifier->getParent());
            } else {
                return NameSpace;
            }
        case ASTIdentifierKind::REF_UNDEFINED:
            if (ResolveUndefinedIdentifier(Identifier)) {
                return FindNameSpace(Identifier);
            }
            break;
    }

    return nullptr;
}

ASTGlobalVar *SemaResolver::FindGlobalVar(ASTIdentifier *Identifier) const {

}

ASTIdentity *SemaResolver::FindIdentity(ASTIdentityType *IdentityType) {
    FLY_DEBUG_MESSAGE("Sema", "FindIdentity", Logger().Attr("IdentityType", (ASTIdentifier *) IdentityType).End());
    ASTIdentity *Identity = nullptr;

    SemaSymbols *Symbols = FindNameSpace(IdentityType);
    if (Symbols) {
        Identity = Symbols->getIdentities().lookup(IdentityType->getName());
        if (Identity) {
            return Identity;
        }
    }

    S.Diag(IdentityType->getLocation(), diag::err_sema_type_notfound);
    return nullptr;
}

/**
 * Search a VarRef into declared Block's vars
 * If found set LocalVar
 * @param Stmt
 * @param Identifier
 * @return the found LocalVar
 */
ASTVar *SemaResolver::FindLocalVar(ASTStmt *Stmt, llvm::StringRef Name) const {
    FLY_DEBUG_MESSAGE("Sema", "FindLocalVar", Logger().Attr("Parent", Stmt).Attr("Name", Name).End());
    if (Stmt->getKind() == ASTStmtKind::STMT_BLOCK) {
        ASTBlockStmt *Block = (ASTBlockStmt *) Stmt;
        const auto &It = Block->getLocalVars().find(Name);
        if (It != Block->getLocalVars().end()) { // Search into this Block
            return It->getValue();
        } else if (Stmt->getParent()) { // search recursively into Parent Blocks to find the right Var definition
            return FindLocalVar(Stmt->getParent(), Name);
        } else {
            llvm::SmallVector<ASTParam *, 8> Params = Stmt->getFunction()->getParams();
            for (auto &Param : Params) {
                if (Param->getName() == Name) { // Search into ASTParam list
                    return Param;
                }
            }
        }
    } else if (Stmt->getKind() == ASTStmtKind::STMT_IF) {
        return FindLocalVar(Stmt->getParent(), Name);
    } else if (Stmt->getKind() == ASTStmtKind::STMT_SWITCH) {
        return FindLocalVar(Stmt->getParent(), Name);
    }  else if (Stmt->getKind() == ASTStmtKind::STMT_LOOP) {
        return FindLocalVar(Stmt->getParent(), Name);
    }  else if (Stmt->getKind() == ASTStmtKind::STMT_LOOP_IN) {
        return FindLocalVar(Stmt->getParent(), Name);
    }
    return nullptr;
}

SemaSymbols *SemaResolver::AddImportSymbols(llvm::StringRef Name) {
    SemaSymbols *ImportSymbols = S.MapSymbols.lookup(Name);
    Imports.insert(std::make_pair(Name, ImportSymbols));
    return ImportSymbols;
}

