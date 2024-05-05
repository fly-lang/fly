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
#include "AST/ASTNode.h"
#include "AST/ASTIfBlock.h"
#include "AST/ASTImport.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTParam.h"
#include "AST/ASTSwitchBlock.h"
#include "AST/ASTLoopBlock.h"
#include "AST/ASTBlock.h"
#include "AST/ASTValue.h"
#include "AST/ASTVar.h"
#include "AST/ASTVarStmt.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTExprStmt.h"
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
    bool Success = true;

    // Resolve Nodes
    for (auto &NodeEntry : S.Context->getNodes()) {
        auto &Node = NodeEntry.getValue();
        Success &= ResolveImports(Node); // resolve Imports with NameSpaces
        Success &= ResolveGlobalVars(Node); // resolve Global Vars
        Success &= ResolveIdentities(Node);  // resolve Identity attributes and methods
        Success &= ResolveFunctions(Node);  // resolve ASTBlock of Body Functions
    }

    // Now all Imports must be read
    for(auto &Import : S.Context->ExternalImports) {
        if (!Import.getValue()->getNameSpace()) {
            S.Diag(Import.getValue()->getLocation(), diag::err_unresolved_import);
            return false;
        }
    }

    return Success;
}

bool SemaResolver::ResolveNameSpace(ASTNode *Node, ASTIdentifier *&Identifier) {
    ASTImport *Import = FindImport(Node, Identifier->FullName);
    if (Import) {
        Identifier = Import->getNameSpace();
        return true;
    }

    return false;
}

/**
 * Resolve Imports with relative Namespace
 * Sync Un-references from Import to Namespace for next resolving
 * @param Node
 * @return
 */
bool SemaResolver::ResolveImports(ASTNode *Node) {
    bool Success = true;

    for (auto &ImportEntry : Node->getImports()) {

        // Search Namespace of the Import
        auto &Import = ImportEntry.getValue();
        ASTNameSpace *NameSpaceFound = Node->Context->NameSpaces.lookup(Import->getName());

        if (NameSpaceFound) {
            FLY_DEBUG_MESSAGE("Sema", "ResolveImports",
                              "Import=" << Import->getName() << ", NameSpace=" << NameSpaceFound->getName());
            Import->setNameSpace(NameSpaceFound);
        } else {
            // Error: NameSpace not found
            Success = false;
            S.Diag(Import->getLocation(), diag::err_namespace_notfound) << Import->getName();
        }
    }

    return Success;
}

bool SemaResolver::ResolveGlobalVars(ASTNode *Node) {
    bool Success = true;

    for (auto &GlobalVarEntry : Node->getGlobalVars()) {
        ASTGlobalVar *GlobalVar = GlobalVarEntry.getValue();
        Success = !GlobalVar->getType()->isIdentity() || ResolveIdentityType(Node, (ASTIdentityType *) GlobalVar->getType());
    }

    return Success;
}

