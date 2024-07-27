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
#include "AST/ASTContext.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTClassMethod.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassAttribute.h"
#include "AST/ASTEnum.h"
#include "AST/ASTEnumEntry.h"
#include "AST/ASTType.h"
#include "AST/ASTModule.h"
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

SemaResolver::SemaResolver(Sema &S) : S(S) {

}

/**
 * Take all unreferenced Global Variables from Functions and try to resolve them
 * into all NameSpaces
 * @return
 */
bool SemaResolver::Resolve() {

    // Resolve Modules
    for (auto &Module : S.Context->getModules()) {
        ResolveModule(Module);
        ResolveNameSpace(Module);
        ResolveImports(Module); // resolve Imports with NameSpaces
        ResolveGlobalVars(Module); // resolve Global Vars
        ResolveIdentities(Module);  // resolve Identity attributes and methods
        ResolveFunctions(Module);  // resolve ASTBlock of Body Functions
    }

    // Now all Imports must be read
    for(auto &Import : S.Context->ExternalImports) {
        if (!Import.getValue()->getNameSpace()) {
            S.Diag(Import.getValue()->getLocation(), diag::err_unresolved_import);
        }
    }

    return !S.Diags.hasErrorOccurred();
}

void SemaResolver::ResolveModule(ASTModule *Module) {
    // If no NameSpace is assign set Default NameSpace
    if (Module->NameSpace == nullptr)
        Module->NameSpace = S.Context->DefaultNameSpace;

    for (auto M : S.Context->Modules) {
        if (M->getId() != Module->getId() && M->getName() == Module->getName()) {
            S.Diag(diag::err_sema_module_duplicated) << M->getName();
            return;
        }
    }
}

void SemaResolver::ResolveNameSpace(ASTModule *Module) {
    for (auto &NameSpaceEntry : S.Context->NameSpaces) {
        if (NameSpaceEntry.getValue()->getName() == Module->getNameSpace()->getName()) {
            return;
        }
    }
    S.Context->NameSpaces.insert(std::make_pair(Module->NameSpace->getName(), Module->NameSpace));
}

bool SemaResolver::ResolveNameSpace(ASTModule *Module, ASTIdentifier *&Identifier) {
    ASTImport *Import = FindImport(Module, Identifier->FullName);
    if (Import) {
        Identifier = Import->getNameSpace();
        return true;
    }

    return false;
}

/**
 * Resolve Imports with relative Namespace
 * Sync Un-references from Import to Namespace for next resolving
 * @param Module
 * @return
 */
void SemaResolver::ResolveImports(ASTModule *Module) {

    for (auto &Import : Module->getImports()) {

        // Search Namespace of the Import
        ASTNameSpace *NameSpaceFound = Module->Context->NameSpaces.lookup(Import->getName());

        if (NameSpaceFound) {
            FLY_DEBUG_MESSAGE("Sema", "ResolveImports",
                              "Import=" << Import->getName() << ", NameSpace=" << NameSpaceFound->getName());
            Import->setNameSpace(NameSpaceFound);
        } else {
            // Error: NameSpace not found
            S.Diag(Import->getLocation(), diag::err_namespace_notfound) << Import->getName();
            return;
        }

        if (S.Validator->CheckImport(Module, Import)) {

            // Check if this ASTModule already contains the imports
            for (auto I : Module->Imports) {
                if (I->getName() == Import->getName()) {
                    S.Diag(Import->getLocation(), diag::err_conflict_import) << Import->getName();
                }
            }

            // Check if this ASTModule already contains the name or alias
            llvm::StringRef Id = (Import->getAlias() == nullptr) ? Import->getName()
                                                                 : Import->getAlias()->getName();
            for (auto AI : Module->AliasImports) {
                if (AI->getName() == Id) {
                    S.Diag(Import->getLocation(), diag::err_conflict_import) << Id;
                    return;
                }
            }
        }
    }
}

