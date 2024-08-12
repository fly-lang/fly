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
            ResolveVarRefChain(Stmt->Parent, VarStmt->getVarRef());
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
                Success = ResolveVarRefChain(Stmt->Parent, HandlStmt->ErrorHandlerRef);
            return Success && ResolveStmt(HandlStmt->Handle);
        }
        case ASTStmtKind::STMT_DELETE: {
            ASTDeleteStmt *DeleteStmt = (ASTDeleteStmt *) Stmt;
            return ResolveVarRefChain(Stmt->Parent, DeleteStmt->VarRef);
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
    bool Success = ResolveVarRefChain(SwitchStmt->getParent(), SwitchStmt->getVarRef()) &&
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
    return ResolveVarRefChain(LoopInStmt->Parent, LoopInStmt->VarRef) && ResolveStmtBlock(LoopInStmt->Block);
}

bool SemaResolver::ResolveIdentityType(ASTIdentityType *IdentityType) {
    // Resolve only if not resolved yet
    if (!IdentityType->isResolved()) {

        // Identity can have only NameSpace as Parent: NameSpace.Type
        ASTIdentity *Identity;
        if (IdentityType->getParent()) {

            // Resolve by Imports
            SemaSymbols *NS = Imports.lookup(IdentityType->getParent()->getFullName());
            Identity = FindIdentity(IdentityType->getName(), NS);
        } else {
            // Resolve in current NameSpace
            Identity = FindIdentity(IdentityType->getName(), NameSpace);

            if (Identity == nullptr)
                // Resolve in Default NameSpace
                Identity = FindIdentity(IdentityType->getName(), S.DefaultSymbols);
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
}

bool SemaResolver::ResolveIdentifier(SemaSymbols *NS, ASTIdentifier *Identifier) {
    if (NS == nullptr) {
        return false;
    }
    
    if (Identifier == nullptr) {
        return false;
    }

    bool Success = false;
    if (Identifier->isCall()) { // Call cannot be undefined
        ASTCall *Call = (ASTCall *) Identifier;
        
        // NameSpace.Call()... or NameSpace.ConstructorCall()...
        Success = ResolveCall(NS, Call);
        if (!Success) {

            // Constructor Call
            ASTIdentity *Identity = FindIdentity(Call->getName(), NS);
            if (Identity)
                Success = ResolveCall(Identity, Call);
        }

        // Resolve Child
        if (Success && Call->getChild()) {
            ASTType *Type = Call->getDef()->getReturnType();
            if (!Type->isIdentity()) {
                // Error: cannot access to not identity var
            }
            if (ResolveIdentityType((ASTIdentityType *) Type))
                return ResolveIdentifier(((ASTIdentityType *) Type)->getDef(), Call->getChild());
        }
    } else {

        // NameSpace.Type...Call() or NameSpace.Type...Var
        ASTIdentity *Identity = FindIdentity(Identifier->getName(), NS);
        if (Identity) {
            if (Identity->getTopDefKind() == ASTTopDefKind::DEF_CLASS)
                Identifier = S.Builder->CreateClassType(Identifier);
            else if (Identity->getTopDefKind() == ASTTopDefKind::DEF_ENUM)
                Identifier = S.Builder->CreateEnumType(Identifier);
            ((ASTIdentityType *) Identifier)->Def = Identity;

            // Resolve Child
            if (Identifier->getChild()) {
                if (Identifier->getChild()->isCall()) {
                    return ResolveCall(Identity, (ASTCall *) Identifier->getChild());
                } else {
                    return ResolveVarRef(Identity, (ASTVarRef *) Identifier->getChild());
                }
            }

            return true;
        }

        // NameSpace.GlobalVar...Call() or NameSpace.GlobalVar...Var
        return ResolveVarRef(NS, (ASTVarRef *) Identifier);
    }

    return Success;
}

bool SemaResolver::ResolveIdentifier(ASTIdentity *Identity, ASTIdentifier *Identifier) {
    if (Identity == nullptr) {
        return false;
    }

    if (Identifier == nullptr) {
        return false;
    }
    
    if (Identifier->isCall()) { // Call cannot be undefined
        return ResolveCall(Identity, (ASTCall *) Identifier);
    } else {
        return ResolveVarRef(Identity, (ASTVarRef *) Identifier);
    }
}

bool SemaResolver::ResolveIdentifier(ASTStmt *Stmt, ASTIdentifier *Identifier) {
    if (Stmt == nullptr) {
        return false;
    }

    if (Identifier == nullptr) {
        return false;
    }

    bool Success = false;
    if (Identifier->isCall()) { // Call cannot be undefined
        ASTCall *Call = (ASTCall *) Identifier;
        
        // Call()
        ASTFunctionBase *Function = FindFunction(Call, NameSpace);
        if (Function == nullptr) {
            Function = FindFunction(Call, S.DefaultSymbols);
        }
        
        // method()
        if (Function == nullptr) {
            if (Stmt->getFunction()->getKind() == ASTFunctionKind::CLASS_METHOD) {
                ASTClass *Class = ((ASTClassMethod *) Stmt->getFunction())->getClass();
                Function = FindMethod(Call, Class);
            }
        }

        if (Function) {
            Call->Def = Function;
        } else {

            // ConstructorCall()
            ASTIdentity *Identity = FindIdentity(Call->getName(), NameSpace);
            if (Identity == nullptr) {
                Identity = FindIdentity(Call->getName(), S.DefaultSymbols);
                if (Identity)
                    Success = ResolveCall(Identity, Call);
            }
        }
        
        // Resolve child
        if (Call->Def && Call->getChild()) {
            ASTType *Type = Call->getDef()->getReturnType();
            if (!Type->isIdentity()) {
                // Error: cannot access to not identity var
            }
            if (ResolveIdentityType((ASTIdentityType *) Type))
                return ResolveIdentifier(((ASTIdentityType *) Type)->getDef(), Call->getChild());
        }
        
    } else {
        ASTVarRef *VarRef = (ASTVarRef *) Identifier;
        
        // Check in LocalVar
        // LocalVar.Var or ClassAttribute.Var
        ASTVar *Var = FindLocalVar(Stmt, Identifier->getName());

        if (Var == nullptr) {
            ASTFunctionBase *Function = Stmt->getFunction();

            // Search for Class Vars if Var is Class Method
            if (Function->getKind() == ASTFunctionKind::CLASS_METHOD)
                for (auto &Attribute : ((ASTClassMethod *) Function)->getClass()->Attributes) {
                    if (Attribute->getName() == Identifier->getName()) {
                        Var = Attribute;
                    }
                }
        }

        VarRef->Def = Var;
    }
}

bool SemaResolver::ResolveVarRef(SemaSymbols *NS, ASTVarRef *VarRef) {
    ASTGlobalVar *GlobalVar = FindGlobalVar(VarRef->getName(), NS);
    if (GlobalVar) {
        VarRef->Def = GlobalVar;
        return true;
    }
    return false;
}

bool SemaResolver::ResolveVarRef(ASTIdentity *Identity, ASTVarRef *VarRef) {
    if (Identity->getTopDefKind() == ASTTopDefKind::DEF_CLASS) {
        for (auto &Attribute : ((ASTClass *) Identity)->Attributes) {
            if (Attribute->getName() == VarRef->getName()) {
                return Attribute;
            }
        }
    } else if (Identity->getTopDefKind() == ASTTopDefKind::DEF_ENUM) {
        for (auto &EnumEntry : ((ASTEnum *) Identity)->Entries) {
            if (EnumEntry->getName() == VarRef->getName()) {
                return EnumEntry;
            }
        }
    } else {
        assert(false && "IdentityType unknown");
    }
    return false;
}

/**
 * ResolveModule a VarRef with its declaration
 * @param VarRef
 * @return true if no error occurs, otherwise false
 */
bool SemaResolver::ResolveVarRefChain(ASTStmt *Stmt, ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveVarRefWithParent", Logger().Attr("VarRef", VarRef).End());
    
    if (!VarRef->Def) {

        ASTIdentifier *Current = nullptr;
        SemaSymbols *NS = FindNameSpace(VarRef, Current);

        return ResolveIdentifier(NS, Current) || // Resolve in NameSpace: Type, Function, GlobalVar
               ResolveIdentifier(Stmt, Current) || // Resolve in statements as LocalVar
               ResolveIdentifier(NameSpace, Current) ||
               ResolveIdentifier(S.DefaultSymbols, Current); // Default NameSpace, Class Method or Attribute, LocalVar
    }

    // VarRef not found in Module, namespace and Module imports
    if (VarRef->getDef() == nullptr) {
        S.Diag(VarRef->getLocation(), diag::err_unref_var) << VarRef->Name;
        return false;
    }

    return true;
}

bool SemaResolver::ResolveCall(SemaSymbols *NS, ASTCall *Call) {
    ASTFunction *Function = FindFunction(Call, NS);
    if (Function) {
        Call->Def = Function;
        return true;
    }
    return false;
}

bool SemaResolver::ResolveCall(ASTIdentity *Identity, ASTCall *Call) {
    if (Identity->getTopDefKind() == ASTTopDefKind::DEF_CLASS) {
        ASTClassMethod *Method = FindClassMethod(Call, (ASTClass *) Identity);
        Call->Def = Method;
    } else {
        S.Diag(Call->getLocation(), diag::err_sema_call_enum);
    }

    return false;
}

bool SemaResolver::ResolveCallChain(ASTStmt *Stmt, ASTCall *Call) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveCallChain", Logger().Attr("Call", Call).End());

    if (!Call->Def) {

        if (Call->getParent()) {
            ASTIdentifier *Current = nullptr;
            SemaSymbols *NS = FindNameSpace(Call, Current);

            return ResolveIdentifier(NS, Current) || // Resolve in NameSpace: Type, Function, GlobalVar
                   ResolveIdentifier(NameSpace, Current) ||
                   ResolveIdentifier(S.DefaultSymbols, Current); // Default NameSpace, Class Method or Attribute, LocalVar
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

bool SemaResolver::ResolveArg(ASTStmt *Stmt, ASTArg *Arg, ASTParam *Param) {
    Arg->Def = Param;
    // Resolve Arg Expr on first
    ASTArg *Arg = Call->getArgs()[i];
    ASTParam *Param = Function->getParams()[i];
    Success &= ResolveArg(Stmt, Arg, Param);
    
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
            if (ResolveVarRefChain(Stmt, VarRef)) {
                Expr->Type = VarRef->getDef()->getType();
                Success = true;
                break;
            } else {
                return false;
            }
        }
        case ASTExprKind::EXPR_CALL: {
            ASTCall *Call = ((ASTCallExpr *)Expr)->getCall();
            if (ResolveCallChain(Stmt, Call)) {
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

SemaSymbols *SemaResolver:: FindNameSpace(ASTIdentifier *Identifier, ASTIdentifier *&Current) const {
    // Find NameSpace by iterating parents
    // one.two.three.four
    // four.Parent, three.Parent, two.Parent, one.Parent
    SemaSymbols *NS = nullptr;
    Current = Identifier;
    while (Current->getParent()) {
        // Check from Imports
        NS = Imports.lookup(Current->getParent()->getFullName());
        if (NS) {
            Current->Parent = S.Builder->CreateNameSpace(Current->getParent());
            break;
        }
        Current = Current->getParent();
    }
    
    return NS;
}

ASTGlobalVar *SemaResolver::FindGlobalVar(llvm::StringRef Name, SemaSymbols *Symbols) const {
    FLY_DEBUG_MESSAGE("Sema", "FindGlobalVar", Logger().Attr("Name", Name).End());
    return Symbols->GlobalVars.lookup(Name);
}

ASTIdentity *SemaResolver::FindIdentity(llvm::StringRef Name, SemaSymbols *Symbols) const {
    FLY_DEBUG_MESSAGE("Sema", "FindIdentity", Logger().Attr("Name", Name).End());
    return Symbols->getIdentities().lookup(Name);
}

ASTFunction *SemaResolver::FindFunction(ASTCall *Call, SemaSymbols *Symbols) const {
    FLY_DEBUG_MESSAGE("Sema", "FindIdentity", Logger().Attr("IdentityType", (ASTIdentifier *) Call).End());
    return FindFunction(Call, Symbols->getFunctions());
}

ASTClassMethod *SemaResolver::FindClassMethod(ASTCall *Call, ASTClass *Class) const {
    FLY_DEBUG_MESSAGE("Sema", "FindIdentity", Logger().Attr("IdentityType", (ASTIdentifier *) Call).End());
    auto Methods = NameSpace->ClassMethods.lookup(Class->getName());
    if (!Methods.empty()) {
        return FindFunction(Call, Methods);
    }

    return nullptr;
}

template <typename T>
T *SemaResolver::FindFunction(ASTCall *Call, llvm::StringMap<std::map<uint64_t, llvm::SmallVector<T *, 4>>> Functions) const {
    for (auto &StrMapIt : Functions) {
        const std::map<uint64_t, llvm::SmallVector<T *, 4>> &IntMap = StrMapIt.getValue();
        const auto &IntMapIt = IntMap.find(Call->getArgs().size());
        if (IntMapIt != IntMap.end()) {
            for (T *Function : IntMapIt->second) {
                if (Function->getParams().size() == Call->getArgs().size()) {
                    return Function;
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

ASTClassMethod * SemaResolver::FindMethod(ASTCall *Call, ASTClass *Class) {
    const llvm::StringMap<std::map<uint64_t, llvm::SmallVector<ASTClassMethod *, 4>>> &Methods = 
            NameSpace->ClassMethods.lookup(Class->getName());
    return (ASTClassMethod *) FindFunction(Call, Methods);
}