bool SemaResolver::ResolveIdentities(ASTNode *Node) {
    bool Success = true;
    if (Node->Identity) {

        if (Node->Identity->getTopDefKind() == ASTTopDefKind::DEF_CLASS) {
            ASTClass *Class = (ASTClass *) Node->Identity;

            // Resolve Super Classes
            if (!Class->SuperClasses.empty()) {
                llvm::StringMap<std::map<uint64_t, llvm::SmallVector<ASTClassMethod *, 4>>> SuperMethods;
                llvm::StringMap<std::map<uint64_t, llvm::SmallVector<ASTClassMethod *, 4>>> ISuperMethods;
                for (ASTIdentityType *SuperClassType: Class->SuperClasses) {
                    if (ResolveIdentityType(Node, SuperClassType)) {
                        ASTClass *SuperClass = (ASTClass *) SuperClassType->getDef();

                        // Struct: Resolve Var in Super Classes
                        if (SuperClass->getClassKind() == ASTClassKind::STRUCT) {

                            // Interface cannot extend a Struct
                            if (Class->getClassKind() == ASTClassKind::INTERFACE) {
                                S.Diag(SuperClassType->getLocation(), diag::err_sema_interface_ext_struct);
                                return false;
                            }

                            // Add Vars to the Struct
                            for (auto &EntryVar: SuperClass->getVars()) {
                                ASTClassAttribute *&SuperVar = EntryVar.getValue();

                                // Check Var already exists and type conflicts in Super Vars
                                ASTClassAttribute *ClassVar = Class->Vars.lookup(EntryVar.getKey());
                                if (ClassVar == nullptr) {
                                    Class->Vars.insert(std::make_pair(SuperVar->getName(), SuperVar));
                                } else if (SuperVar->getType() != ClassVar->getType()) {
                                    S.Diag(ClassVar->getLocation(), diag::err_sema_super_struct_var_conflict);
                                    return false;
                                }
                            }
                        }

                        // Interface cannot extend a Class
                        if (Class->getClassKind() == ASTClassKind::INTERFACE &&
                            SuperClass->getClassKind() == ASTClassKind::CLASS) {
                            S.Diag(SuperClassType->getLocation(), diag::err_sema_interface_ext_class);
                            return false;
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
                                                ASTClassMethod *M = S.Builder->CreateClassMethod(SuperMethod->getLocation(),
                                                                                                 SuperMethod->getType(),
                                                                                                 SuperMethod->getName(),
                                                                                                 SuperMethod->getScopes());
                                                M->Params = SuperMethod->Params;
                                                M->Body = SuperMethod->Body;
                                                M->DerivedClass = Class;
                                                S.Builder->InsertFunction(Class->Methods, M);

                                            } else {
                                                // Multiple Methods Implementations in Super Class need to be re-defined in base class
                                                // Search if this method is re-defined in the base class
                                                if (SuperMethod->Scopes->getVisibility() !=
                                                    ASTVisibilityKind::V_PRIVATE &&
                                                    !S.Builder->ContainsFunction(Class->Methods, SuperMethod)) {
                                                    S.Diag(SuperMethod->getLocation(),
                                                           diag::err_sema_super_class_method_conflict);
                                                    return false;
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
                                return false;
                            }
                        }
                        MapIt++;
                    }
                }
            }

            // Constructors
            for (auto &IntMap: Class->Constructors) {
                for (auto &Function: IntMap.second) {

                    // Check Class vars for each Constructor
                    for (auto &EntryVar: Class->Vars) {

                        // FIXME: Check if Method already contains this var name as LocalVar
//                    if (!S.Validator->CheckDuplicateLocalVars(Function->Body, EntryVar.getKey())) {
//                        return false;
//                    }
                    }

                    Success &= ResolveBlock(Function->Body);
                }
            }

            // Methods
            for (auto &StrMapEntry: Class->Methods) {
                for (auto &IntMap: StrMapEntry.getValue()) {
                    for (auto &Method: IntMap.second) {

                        // Add Class vars for each Method
                        for (auto &EntryVar: Class->Vars) {

                            // Check if Method already contains this var name as LocalVar
                            if (!S.Validator->CheckDuplicateLocalVars(Method->Body, EntryVar.getKey())) {
                                return false;
                            }
                        }

                        if (!Method->isAbstract()) {
                            Success &= ResolveBlock(Method->Body); // FIXME check if already resolved
                        }
                    }
                }
            }
        } else if (Node->Identity->getTopDefKind() == ASTTopDefKind::DEF_ENUM) {
            // TODO
        }
    }
    return Success;
}

bool SemaResolver::ResolveFunctions(ASTNode *Node) {
    bool Success = true;
    for (auto &StrMapEntry : Node->Functions) {
        for (auto &IntMap : StrMapEntry.getValue()) {
            for (auto &Function : IntMap.second) {
                Success &= ResolveBlock(Function->Body);
            }
        }
    }
    return Success;
}