void SemaResolver::ResolveGlobalVars(ASTModule *Module) {
    for (auto GlobalVar : Module->getGlobalVars()) {
        if (GlobalVar->Expr->getExprKind() != ASTExprKind::EXPR_VALUE) {
            S.Diag(GlobalVar->Expr->getLocation(), diag::err_invalid_gvar_value);
        }

        // Lookup into namespace for public var
        if(GlobalVar->getVisibility() == ASTVisibilityKind::V_PUBLIC ||
           GlobalVar->getVisibility() == ASTVisibilityKind::V_DEFAULT) {
            ASTGlobalVar *LookupVar = Module->NameSpace->getGlobalVars().lookup(GlobalVar->getName());

            if (LookupVar) { // This NameSpace already contains this GlobalVar
                S.Diag(LookupVar->getLocation(), diag::err_duplicate_gvar) << LookupVar->getName();
                return;
            }

            // Add into NameSpace for global resolution
            // Add into Module for local resolution
            auto Pair = std::make_pair(GlobalVar->getName(), GlobalVar);
            // TODO check duplicate in namespace and Module
            Module->GlobalVars.push_back(GlobalVar);
            Module->NameSpace->GlobalVars.insert(Pair).second;
            return;
        }

        // Lookup into Module for private var
        if(GlobalVar->getVisibility() == ASTVisibilityKind::V_PRIVATE) {
            for (auto GB : Module->GlobalVars) {
                if (GlobalVar->getName() == GB->getName()) {
                    S.Diag(GlobalVar->getLocation(), diag::err_duplicate_gvar) << GlobalVar->getName();
                    return;
                }

                // Add into Module for local resolution
                // TODO check duplicate in Module
                Module->GlobalVars.push_back(GlobalVar);
                return;
            }
        }

        GlobalVar->getType()->isIdentity() || ResolveIdentityType(Module, (ASTIdentityType *) GlobalVar->getType());
    }
}