bool SemaResolver::ResolveStmt(ASTStmt *Stmt) {
    bool Success = false;

    if (Stmt->getKind() == ASTStmtKind::STMT_BLOCK) {
        Success = ResolveBlock((ASTBlock *) Stmt);
    } else {
        ASTBlock *Block = (ASTBlock *) Stmt->getParent(); // FIXME check if parent is not null
        if (Stmt->getKind() == ASTStmtKind::STMT_EXPR) {
            Success &= ResolveExpr(Block, ((ASTExprStmt *) Stmt)->getExpr());
        } else if (Stmt->getKind() == ASTStmtKind::STMT_RETURN) {
            Success &= ResolveExpr(Block, ((ASTReturnStmt *) Stmt)->getExpr());
        } else if (Stmt->getKind() == ASTStmtKind::STMT_VAR) {
            ASTVarStmt *VarStmt = ((ASTVarStmt *) Stmt);

            // Error: Expr cannot be null
            if (!VarStmt->getVarRef()->getDef()) {
                S.Diag(VarStmt->getLocation(), diag::err_var_undefined) << VarStmt->getVarRef()->getName();
                return false;
            }

            // Take Node
            ASTVar *Var = VarStmt->getVarRef()->getDef();
            if (Var->getVarKind() == ASTVarKind::VAR_LOCAL) {
                ASTLocalVar *LocalVar = (ASTLocalVar *) Var;

                if (!LocalVar->isInitialized()) {
                    S.Diag(VarStmt->getLocation(), diag::err_sema_uninit_var) << VarStmt->getVarRef()->getName();
                    return false;
                }

                const auto &Node = FindNode(Block->getTop());
                Success &= VarStmt && !LocalVar->getType()->isIdentity() ||
                           ResolveIdentityType(Node, (ASTIdentityType *) VarStmt->getVarRef()->getDef()->getType());

                if (VarStmt->getExpr())
                    Success &= ResolveExpr(Block, VarStmt->getExpr());
            }

            Success &= ResolveVarRef(Block, VarStmt->getVarRef()) &&
                       ResolveExpr(Block, VarStmt->getExpr());

            // Check Expr Type if it needs to be converted to Stmt Type
            Success &= S.Validator->CheckConvertibleTypes(VarStmt->getExpr()->getType(), Var->getType());
        } else if (Stmt->getKind() == ASTStmtKind::STMT_DELETE) {
            // TODO
        } else {
            // ASTStmtKind::STMT_BREAK
            // ASTStmtKind::STMT_CONTINUE
        }
    }
    return Success;
}

bool SemaResolver::ResolveBlock(ASTBlock *Block) {
    bool Success = false;
    switch (Block->getBlockKind()) {

        case ASTBlockKind::BLOCK_IF:
            Success &= ResolveIfBlock((ASTIfBlock *) Block);
            break;
        case ASTBlockKind::BLOCK_SWITCH:
            Success &= ResolveSwitchBlock((ASTSwitchBlock *) Block);
            break;
        case ASTBlockKind::BLOCK_LOOP:
            Success &= ResolveLoopBlock((ASTLoopBlock *) Block);
            break;
        case ASTBlockKind::BLOCK:
            for (ASTStmt *Stmt : Block->getContent()) {
                Success &= ResolveStmt(Stmt);
            }
            break;
    }

    for (auto &LocalVar : Block->LocalVars) {
        if (!LocalVar.second->isInitialized())
            S.Diag(LocalVar.getValue()->getLocation(), diag::err_sema_uninit_var) << LocalVar.getValue()->getName();
    }
    return false;

    return Success;
}

bool SemaResolver::ResolveIfBlock(ASTIfBlock *IfBlock) {
    IfBlock->Condition->Type = S.Builder->CreateBoolType(IfBlock->Condition->getLocation());
    bool Success = ResolveExpr(IfBlock->getParent(), IfBlock->Condition) &&
                   S.Validator->CheckConvertibleTypes(IfBlock->Condition->Type, S.Builder->CreateBoolType(SourceLocation())) &&
            ResolveBlock(IfBlock);
    for (ASTElsifBlock *ElsifBlock : IfBlock->ElsifBlocks) {
        ElsifBlock->Condition->Type = S.Builder->CreateBoolType(ElsifBlock->Condition->getLocation());
        Success &= ResolveExpr(IfBlock->getParent(), ElsifBlock->Condition) &&
                   S.Validator->CheckConvertibleTypes(ElsifBlock->Condition->Type, S.Builder->CreateBoolType(SourceLocation())) &&
                ResolveBlock(ElsifBlock);
    }
    if (Success && IfBlock->ElseBlock) {
        Success = ResolveBlock(IfBlock->ElseBlock);
    }
    return Success;
}

bool SemaResolver::ResolveSwitchBlock(ASTSwitchBlock *SwitchBlock) {
    assert(SwitchBlock && "Switch Block cannot be null");
    bool Success = ResolveVarRef(SwitchBlock->getParent(), SwitchBlock->getVarRef()) &&
            S.Validator->CheckEqualTypes(SwitchBlock->getVarRef()->getDef()->getType(), ASTTypeKind::TYPE_INTEGER);
    for (ASTSwitchCaseBlock *Case : SwitchBlock->Cases) {
        Success &= S.Validator->CheckEqualTypes(Case->getType(), ASTTypeKind::TYPE_INTEGER) &&
                ResolveBlock(Case);
    }
    return Success && ResolveBlock(SwitchBlock->Default);
}

bool SemaResolver::ResolveLoopBlock(ASTLoopBlock *LoopBlock) {
    bool Success = S.Validator->CheckConvertibleTypes(LoopBlock->Condition->Type, S.Builder->CreateBoolType(LoopBlock->Condition->getLocation()));
    Success &= LoopBlock->Condition ? ResolveExpr(LoopBlock, LoopBlock->Condition) : true;
    Success &= ResolveBlock(LoopBlock);
    Success &= LoopBlock->Init ? ResolveBlock(LoopBlock->Post) : true;
    Success &= LoopBlock->Post ? ResolveBlock(LoopBlock->Post) : true;
    return Success;
}

bool SemaResolver::ResolveParentIdentifier(ASTStmt *Parent, ASTIdentifier *&Identifier) {
    const auto &Node = FindNode(Parent->getTop());

    if (Identifier->getParent()) {
        if (ResolveParentIdentifier(Parent, Identifier->Parent)) {

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
                        ResolveIdentityType(Node, IdentityType);
                    } else {
                        // Error:
                        S.Diag(Identifier->getLocation(), diag::err_sema_resolve_identifier);
                        return false;
                    }
                    break;
                }

                    // Instance
                case ASTIdentifierKind::REF_CALL: // NameSpace.call().call() or call().call()
                    return ResolveCallWithParent(Parent, (ASTCall *) Identifier);

                case ASTIdentifierKind::REF_VAR: // NameSpace.call().var or call().var
                    return ResolveVarRefWithParent((ASTVarRef *) Identifier);

                case ASTIdentifierKind::REF_UNDEF: // Error: identifier not resolved
                    assert(false && "Unexpected Identifier Kind");
            }
        }
    } else { // Do these only on first Parent identifier

        // Check if Identifier is a Call
        if (Identifier->isCall()) {
            return ResolveCallNoParent(Parent, (ASTCall *) Identifier);
        }

        // Check if Identifier is a Var
        ASTVar *Var = ResolveVarRefNoParent(Parent, Identifier->getName());
        if (Var) {
            Identifier = S.Builder->CreateVarRef(Identifier);
            ((ASTVarRef *) Identifier)->Def = Var;
            Identifier->Resolved = true;
            return true;
        }

        // Check if Identifier is an IdentityType
        ASTIdentityType *IdentityType = FindIdentityType(Identifier->getName(), Node->getNameSpace());
        if (IdentityType) {
            Identifier = IdentityType; // Take From Types
            return true;
        }

        // Check if Identifier is a NameSpace
        if (ResolveNameSpace(Node, Identifier)) {
            return true;
        }
    }

    S.Diag(Identifier->getLocation(), diag::err_sema_resolve_identifier);
    return false;
}