void SemaResolver::ResolveIdentities(ASTModule *Module) {
    if (!Module->Identities.empty()) {
        for (auto Identity : Module->Identities) {

            if (Identity->getTopDefKind() == ASTTopDefKind::DEF_CLASS) {
                ASTClass *Class = (ASTClass *) Identity;

                // Add to NameSpaces
                if (Identity->getVisibility() == ASTVisibilityKind::V_PUBLIC || Identity->getVisibility() == ASTVisibilityKind::V_DEFAULT) {
                    ASTIdentity *LookupClass = Module->NameSpace->getIdentities().lookup(Identity->getName());
                    if (LookupClass) { // This NameSpace already contains this Function
                        S.Diag(LookupClass->Location, diag::err_duplicate_class) << LookupClass->getName();
                        return;
                    }
                    Module->NameSpace->Identities.insert(std::make_pair(Identity->getName(), Identity)).second;
                }

                // Resolve Super Classes
                if (!Class->SuperClasses.empty()) {
                    llvm::StringMap<std::map<uint64_t, llvm::SmallVector<ASTClassMethod *, 4>>> SuperMethods;
                    llvm::StringMap<std::map<uint64_t, llvm::SmallVector<ASTClassMethod *, 4>>> ISuperMethods;
                    for (ASTIdentityType *SuperClassType: Class->SuperClasses) {
                        if (ResolveIdentityType(Module, SuperClassType)) {
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

                                // Collects Super Methods of the Super Classes
                                for (auto &EntryMap: SuperClass->getMethods()) {
                                    const auto &Map = EntryMap.getValue();
                                    auto MapIt = Map.begin();
                                    while (MapIt != Map.end()) {
                                        for (ASTClassMethod *SuperMethod: MapIt->second) {
                                            if (SuperClass->getClassKind() == ASTClassKind::INTERFACE) {
                                                S.Builder->InsertFunction(ISuperMethods, SuperMethod);
                                            } else {
                                                // Insert methods in the Super and if is ok also in the base Class
                                                if (S.Builder->InsertFunction(SuperMethods, SuperMethod)) {
                                                    SmallVector<ASTScope *, 8> Scopes = SuperMethod->getScopes();
                                                    ASTClassMethod *M = S.Builder->CreateClassMethod(SuperMethod->getLocation(),
                                                                                                     *Class,
                                                                                                     SuperMethod->getReturnType(),
                                                                                                     SuperMethod->getName(),
                                                                                                     Scopes);
                                                    M->Params = SuperMethod->Params;
                                                    M->Body = SuperMethod->Body;
                                                    M->DerivedClass = Class;
                                                    S.Builder->InsertFunction(Class->Methods, M);

                                                } else {
                                                    // Multiple Methods Implementations in Super Class need to be re-defined in base class
                                                    // Search if this method is re-defined in the base class
                                                    if (SuperMethod->getVisibility() !=
                                                        ASTVisibilityKind::V_PRIVATE &&
                                                        !S.Builder->ContainsFunction(Class->Methods, SuperMethod)) {
                                                        S.Diag(SuperMethod->getLocation(),
                                                               diag::err_sema_super_class_method_conflict);
                                                        return;
                                                    }
                                                }
                                            }
                                        }
                                        MapIt++;
                                    }
                                }
                            }
                        }
                    }

                    // Check if all abstract methods are implemented
                    for (const auto &EntryMap: ISuperMethods) {
                        const auto &Map = EntryMap.getValue();
                        auto MapIt = Map.begin();
                        while (MapIt != Map.end()) {
                            for (ASTClassMethod *ISuperMethod: MapIt->second) {
                                if (!S.Builder->ContainsFunction(Class->Methods, ISuperMethod)) {
                                    S.Diag(ISuperMethod->getLocation(),
                                           diag::err_sema_method_not_implemented);
                                    return;
                                }
                            }
                            MapIt++;
                        }
                    }
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
                if (!Class->Constructors.empty()) {
                    delete Class->DefaultConstructor;
                }

                // Constructors
                for (auto &IntMap: Class->Constructors) {
                    for (auto &Function: IntMap.second) {

                        // Check duplicates
//                            if ()
//                                S.Diag(Method->getLocation(), diag::err_sema_class_method_redeclare) << Method->getName();
//                                return;
//                            }

                        // Resolve Attribute types
                        for (auto &EntryVar: Class->Attributes) {
                            // TODO
                        }

                        ResolveStmtBlock(Function->Body);
                    }
                }

                // Methods
                for (auto &StrMapEntry: Class->Methods) {
                    for (auto &IntMap: StrMapEntry.getValue()) {
                        for (auto &Method: IntMap.second) {

                            // TODO
                            // Check duplicates
//                            if ()
//                                S.Diag(Method->getLocation(), diag::err_sema_class_method_redeclare) << Method->getName();
//                                return;
//                            }

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

            } else if (Identity->getTopDefKind() == ASTTopDefKind::DEF_ENUM) {
                // TODO
            }
        }
    }
}

void SemaResolver::ResolveFunctions(ASTModule *Module) {
    for (auto &StrMapEntry : Module->Functions) {
        for (auto &IntMap : StrMapEntry.getValue()) {
            for (auto &Function : IntMap.second) {

                // TODO
                // Lookup into namespace for public var
//                if(Function->getVisibility() == ASTVisibilityKind::V_PUBLIC ||
//                   Function->getVisibility() == ASTVisibilityKind::V_DEFAULT) {
//
//                    // Add into NameSpace for global resolution
//                    // Add into Module for local resolution
//                    return InsertFunction(Module->NameSpace->Functions, Function) &&
//                           InsertFunction(Module->Functions, Function);
//                }
//
//                // Lookup into Module for private var
//                if (Function->getVisibility() == ASTVisibilityKind::V_PRIVATE) {
//
//                    // Add into Module for local resolution
//                    return InsertFunction(Module->Functions, Function);
//                }

                ResolveStmtBlock(Function->Body);
            }
        }
    }
}

void SemaResolver::ResolveScopes(llvm::SmallVector<ASTScope *, 8> Scopes, ASTGlobalVar *GlobalVar) {
    for (auto &Scope : Scopes) {
        // TODO
    }
}

void SemaResolver::ResolveScopes(llvm::SmallVector<ASTScope *, 8> Scopes, ASTFunction *Function) {
    for (auto &Scope : Scopes) {
        // TODO
    }
}

void SemaResolver::ResolveScopes(llvm::SmallVector<ASTScope *, 8> Scopes, ASTClass *Class) {
    for (auto &Scope : Scopes) {
        // TODO
    }
}

void SemaResolver::ResolveScopes(llvm::SmallVector<ASTScope *, 8> Scopes, ASTEnum *Enum) {
    for (auto &Scope : Scopes) {
        // TODO
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

bool SemaResolver::ResolveParentIdentifier(ASTStmt *Stmt, ASTIdentifier *&Identifier) {
    const auto &Module = FindModule(Stmt->getFunction());

    if (Identifier->getParent()) {
        if (ResolveParentIdentifier(Stmt, Identifier->Parent)) {

            // Do these in the parents different from first
            switch (Identifier->getIdKind()) {

                case ASTIdentifierKind::REF_NAMESPACE: {
                    if (Identifier->getParent()->getIdKind() != ASTIdentifierKind::REF_NAMESPACE) {
                        // Error:
                        S.Diag(Identifier->getLocation(), diag::err_sema_resolve_identifier);
                        return false;
                    }
                    break;
                }

                case ASTIdentifierKind::REF_TYPE: {
                    ASTIdentityType *IdentityType = (ASTIdentityType *) Identifier;
                    if (Identifier->getParent()->getIdKind() == ASTIdentifierKind::REF_NAMESPACE) {
                        ResolveIdentityType(Module, IdentityType);
                    } else {
                        // Error:
                        S.Diag(Identifier->getLocation(), diag::err_sema_resolve_identifier);
                        return false;
                    }
                    break;
                }

                    // Instance
                case ASTIdentifierKind::REF_CALL: // NameSpace.call().call() or call().call()
                    return ResolveCallWithParent(Stmt, (ASTCall *) Identifier);

                case ASTIdentifierKind::REF_VAR: // NameSpace.call().var or call().var
                    return ResolveVarRefWithParent((ASTVarRef *) Identifier);

                case ASTIdentifierKind::REF_UNDEF: // Error: identifier not resolved
                    assert(false && "Unexpected Identifier Kind");
            }
        }
    } else { // Do these only on first Parent identifier

        // Check if Identifier is a Call
        if (Identifier->isCall()) {
            return ResolveCallNoParent(Stmt, (ASTCall *) Identifier);
        }

        // Check if Identifier is a Var
        ASTVar *Var = ResolveVarRefNoParent(Stmt, Identifier->getName());
        if (Var) {
            Identifier = S.Builder->CreateVarRef(Identifier);
            ((ASTVarRef *) Identifier)->Def = Var;
            return true;
        }

        // Check if Identifier is an IdentityType
        ASTIdentityType *IdentityType = FindIdentityType(Identifier->getName(), Module->getNameSpace());
        if (IdentityType) {
            Identifier = IdentityType; // Take From Types
            return true;
        }

        // Check if Identifier is a NameSpace
        if (ResolveNameSpace(Module, Identifier)) {
            return true;
        }
    }

    S.Diag(Identifier->getLocation(), diag::err_sema_resolve_identifier);
    return false;
}

bool SemaResolver::ResolveIdentityType(ASTModule *Module, ASTIdentityType *IdentityType) {
    // Resolve Identifier
    if (IdentityType->getDef() == nullptr) {

        if (IdentityType->getParent() == nullptr) {
            ASTIdentityType *Type = FindIdentityType(IdentityType->getName(), Module->getNameSpace());
            if (Type) {
                IdentityType->Def = Type->getDef();
                IdentityType->IdentityTypeKind = Type->getIdentityTypeKind();
            }
        } else if (ResolveNameSpace(Module,IdentityType->Parent)) {
            ASTNameSpace *NameSpace = (ASTNameSpace *) IdentityType->getParent();
            ASTIdentityType *Type = FindIdentityType(IdentityType->getName(), NameSpace);
            if (Type) {
                IdentityType->Def = Type->getDef();
                IdentityType->IdentityTypeKind = Type->getIdentityTypeKind();
            }
        } else {
            S.Diag(IdentityType->getLocation(), diag::err_sema_resolve_identifier);
            return false;
        }
    }

    if (!IdentityType->Def) {
        S.Diag(IdentityType->getLocation(), diag::err_unref_type);
        return false;
    }

    return true;
}

/**
 * Resolve a VarRef with its declaration
 * @param VarRef
 * @return true if no error occurs, otherwise false
 */
bool SemaResolver::ResolveVarRef(ASTStmt *Stmt, ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveVarRefWithParent", Logger().Attr("VarRef", VarRef).End());
    
    if (!VarRef->Def) {
        if (VarRef->getParent() == nullptr) {
            VarRef->Def = ResolveVarRefNoParent(Stmt, VarRef->getName());
        } else {
            ResolveParentIdentifier(Stmt, VarRef->Parent) && ResolveVarRefWithParent((ASTVarRef *) VarRef);
        }
    }

    // VarRef not found in Module, namespace and Module imports
    if (VarRef->getDef() == nullptr) {
        S.Diag(VarRef->getLocation(), diag::err_unref_var) << VarRef->Name;
        return false;
    }

    return true;
}

ASTVar *SemaResolver::ResolveVarRefNoParent(ASTStmt *Stmt, llvm::StringRef Name) {

    // Search for LocalVar
    ASTVar *Var = FindLocalVar(Stmt, Name);

    if (Var == nullptr) {
        ASTFunctionBase *Function = Stmt->getFunction();
        const auto &Module = FindModule(Function);

        // Search for Class Vars if Var is Class Method
        if (Function->getKind() == ASTFunctionKind::CLASS_METHOD)
            for (auto &Attribute : ((ASTClassMethod *) Function)->getClass()->Attributes) {
                if (Attribute->getName() == Name) {
                    Var = Attribute;
                }
            }

        // Search for GlobalVars in Module
        if (Var == nullptr)
            for (auto &GlobalVar : Module->GlobalVars)
                if (GlobalVar->getName() == Name)
                    Var = GlobalVar;

        // Search for GlobalVars in NameSpace
        if (Var == nullptr)
            Var = Module->NameSpace->GlobalVars.lookup(Name);
    }

    return Var;
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
}

bool SemaResolver::ResolveVarRefWithParent(ASTVarRef *VarRef) {
    switch (VarRef->getParent()->getIdKind()) {

        case ASTIdentifierKind::REF_NAMESPACE: { // Namespace.globalVar
            ASTNameSpace *NameSpace = (ASTNameSpace *) VarRef->getParent();
            VarRef->Def = NameSpace->GlobalVars.lookup(VarRef->getName());
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
        case ASTIdentifierKind::REF_UNDEF:
            assert(false && "Unexpected Identifier Kind");
    }

    return VarRef->Def;
}

bool SemaResolver::ResolveCall(ASTStmt *Stmt, ASTCall *Call) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveCall", Logger().Attr("Call", Call).End());

    if (!Call->Def) {

        if (Call->getParent() == nullptr) {
            ResolveCallNoParent(Stmt, Call);
        } else {
            ResolveParentIdentifier(Stmt, Call->Parent) && ResolveCallWithParent(Stmt, Call);
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

bool SemaResolver::ResolveCall(ASTStmt *Stmt, ASTCall *Call, ASTNameSpace *NameSpace) {

    // NameSpace.func()
    bool Success = ResolveCall(Stmt, Call, NameSpace->Functions);

    // NameSpace.constructor()
    if (!Success) {
        ASTIdentity *Identity = FindIdentity(Call->getName(), NameSpace);
        Identity != nullptr && Identity->getTopDefKind() == ASTTopDefKind::DEF_CLASS &&
        ResolveCall(Stmt, Call, ((ASTClass *) Identity)->Constructors);
    }

    return Call->Def;
}

bool SemaResolver::ResolveCallNoParent(ASTStmt *Stmt, ASTCall *Call) {
    const auto &Module = FindModule(Stmt->getFunction());

    // func()
    bool Success = ResolveCall(Stmt, Call, Module->Functions) ||
                   ResolveCall(Stmt, Call, Module->Context->DefaultNameSpace->Functions) ||
                   ResolveCall(Stmt, Call, Module->getNameSpace()->Functions);

    // constructor()
    if (!Success) {
        ASTIdentity *Identity = FindIdentity(Call->getName(), Module->getNameSpace());
        Identity != nullptr && Identity->getTopDefKind() == ASTTopDefKind::DEF_CLASS &&
        ResolveCall(Stmt, Call, ((ASTClass *) Identity)->Constructors);
    }

    return Call->Def;
}

bool SemaResolver::ResolveCallWithParent(ASTStmt *Stmt, ASTCall *Call) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveCall", Logger().Attr("Call", Call).End());
        
    ASTFunctionBase *Function = Stmt->getFunction();
    const auto &Module = FindModule(Function);

    switch (Call->getParent()->getIdKind()) {

        case ASTIdentifierKind::REF_NAMESPACE: {
            ASTNameSpace *NameSpace = (ASTNameSpace *) Call->getParent();
            return ResolveCall(Stmt, Call, NameSpace);
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
        case ASTIdentifierKind::REF_UNDEF:
            assert(false && "Unexpected Identifier Kind");
    }

    if (Call->Def == nullptr) {
        S.Diag(Call->getLocation(), diag::err_unref_call);
        return false;
    }

    return Call->Def;
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
 * Resolve Expr contents
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

ASTNameSpace *SemaResolver::FindNameSpace(llvm::StringRef Name) const {
    FLY_DEBUG_MESSAGE("Sema", "FindNameSpace", "Name=" << Name);
    ASTNameSpace *NameSpace = S.Context->NameSpaces.lookup(Name);
    if (!NameSpace) {
        S.Diag(diag::err_unref_namespace) << Name;
    }
    return NameSpace;
}

ASTModule *SemaResolver::FindModule(ASTFunctionBase *FunctionBase) const {
    FLY_DEBUG_MESSAGE("Sema", "FindModule", Logger().Attr("FunctionBase", FunctionBase).End());
    if (FunctionBase->getKind() == ASTFunctionKind::FUNCTION) {
        return ((ASTFunction *) FunctionBase)->getModule();
    } else if (FunctionBase->getKind() == ASTFunctionKind::CLASS_METHOD) {
        return ((ASTClassMethod *) FunctionBase)->getClass()->getModule();
    } else {
        assert("Unknown Function Kind");
        return nullptr;
    }
}

ASTModule *SemaResolver::FindModule(llvm::StringRef Name, ASTNameSpace *NameSpace) const {
    FLY_DEBUG_MESSAGE("Sema", "FindModule", Logger().Attr("Name", Name).Attr("NameSpace", NameSpace).End());
    ASTModule *Module = NameSpace->Modules.lookup(Name);
    if (!Module) {
        S.Diag(diag::err_unref_module) << Name;
    }
    return Module;
}

ASTImport *SemaResolver:: FindImport(ASTModule *Module, llvm::StringRef Name) {
    // Search into Module imports
    ASTImport *Import = nullptr;
    for (auto &I : Module->Imports)
        if (I->getName() == Name)
            Import = I;
    if (Import == nullptr)
        for (auto &AliasImport :Module->AliasImports)
            if (AliasImport->getName() == Name)
                Import = AliasImport;
    return Import;
}

ASTIdentityType *SemaResolver::FindIdentityType(llvm::StringRef Name, ASTNameSpace *NameSpace) const {
    FLY_DEBUG_MESSAGE("Sema", "FindIdentityType", Logger().Attr("Name", Name).Attr("NameSpace", NameSpace).End());
    ASTIdentityType *IdentityType = NameSpace->getIdentityTypes().lookup(Name);
    return IdentityType;
}

ASTIdentity *SemaResolver::FindIdentity(llvm::StringRef Name, ASTNameSpace *NameSpace) const {
    FLY_DEBUG_MESSAGE("Sema", "FindIdentity", Logger().Attr("Name", Name).Attr("NameSpace", NameSpace).End());
    ASTIdentity *Identity = NameSpace->Identities.lookup(Name);
    return Identity;
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