bool SemaResolver::ResolveIdentityType(ASTNode *Node, ASTIdentityType *IdentityType) {
    // Resolve Identifier
    if (IdentityType->getDef() == nullptr) {

        if (IdentityType->getParent() == nullptr) {
            ASTIdentityType *Type = FindIdentityType(IdentityType->getName(), Node->getNameSpace());
            if (Type) {
                IdentityType->Def = Type->getDef();
                IdentityType->IdentityTypeKind = Type->getIdentityTypeKind();
            }
        } else if (ResolveNameSpace(Node,IdentityType->Parent)) {
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

    IdentityType->Resolved = true;
    return true;
}

/**
 * Resolve a VarRef with its declaration
 * @param VarRef
 * @return true if no error occurs, otherwise false
 */
bool SemaResolver::ResolveVarRef(ASTStmt *Parent, ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveVarRefWithParent", Logger().Attr("VarRef", VarRef).End());
    
    if (!VarRef->Resolved) {
        if (VarRef->getParent() == nullptr) {
            VarRef->Def = ResolveVarRefNoParent(Parent, VarRef->getName());
        } else {
            ResolveParentIdentifier(Parent, VarRef->Parent) && ResolveVarRefWithParent((ASTVarRef *) VarRef);
        }
    }

    // VarRef not found in node, namespace and node imports
    if (VarRef->getDef() == nullptr) {
        S.Diag(VarRef->getLocation(), diag::err_unref_var);
        return false;
    }

    VarRef->Resolved = true;
    return true;
}

ASTVar *SemaResolver::ResolveVarRefNoParent(ASTStmt *Parent, llvm::StringRef Name) {

    // Search for LocalVar
    ASTVar *Var = FindLocalVar(Parent, Name);

    if (Var == nullptr) {
        ASTFunctionBase *Top = Parent->getTop();
        const auto &Node = FindNode(Top);

        // Search for Class Vars if Var is Class Method
        if (Top->getKind() == ASTFunctionKind::CLASS_METHOD)
            Var = ((ASTClassMethod *) Top)->getClass()->Vars.lookup(Name);

        // Search for GlobalVars
        if (Var == nullptr)
            Var = Node->getNameSpace()->GlobalVars.lookup(Name);
    }

    return Var;
}

ASTVar *SemaResolver::ResolveVarRef(llvm::StringRef Name, ASTIdentityType *IdentityType) {
    if (IdentityType->isClass()) {
        return ((ASTClass *) ((ASTClassType *) IdentityType)->getDef())->Vars.lookup(Name);
    } else if (IdentityType->isEnum()) {
        return ((ASTEnum *) ((ASTEnumType *) IdentityType)->getDef())->Vars.lookup(Name);
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
            ASTType * ParentType = ParentCall->getDef()->getType();

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

    if (VarRef->Def)
        VarRef->Resolved = true;

    return VarRef->Def;
}

bool SemaResolver::ResolveCall(ASTStmt *Parent, ASTCall *Call) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveCall", Logger().Attr("Call", Call).End());

    if (!Call->Resolved) {

        if (Call->getParent() == nullptr) {
            ResolveCallNoParent(Parent, Call);
        } else {
            ResolveParentIdentifier(Parent, Call->Parent) && ResolveCallWithParent(Parent, Call);
        }
    }

    // VarRef not found in node, namespace and node imports
    if (Call->getDef() == nullptr) {
        S.Diag(Call->getLocation(), diag::err_unref_call);
        return false;
    }

    Call->Resolved = true;
    return true;
}

bool SemaResolver::ResolveCall(ASTStmt *Parent, ASTCall *Call, ASTIdentityType *IdentityType) {

    if (IdentityType->isClass()) {
        auto &ClassMethods = ((ASTClass *) ((ASTClassType *) IdentityType)->getDef())->Methods;
        return ResolveCall(Parent, Call, ClassMethods);
    } else if (IdentityType->isEnum()) {
        S.Diag(Call->getLocation(), diag::err_sema_call_enum);
    } else {
        assert(false && "IdentityType unknown");
    }

    return false;
}

bool SemaResolver::ResolveCall(ASTStmt *Parent, ASTCall *Call, ASTNameSpace *NameSpace) {

    // NameSpace.func()
    bool Success = ResolveCall(Parent, Call, NameSpace->Functions);

    // NameSpace.constructor()
    if (!Success) {
        ASTIdentity *Identity = FindIdentity(Call->getName(), NameSpace);
        Identity != nullptr && Identity->getTopDefKind() == ASTTopDefKind::DEF_CLASS &&
        ResolveCall(Parent, Call, ((ASTClass *) Identity)->Constructors);
    }

    return Call->Def;
}

bool SemaResolver::ResolveCallNoParent(ASTStmt *Parent, ASTCall *Call) {
    const auto &Node = FindNode(Parent->getTop());

    // func()
    bool Success = ResolveCall(Parent, Call, Node->Functions) ||
                   ResolveCall(Parent, Call, Node->Context->DefaultNameSpace->Functions) ||
                   ResolveCall(Parent, Call, Node->getNameSpace()->Functions);

    // constructor()
    if (!Success) {
        ASTIdentity *Identity = FindIdentity(Call->getName(), Node->getNameSpace());
        Identity != nullptr && Identity->getTopDefKind() == ASTTopDefKind::DEF_CLASS &&
        ResolveCall(Parent, Call, ((ASTClass *) Identity)->Constructors);
    }

    return Call->Resolved;
}

bool SemaResolver::ResolveCallWithParent(ASTStmt *Parent, ASTCall *Call) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveCall", Logger().Attr("Call", Call).End());
        
    ASTFunctionBase *Top = Parent->getTop();
    const auto &Node = FindNode(Top);

    switch (Call->getParent()->getIdKind()) {

        case ASTIdentifierKind::REF_NAMESPACE: {
            ASTNameSpace *NameSpace = (ASTNameSpace *) Call->getParent();
            return ResolveCall(Parent, Call, NameSpace);
        }

        case ASTIdentifierKind::REF_TYPE: {
            ASTIdentityType *IdentityType = (ASTIdentityType *) Call->getParent();
            // NameSpace.IdentityType.call() or IdentityType.call()
            ResolveCall(Parent, Call, IdentityType);
            break;
        }

        // Instance
        case ASTIdentifierKind::REF_CALL: // NameSpace.call().call() call().call()
        {
            ASTCall *ParentCall = (ASTCall*) Call->getParent();

            // Parent is an Identity instance
            ASTType * ParentType = ParentCall->getDef()->getType();
            return ParentType->isIdentity() && ResolveCall(Parent, Call, (ASTIdentityType *) ParentType);
        }
        case ASTIdentifierKind::REF_VAR: // NameSpace.globalVarInstance.call() or instance.call()
        {
            ASTVarRef *ParentVarRef = (ASTVarRef *) Call->getParent();

            // Parent is an Identity instance
            ASTType * ParentType = ParentVarRef->getDef()->getType();
            return ParentType->isIdentity() && ResolveCall(Parent, Call, (ASTIdentityType *) ParentType);
        }

            // Error: identifier not resolved
        case ASTIdentifierKind::REF_UNDEF:
            assert(false && "Unexpected Identifier Kind");
    }

    if (Call->Def == nullptr) {
        S.Diag(Call->getLocation(), diag::err_unref_call);
        return false;
    }

    Call->Resolved = true;
    return Call->Def;
}

template <class T>
bool SemaResolver::ResolveCall(ASTStmt *Parent, ASTCall *Call,
                               llvm::StringMap<std::map <uint64_t,llvm::SmallVector <T *, 4>>> &Functions) {

    // Search by Call Name
    auto StrMapIt = Functions.find(Call->getName());
    if (StrMapIt != Functions.end()) {
        std::map<uint64_t, llvm::SmallVector<T *, 4>> &IntMap = StrMapIt->getValue();
        return ResolveCall(Parent, Call, IntMap);
    }

    return Call->Resolved;
}

template <class T>
bool SemaResolver::ResolveCall(ASTStmt *Parent, ASTCall *Call,
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
                        Success &= ResolveArg(Parent, Arg, Param);
                    }
                }

                if (Success) {
                    Call->Def = Function;
                    Call->Resolved = true;
                    break;
                }
            }
        }
        S.Validator->DiagEnabled = true;
    }

    return Call->Resolved;
}

bool SemaResolver::ResolveArg(ASTStmt *Parent, ASTArg *Arg, ASTParam *Param) {
    Arg->Def = Param;
    if (ResolveExpr(Parent, Arg->Expr)) {
        return S.Validator->CheckConvertibleTypes(Arg->Expr->Type, Param->getType());
    }

    return false;
}

/**
 * Resolve Expr contents
 * @param Expr
 * @return true if no error occurs, otherwise false
 */
bool SemaResolver::ResolveExpr(ASTStmt *Parent, ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveExpr", Logger().Attr("Expr", Expr).End());

    bool Success = false;
    switch (Expr->getExprKind()) {
        case ASTExprKind::EXPR_EMPTY:
            return true;
        case ASTExprKind::EXPR_VALUE: // Select the best option for this Value
            return ResolveValueExpr((ASTValueExpr *) Expr);
        case ASTExprKind::EXPR_VAR_REF: {
            ASTVarRef *VarRef = ((ASTVarRefExpr *)Expr)->getVarRef();
            if (ResolveVarRef(Parent, VarRef)) {
                Expr->Type = VarRef->getDef()->getType();
                Success = true;
                break;
            } else {
                return false;
            }
        }
        case ASTExprKind::EXPR_CALL: {
            ASTCall *Call = ((ASTCallExpr *)Expr)->getCall();
            if (ResolveCall(Parent, Call)) {
                switch (Call->getCallKind()) {

                    case ASTCallKind::CALL_FUNCTION:
                        Expr->Type = Call->Def->ReturnType;
                        break;
                    case ASTCallKind::CALL_CONSTRUCTOR:
                        Expr->Type = ((ASTClassMethod *) Call->Def)->getType();
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
                    Success = ResolveExpr(Parent, (ASTExpr *) Unary->First);
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

                    Success = ResolveExpr(Parent, Binary->First) && ResolveExpr(Parent, Binary->Second);
                    if (Success) {
                        if (Binary->getOptionKind() == ASTBinaryOptionKind::BINARY_ARITH ||
                                Binary->getOptionKind() == ASTBinaryOptionKind::BINARY_COMPARISON) {
                            Success = S.Validator->CheckArithTypes(Binary->OpLoc, Binary->First->Type,
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

                                Binary->Type = Binary->getOptionKind() == ASTBinaryOptionKind::BINARY_ARITH ?
                                        Binary->First->Type : S.Builder->CreateBoolType(Expr->getLocation());
                            }
                        } else if (Binary->getOptionKind() ==  ASTBinaryOptionKind::BINARY_LOGIC) {
                            Success = S.Validator->CheckLogicalTypes(Binary->OpLoc,
                                                                     Binary->First->Type, Binary->Second->Type);
                            Binary->Type = S.Builder->CreateBoolType(Expr->getLocation());
                        }
                    }
                    break;
                }
                case ASTExprGroupKind::GROUP_TERNARY: {
                    ASTTernaryGroupExpr *Ternary = (ASTTernaryGroupExpr *) Expr;
                    Success = ResolveExpr(Parent, Ternary->First) &&
                              S.Validator->CheckConvertibleTypes(Ternary->First->Type, S.Builder->CreateBoolType(SourceLocation())) &&
                              ResolveExpr(Parent, Ternary->Second) &&
                              ResolveExpr(Parent, Ternary->Third);
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

bool SemaResolver::ResolveValueExpr(ASTValueExpr *Expr) {
    const SourceLocation &Loc = Expr->Value->getLocation();
    
    switch (Expr->Value->getTypeKind()) {
        
        case ASTTypeKind::TYPE_BOOL:
            Expr->Type = S.Builder->CreateBoolType(Loc);
            break;
            
        case ASTTypeKind::TYPE_INTEGER: {
            ASTIntegerValue *Integer = ((ASTIntegerValue *) Expr->Value);

            if (Integer->Negative) { // Integer is negative (Ex. -2)

                if (Integer->Value > MIN_LONG) { // Negative Integer overflow min value
                    S.Diag(Expr->getLocation(), diag::err_sema_int_min_overflow);
                    return false;
                }

                if (Integer->Value > MIN_INT) {
                    Expr->Type = S.Builder->CreateLongType(Loc);
                } else if (Integer->Value > MIN_SHORT) {
                    Expr->Type = S.Builder->CreateIntType(Loc);
                } else {
                    Expr->Type = S.Builder->CreateShortType(Loc);
                }
            } else { // Positive Integer

                if (Integer->Value > MAX_LONG) { // Positive Integer overflow max value
                    S.Diag(Expr->getLocation(), diag::err_sema_int_max_overflow);
                    return false;
                }

                if (Integer->Value > MAX_INT) {
                    Expr->Type = S.Builder->CreateLongType(Loc);
                } else if (Integer->Value > MAX_SHORT) {
                    Expr->Type = S.Builder->CreateIntType(Loc);
                } else if (Integer->Value > MAX_BYTE) {
                    Expr->Type = S.Builder->CreateShortType(Loc);
                } else {
                    Expr->Type = S.Builder->CreateByteType(Loc);
                }
            }
            break;
        }
        
        case ASTTypeKind::TYPE_FLOATING_POINT:
            // Creating as Float on first but transform in Double if is contained into a Binary Expr with a Double Type
            Expr->Type = S.Builder->CreateDoubleType(Loc);
            break;

        case ASTTypeKind::TYPE_STRING:
            Expr->Type = S.Builder->CreateStringType(Loc);
            break;

        case ASTTypeKind::TYPE_ARRAY:
            // TODO
            break;
        case ASTTypeKind::TYPE_IDENTITY:
            // TODO
            break;
    }
    
    return true;
}


ASTNameSpace *SemaResolver::FindNameSpace(llvm::StringRef Name) const {
    FLY_DEBUG_MESSAGE("Sema", "FindNameSpace", "Name=" << Name);
    ASTNameSpace *NameSpace = S.Context->NameSpaces.lookup(Name);
    if (!NameSpace) {
        S.Diag(diag::err_unref_namespace) << Name;
    }
    return NameSpace;
}

ASTNode *SemaResolver::FindNode(ASTFunctionBase *FunctionBase) const {
    FLY_DEBUG_MESSAGE("Sema", "FindNode", Logger().Attr("FunctionBase", FunctionBase).End());
    if (FunctionBase->getKind() == ASTFunctionKind::FUNCTION) {
        return ((ASTFunction *) FunctionBase)->getNode();
    } else if (FunctionBase->getKind() == ASTFunctionKind::CLASS_METHOD) {
        return ((ASTClassMethod *) FunctionBase)->getClass()->getNode();
    } else {
        assert("Unknown Function Kind");
        return nullptr;
    }
}

ASTNode *SemaResolver::FindNode(llvm::StringRef Name, ASTNameSpace *NameSpace) const {
    FLY_DEBUG_MESSAGE("Sema", "FindNode", Logger().Attr("Name", Name).Attr("NameSpace", NameSpace).End());
    ASTNode *Node = NameSpace->Nodes.lookup(Name);
    if (!Node) {
        S.Diag(diag::err_unref_node) << Name;
    }
    return Node;
}

ASTImport *SemaResolver:: FindImport(ASTNode *Node, llvm::StringRef Name) {
    // Search into Node imports
    ASTImport *Import = Node->Imports.lookup(Name);
    return Import == nullptr ? Node->AliasImports.lookup(Name) : Import;
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
 * @param Parent
 * @param Identifier
 * @return the found LocalVar
 */
ASTVar *SemaResolver::FindLocalVar(ASTStmt *Parent, llvm::StringRef Name) const {
    FLY_DEBUG_MESSAGE("Sema", "FindLocalVar", Logger().Attr("Parent", Parent).Attr("Name", Name).End());
    if (Parent->getKind() == ASTStmtKind::STMT_BLOCK) {
        ASTBlock *Block = (ASTBlock *) Parent;
        const auto &It = Block->getLocalVars().find(Name);
        if (It != Block->getLocalVars().end()) { // Search into this Block
            return It->getValue();
        } else if (Parent->getParent()) { // search recursively into Parent Blocks to find the right Var definition
            if (Parent->Parent->getKind() == ASTStmtKind::STMT_BLOCK)
                return FindLocalVar((ASTBlock *) Parent->getParent(), Name);
        } else {
            llvm::SmallVector<ASTParam *, 8> Params = Parent->getTop()->getParams();
            for (auto &Param : Params) {
                if (Param->getName() == Name) { // Search into ASTParam list
                    return Param;
                }
            }
        }
    }
    return nullptr;
}
