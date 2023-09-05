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
#include "AST/ASTClassFunction.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassVar.h"
#include "AST/ASTEnum.h"
#include "AST/ASTEnumVar.h"
#include "AST/ASTType.h"
#include "AST/ASTNode.h"
#include "AST/ASTIfBlock.h"
#include "AST/ASTImport.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTParams.h"
#include "AST/ASTSwitchBlock.h"
#include "AST/ASTForBlock.h"
#include "AST/ASTWhileBlock.h"
#include "AST/ASTBlock.h"
#include "AST/ASTValue.h"
#include "AST/ASTVar.h"
#include "AST/ASTVarAssign.h"
#include "AST/ASTVarRef.h"
#include "Sys/Sys.h"
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
    for (auto &NodeEntry : S.Builder->Context->getNodes()) {
        auto &Node = NodeEntry.getValue();
        Success &= ResolveImports(Node); // resolve Imports with NameSpaces
        Success &= ResolveGlobalVars(Node); // resolve Global Vars
        Success &= ResolveIdentities(Node);  // resolve Identity attributes and methods
        Success &= ResolveFunctions(Node);  // resolve ASTBlock of Body Functions
    }

    // Now all Imports must be read
    for(auto &Import : S.Builder->Context->ExternalImports) {
        if (!Import.getValue()->getNameSpace()) {
            S.Diag(Import.getValue()->getLocation(), diag::err_unresolved_import);
            return false;
        }
    }

    return Success;
}

bool SemaResolver::ResolveNameSpace(ASTNode *Node, ASTIdentifier *&Identifier) {
    ASTImport *Import = S.FindImport(Node, Identifier->FullName);
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
            S.Diag(Import->NameLocation, diag::err_namespace_notfound) << Import->getName();
        }
    }

    return Success;
}

bool SemaResolver::ResolveGlobalVars(ASTNode *Node) {
    bool Success = true;

    for (auto &GlobalVarEntry : Node->getGlobalVars()) {
        ASTGlobalVar *GlobalVar = GlobalVarEntry.getValue();
        Success = !GlobalVar->getType()->isIdentity() || ResolveIdentityType(Node, (ASTIdentityType *) GlobalVar->getType());
        Success &= S.Validator->CheckGlobalVar(GlobalVar);
    }

    return Success;
}

bool SemaResolver::ResolveIdentities(ASTNode *Node) {
    bool Success = true;
    if (Node->Identity) {

        if (Node->Identity->getKind() == ASTTopDefKind::DEF_CLASS) {
            ASTClass *Class = (ASTClass *) Node->Identity;

            // Resolve Super Classes
            if (!Class->SuperClasses.empty()) {
                llvm::StringMap<std::map<uint64_t, llvm::SmallVector<ASTClassFunction *, 4>>> SuperMethods;
                llvm::StringMap<std::map<uint64_t, llvm::SmallVector<ASTClassFunction *, 4>>> ISuperMethods;
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
                                ASTClassVar *&SuperVar = EntryVar.getValue();

                                // Check Var already exists and type conflicts in Super Vars
                                ASTClassVar *ClassVar = Class->Vars.lookup(EntryVar.getKey());
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
                                    for (ASTClassFunction *SuperMethod: MapIt->second) {
                                        if (SuperClass->getClassKind() == ASTClassKind::INTERFACE) {
                                            S.Builder->InsertFunction(ISuperMethods, SuperMethod);
                                        } else {
                                            // Insert methods in the Super and if is ok also in the base Class
                                            if (S.Builder->InsertFunction(SuperMethods, SuperMethod)) {
                                                ASTClassFunction *M = S.Builder->CreateClassMethod(Class,
                                                                                                   SuperMethod->getLocation(),
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
                        for (ASTClassFunction *ISuperMethod: MapIt->second) {
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
        } else if (Node->Identity->getKind() == ASTTopDefKind::DEF_ENUM) {
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

bool SemaResolver::ResolveBlock(ASTBlock *Block) {
    bool Success = true;
    for (ASTStmt *Stmt : Block->Content) {
        switch (Stmt->getKind()) {

            case ASTStmtKind::STMT_BLOCK:
                switch (((ASTBlock *) Stmt)->getBlockKind()) {

                    case ASTBlockKind::BLOCK_IF:
                        Success &= ResolveIfBlock((ASTIfBlock *) Stmt);
                        break;
                    case ASTBlockKind::BLOCK_SWITCH:
                        Success &= ResolveSwitchBlock((ASTSwitchBlock *) Stmt);
                        break;
                    case ASTBlockKind::BLOCK_WHILE:
                        Success &= ResolveWhileBlock((ASTWhileBlock *) Stmt);
                        break;
                    case ASTBlockKind::BLOCK_FOR:
                        Success &= ResolveForBlock((ASTForBlock *) Stmt);
                        break;
                    case ASTBlockKind::BLOCK:
                    case ASTBlockKind::BLOCK_ELSIF:
                    case ASTBlockKind::BLOCK_ELSE:
                    case ASTBlockKind::BLOCK_SWITCH_CASE:
                    case ASTBlockKind::BLOCK_SWITCH_DEFAULT:
                    case ASTBlockKind::BLOCK_FOR_LOOP:
                    case ASTBlockKind::BLOCK_FOR_POST:
                        Success &= ResolveBlock((ASTBlock *) Stmt);
                        break;
                }
                break;
            case ASTStmtKind::STMT_EXPR:
                Success &= ResolveExpr(Block, ((ASTExprStmt *) Stmt)->getExpr());
                break;
            case ASTStmtKind::STMT_VAR_DEFINE: {
                ASTLocalVar *LocalVar = ((ASTLocalVar *) Stmt);
                // Take Node
                const auto &Node = S.FindNode(Block->getTop());
                Success &= !LocalVar->getType()->isIdentity() || ResolveIdentityType(Node, (ASTIdentityType *) LocalVar->Type);

                if (LocalVar->getExpr())
                    Success &= ResolveExpr(Block, LocalVar->getExpr());
                else // Var not initialized
                    Block->UnInitVars.insert(std::make_pair(LocalVar->getName(), LocalVar));
                break;
            }
            case ASTStmtKind::STMT_VAR_ASSIGN: {
                ASTVarAssign *VarAssign = ((ASTVarAssign *) Stmt);

                // Error: Expr cannot be null
                if (!VarAssign->getExpr()) {
                    S.Diag(VarAssign->getLocation(), diag::err_var_assign_empty) << VarAssign->getVarRef()->getName();
                    return false;
                }

                Success &= (VarAssign->getVarRef()->getDef() || ResolveVarRef(Block, VarAssign->getVarRef())) &&
                           ResolveExpr(Block, VarAssign->getExpr());

                // Remove from Un-Initialized Var
                if (Success) {
                    auto It = Block->UnInitVars.find(VarAssign->getVarRef()->getDef()->getName());
                    if (It != Block->UnInitVars.end())
                        Block->UnInitVars.erase(It);
                }
                break;
            }
            case ASTStmtKind::STMT_RETURN:
                Success &= ResolveExpr(Block, ((ASTReturn *) Stmt)->getExpr());
                break;
            case ASTStmtKind::STMT_BREAK:
            case ASTStmtKind::STMT_CONTINUE:
                break;
        }
    }

    if (!Block->UnInitVars.empty()) {
        for (auto &UnInitVar : Block->UnInitVars) {
            S.Diag(UnInitVar.getValue()->getLocation(), diag::err_sema_uninit_var) << UnInitVar.getValue()->getName();
        }
        return false;
    }

    return Success;
}

bool SemaResolver::ResolveIfBlock(ASTIfBlock *IfBlock) {
    IfBlock->Condition->Type = SemaBuilder::CreateBoolType(IfBlock->Condition->getLocation());
    bool Success = ResolveExpr(IfBlock->getParent(), IfBlock->Condition) &&
            S.Validator->CheckConvertibleTypes(IfBlock->Condition->Type, SemaBuilder::CreateBoolType(SourceLocation())) &&
                   ResolveBlock(IfBlock);
    for (ASTElsifBlock *ElsifBlock : IfBlock->ElsifBlocks) {
        ElsifBlock->Condition->Type = SemaBuilder::CreateBoolType(ElsifBlock->Condition->getLocation());
        Success &= ResolveExpr(IfBlock->getParent(), ElsifBlock->Condition) &&
                S.Validator->CheckConvertibleTypes(ElsifBlock->Condition->Type, SemaBuilder::CreateBoolType(SourceLocation())) &&
                   ResolveBlock(ElsifBlock);
    }
    if (Success && IfBlock->ElseBlock) {
        Success = ResolveBlock(IfBlock->ElseBlock);
    }
    return Success;
}

bool SemaResolver::ResolveSwitchBlock(ASTSwitchBlock *SwitchBlock) {
    assert(SwitchBlock && "Switch Block cannot be null");
    bool Success = ResolveExpr(SwitchBlock->getParent(), SwitchBlock->Expr) &&
            S.Validator->CheckEqualTypes(SwitchBlock->Expr->Type, ASTTypeKind::TYPE_INTEGER);
    for (ASTSwitchCaseBlock *Case : SwitchBlock->Cases) {
        Success &= ResolveExpr(SwitchBlock, Case->Expr) &&
                S.Validator->CheckEqualTypes(SwitchBlock->Expr->Type, ASTTypeKind::TYPE_INTEGER) && ResolveBlock(Case);
    }
    return Success && ResolveBlock(SwitchBlock->Default);
}

bool SemaResolver::ResolveWhileBlock(ASTWhileBlock *WhileBlock) {
    return ResolveExpr(WhileBlock->getParent(), WhileBlock->Condition) &&
            S.Validator->CheckConvertibleTypes(WhileBlock->Condition->Type,
                                    SemaBuilder::CreateBoolType(WhileBlock->Condition->Loc)) &&
            ResolveBlock(WhileBlock);
}

bool SemaResolver::ResolveForBlock(ASTForBlock *ForBlock) {
    bool Success = ResolveBlock(ForBlock) && ResolveExpr(ForBlock, ForBlock->Condition) &&
            S.Validator->CheckConvertibleTypes(ForBlock->Condition->Type, SemaBuilder::CreateBoolType(ForBlock->Condition->Loc));
    if (ForBlock->Post) {
        Success &= ResolveBlock(ForBlock->Post);
    }
    if (ForBlock->Loop) {
        Success &= ResolveBlock(ForBlock->Loop);
    }

    return Success;
}

bool SemaResolver::ResolveParentIdentifier(ASTBlock *Block, ASTIdentifier *&Identifier) {
    const auto &Node = S.FindNode(Block->getTop());

    if (Identifier->getParent()) {
        if (ResolveParentIdentifier(Block, Identifier->Parent)) {

            // Do these in the parents different from first
            switch (Identifier->getKind()) {

                case ASTIdentifierKind::REF_NAMESPACE: {
                    if (Identifier->getParent()->getKind() != ASTIdentifierKind::REF_NAMESPACE) {
                        // Error:
                        S.Diag(Identifier->getLocation(), diag::err_sema_resolve_identifier);
                        return false;
                    }
                    break;
                }

                case ASTIdentifierKind::REF_TYPE: {
                    ASTIdentityType *IdentityType = (ASTIdentityType *) Identifier;
                    if (Identifier->getParent()->getKind() == ASTIdentifierKind::REF_NAMESPACE) {
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
                    return ResolveCallWithParent(Block, (ASTCall *) Identifier);

                case ASTIdentifierKind::REF_VAR: // NameSpace.call().var or call().var
                    return ResolveVarRefWithParent((ASTVarRef *) Identifier);

                case ASTIdentifierKind::REF_UNDEF: // Error: identifier not resolved
                    assert(false && "Unexpected Identifier Kind");
            }
        }
    } else { // Do these only on first Parent identifier

        // Check if Identifier is a Call
        if (Identifier->isCall()) {
            return ResolveCallNoParent(Block, (ASTCall *) Identifier);
        }

        // Check if Identifier is a Var
        ASTVar *Var = ResolveVarRefNoParent(Block, Identifier->getName());
        if (Var) {
            Identifier = S.Builder->CreateVarRef(Identifier);
            ((ASTVarRef *) Identifier)->Def = Var;
            return true;
        }

        // Check if Identifier is an IdentityType
        ASTIdentity *Identity = S.FindIdentity(Identifier->getName(), Node->getNameSpace());
        if (Identity) {
            Identifier = Identity->getType();
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
    if (IdentityType->Def == nullptr) {

        if (IdentityType->getParent() == nullptr) {
            IdentityType->Def = S.FindIdentity(IdentityType->getName(), Node->getNameSpace());
            IdentityType->IdentityKind = IdentityType->Def->getType()->getIdentityKind();
        } else if (ResolveNameSpace(Node,IdentityType->Parent)) {
            ASTNameSpace *NameSpace = (ASTNameSpace *) IdentityType->getParent();
            IdentityType->Def = S.FindIdentity(IdentityType->getName(), NameSpace);
            IdentityType->IdentityKind = IdentityType->Def->getType()->getIdentityKind();
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
bool SemaResolver::ResolveVarRef(ASTBlock *Block, ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveVarRefWithParent", Logger().Attr("VarRef", VarRef).End());
    
    if (VarRef->Def == nullptr) {

        // error type
        if (VarRef->getName() == "error") {

            // this is the current function error var
            if (VarRef->Parent == nullptr ||
                    (ResolveParentIdentifier(Block, VarRef->Parent) && VarRef->Parent->isCall()) // check if parent is a call()
                    ) {
                VarRef->Def = Sys::getError();
            }
        } else {
            if (VarRef->getParent() == nullptr) {
                VarRef->Def = ResolveVarRefNoParent(Block, VarRef->getName());
            } else {
                ResolveParentIdentifier(Block, VarRef->Parent) && ResolveVarRefWithParent((ASTVarRef *) VarRef);
            }
        }
    }

    // VarRef not found in node, namespace and node imports
    if (VarRef->Def == nullptr) {
        S.Diag(VarRef->getLocation(), diag::err_unref_var);
        return false;
    }

    return true;
}

ASTVar *SemaResolver::ResolveVarRefNoParent(ASTBlock *Block, llvm::StringRef Name) {

    // Search for LocalVar
    ASTVar *Var = S.FindLocalVar(Block, Name);

    if (Var == nullptr) {
        ASTFunctionBase *Top = Block->getTop();
        const auto &Node = S.FindNode(Top);

        // Search for Class Vars if Var is Class Method
        if (Top->getKind() == ASTFunctionKind::CLASS_FUNCTION)
            Var = ((ASTClassFunction *) Top)->getClass()->Vars.lookup(Name);

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
    switch (VarRef->getParent()->getKind()) {

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

    return VarRef->Def;
}

bool SemaResolver::ResolveCall(ASTBlock *Block, ASTCall *Call) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveCall", Logger().Attr("Call", Call).End());

    if (Call->Def == nullptr) {

        if (Call->getParent() == nullptr) {
            ResolveCallNoParent(Block, Call);
        } else {
            ResolveParentIdentifier(Block, Call->Parent) && ResolveCallWithParent(Block, Call);
        }
    }

    // VarRef not found in node, namespace and node imports
    if (Call->Def == nullptr) {
        S.Diag(Call->getLocation(), diag::err_unref_call);
        return false;
    }

    return true;
}

bool SemaResolver::ResolveCall(ASTBlock *Block, ASTCall *Call, ASTIdentityType *IdentityType) {

    if (IdentityType->isClass()) {
        auto &ClassMethods = ((ASTClass *) ((ASTClassType *) IdentityType)->getDef())->Methods;
        return ResolveCall(Block, Call, ClassMethods);
    } else if (IdentityType->isEnum()) {
        S.Diag(Call->getLocation(), diag::err_sema_call_enum);
    } else {
        assert(false && "IdentityType unknown");
    }

    return false;
}

bool SemaResolver::ResolveCall(ASTBlock *Block, ASTCall *Call, ASTNameSpace *NameSpace) {

    // NameSpace.func()
    bool Success = ResolveCall(Block, Call, NameSpace->Functions);

    // NameSpace.constructor()
    if (!Success) {
        ASTIdentity *Identity = S.FindIdentity(Call->getName(), NameSpace);
        Identity != nullptr && Identity->getKind() == ASTTopDefKind::DEF_CLASS &&
        ResolveCall(Block, Call, ((ASTClass *) Identity)->Constructors);
    }

    return Call->Def;
}

bool SemaResolver::ResolveCallNoParent(ASTBlock *Block, ASTCall *Call) {
    const auto &Node = S.FindNode(Block->getTop());

    // func()
    bool Success = ResolveCall(Block, Call, Node->Functions) ||
            ResolveCall(Block, Call, Node->Context->DefaultNameSpace->Functions) ||
            ResolveCall(Block, Call, Node->getNameSpace()->Functions);

    // constructor()
    if (!Success) {
        ASTIdentity *Identity = S.FindIdentity(Call->getName(), Node->getNameSpace());
        Identity != nullptr && Identity->getKind() == ASTTopDefKind::DEF_CLASS &&
        ResolveCall(Block, Call, ((ASTClass *) Identity)->Constructors);
    }

    return Call->Def;
}

bool SemaResolver::ResolveCallWithParent(ASTBlock *Block, ASTCall *Call) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveCall", Logger().Attr("Call", Call).End());
        
    ASTFunctionBase *Top = Block->getTop();
    const auto &Node = S.FindNode(Top);

    switch (Call->getParent()->getKind()) {

        case ASTIdentifierKind::REF_NAMESPACE: {
            ASTNameSpace *NameSpace = (ASTNameSpace *) Call->getParent();
            return ResolveCall(Block, Call, NameSpace);
        }

        case ASTIdentifierKind::REF_TYPE: {
            ASTIdentityType *IdentityType = (ASTIdentityType *) Call->getParent();
            // NameSpace.IdentityType.call() or IdentityType.call()
            ResolveCall(Block, Call, IdentityType);
            break;
        }

        // Instance
        case ASTIdentifierKind::REF_CALL: // NameSpace.call().call() call().call()
        {
            ASTCall *ParentCall = (ASTCall*) Call->getParent();

            // Parent is an Identity instance
            ASTType * ParentType = ParentCall->getDef()->getType();
            return ParentType->isIdentity() && ResolveCall(Block, Call, (ASTIdentityType *) ParentType);
        }
        case ASTIdentifierKind::REF_VAR: // NameSpace.globalVarInstance.call() or instance.call()
        {
            ASTVarRef *ParentVarRef = (ASTVarRef *) Call->getParent();

            // Parent is an Identity instance
            ASTType * ParentType = ParentVarRef->getDef()->getType();
            return ParentType->isIdentity() && ResolveCall(Block, Call, (ASTIdentityType *) ParentType);
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
bool SemaResolver::ResolveCall(ASTBlock *Block, ASTCall *Call,
                               llvm::StringMap<std::map <uint64_t,llvm::SmallVector <T *, 4>>> &Functions) {

    // Search by Call Name
    auto StrMapIt = Functions.find(Call->getName());
    if (StrMapIt != Functions.end()) {
        std::map<uint64_t, llvm::SmallVector<T *, 4>> &IntMap = StrMapIt->getValue();
        return ResolveCall(Block, Call, IntMap);
    }

    return Call->Def;
}

template <class T>
bool SemaResolver::ResolveCall(ASTBlock *Block, ASTCall *Call,
                               std::map <uint64_t,llvm::SmallVector <T *, 4>> &Functions) {
    // Search by number of arguments
    const auto &IntMapIt = Functions.find(Call->getArgs().size());
    if (IntMapIt != Functions.end()) { // Map contains Function with this size of args
        S.Validator->DiagEnabled = false;
        for (T *Function : IntMapIt->second) {
            if (Function->getParams()->getSize() == Call->getArgs().size()) {
                bool Success = true; // if Params = Args = 0 skip for cycle
                if (Call->getArgs().size() == 0) { // call function with no parameters
                    Success = true;
                } else {
                    for (unsigned long i = 0; i < Function->getParams()->getSize(); i++) {
                        // Resolve Arg Expr on first
                        ASTArg *Arg = Call->getArgs().at(i);
                        ASTParam *Param = Function->getParams()->at(i);
                        Success &= ResolveArg(Block, Arg, Param);
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

//bool SemaResolver::ResolveCall(ASTBlock *Block, ASTCall *Call, llvm::StringMap<ASTImport *> Imports) {
//    for (auto &MapEntry : Imports) {
//        ASTImport * Import = MapEntry.getValue();
//
//    }
//
//    return Call->Def;
//}

bool SemaResolver::ResolveArg(ASTBlock *Block, ASTArg *Arg, ASTParam *Param) {
    Arg->Def = Param;
    if (ResolveExpr(Block, Arg->Expr)) {
        return S.Validator->CheckConvertibleTypes(Arg->Expr->Type, Param->Type);
    }

    return false;
}

/**
 * Resolve Expr contents
 * @param Expr
 * @return true if no error occurs, otherwise false
 */
bool SemaResolver::ResolveExpr(ASTBlock *Block, ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveExpr", Logger().Attr("Expr", Expr).End());

    bool Success = false;
    switch (Expr->getExprKind()) {
        case ASTExprKind::EXPR_EMPTY:
            return true;
        case ASTExprKind::EXPR_VALUE: // Select the best option for this Value
            return ResolveValueExpr((ASTValueExpr *) Expr);
        case ASTExprKind::EXPR_VAR_REF: {
            ASTVarRef *VarRef = ((ASTVarRefExpr *)Expr)->getVarRef();
            if (VarRef->getDef() || ResolveVarRef(Block, VarRef)) {
                Expr->Type = VarRef->getDef()->getType();
                Success = true;
                break;
            } else {
                return false;
            }
        }
        case ASTExprKind::EXPR_CALL: {
            ASTCall *Call = ((ASTCallExpr *)Expr)->getCall();
            if (Call->getDef() || ResolveCall(Block, Call)) {
                Expr->Type = Call->getCallKind() == ASTCallKind::CALL_NORMAL ?
                        Call->Def->ReturnType :
                        ((ASTClassFunction *) Call->Def)->getClass()->getType();
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
                    Success = ResolveExpr(Block, (ASTExpr *) Unary->First);
                    Expr->Type = Unary->First->Type;
                    break;
                }
                case ASTExprGroupKind::GROUP_BINARY: {
                    ASTBinaryGroupExpr *Binary = (ASTBinaryGroupExpr *) Expr;

                    if (Binary->First->Kind == ASTExprKind::EXPR_EMPTY) {
                        // Error: Binary cannot contain ASTEmptyExpr
                        S.Diag(Binary->First->Loc, diag::err_sema_empty_expr);
                        return false;
                    }

                    if (Binary->Second->Kind == ASTExprKind::EXPR_EMPTY) {
                        // Error: Binary cannot contain ASTEmptyExpr
                        S.Diag(Binary->Second->Loc, diag::err_sema_empty_expr);
                        return false;
                    }

                    Success = ResolveExpr(Block, Binary->First) && ResolveExpr(Block, Binary->Second);
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
                                        Binary->First->Type : SemaBuilder::CreateBoolType(Expr->Loc);
                            }
                        } else if (Binary->getOptionKind() ==  ASTBinaryOptionKind::BINARY_LOGIC) {
                            Success = S.Validator->CheckLogicalTypes(Binary->OpLoc,
                                                                     Binary->First->Type, Binary->Second->Type);
                            Binary->Type = SemaBuilder::CreateBoolType(Expr->Loc);
                        }
                    }
                    break;
                }
                case ASTExprGroupKind::GROUP_TERNARY: {
                    ASTTernaryGroupExpr *Ternary = (ASTTernaryGroupExpr *) Expr;
                    Success = ResolveExpr(Block, Ternary->First) &&
                            S.Validator->CheckConvertibleTypes(Ternary->First->Type, SemaBuilder::CreateBoolType(SourceLocation())) &&
                              ResolveExpr(Block, Ternary->Second) &&
                              ResolveExpr(Block, Ternary->Third);
                    Ternary->Type = Ternary->Second->Type; // The group type is equals to the second type
                    break;
                }
            }
            break;
        }
        default:
            assert(0 && "Invalid ASTExprKind");
    }

    // Check Expr Type if it needs to be converted to Stmt Type
    if (!Expr->Parent &&Expr->Stmt && Expr->Stmt->Kind != ASTStmtKind::STMT_EXPR) {
        ASTType *ToType = getType(Expr->Stmt);
        Success &= S.Validator->CheckConvertibleTypes(Expr->Type, ToType);
    }

    return Success;
}

bool SemaResolver::ResolveValueExpr(ASTValueExpr *Expr) {
    const SourceLocation &Loc = Expr->Value->getLocation();
    
    switch (Expr->Value->getTypeKind()) {
        
        case ASTTypeKind::TYPE_BOOL:
            Expr->Type = SemaBuilder::CreateBoolType(Loc);
            break;
            
        case ASTTypeKind::TYPE_INTEGER: {
            ASTIntegerValue *Integer = ((ASTIntegerValue *) Expr->Value);

            if (Integer->Negative) { // Integer is negative (Ex. -2)

                if (Integer->Value > MIN_LONG) { // Negative Integer overflow min value
                    S.Diag(Expr->getLocation(), diag::err_sema_int_min_overflow);
                    return false;
                }

                if (Integer->Value > MIN_INT) {
                    Expr->Type = SemaBuilder::CreateLongType(Loc);
                } else if (Integer->Value > MIN_SHORT) {
                    Expr->Type = SemaBuilder::CreateIntType(Loc);
                } else {
                    Expr->Type = SemaBuilder::CreateShortType(Loc);
                }
            } else { // Positive Integer

                if (Integer->Value > MAX_LONG) { // Positive Integer overflow max value
                    S.Diag(Expr->getLocation(), diag::err_sema_int_max_overflow);
                    return false;
                }

                if (Integer->Value > MAX_INT) {
                    Expr->Type = SemaBuilder::CreateLongType(Loc);
                } else if (Integer->Value > MAX_SHORT) {
                    Expr->Type = SemaBuilder::CreateIntType(Loc);
                } else if (Integer->Value > MAX_BYTE) {
                    Expr->Type = SemaBuilder::CreateShortType(Loc);
                } else {
                    Expr->Type = SemaBuilder::CreateByteType(Loc);
                }
            }
            break;
        }
        
        case ASTTypeKind::TYPE_FLOATING_POINT:
            // Creating as Float on first but transform in Double if is contained into a Binary Expr with a Double Type
            Expr->Type = SemaBuilder::CreateDoubleType(Loc);
            break;

        case ASTTypeKind::TYPE_STRING:
            Expr->Type = SemaBuilder::CreateStringType(Loc);
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

/**
 * Get the Type of a Stmt
 * @param Stmt
 * @return
 */
ASTType *SemaResolver::getType(ASTStmt *Stmt) {
    switch (Stmt->getKind()) {
        case ASTStmtKind::STMT_VAR_DEFINE: // int a = 1
            return ((ASTLocalVar *) Stmt)->getType();
        case ASTStmtKind::STMT_VAR_ASSIGN: // a = 1
            return ((ASTVarAssign *) Stmt)->getVarRef()->getDef()->getType();
        case ASTStmtKind::STMT_RETURN:
            return ((ASTBlock *) Stmt->getParent())->Top->ReturnType;
        case ASTStmtKind::STMT_EXPR:
            return ((ASTExprStmt *) Stmt)->Expr->Type;
        case ASTStmtKind::STMT_BLOCK:
            switch (((ASTBlock *) Stmt)->getBlockKind()) {
                case ASTBlockKind::BLOCK_IF:
                    return ((ASTIfBlock *) Stmt)->Condition->Type;
                case ASTBlockKind::BLOCK_ELSIF:
                    return ((ASTElsifBlock *) Stmt)->Condition->Type;
                case ASTBlockKind::BLOCK_SWITCH:
                    return ((ASTSwitchBlock *) Stmt)->Expr->Type;
                case ASTBlockKind::BLOCK_SWITCH_CASE:
                    return ((ASTSwitchCaseBlock *) Stmt)->Expr->Type;
                case ASTBlockKind::BLOCK_WHILE:
                    return ((ASTWhileBlock *) Stmt)->Condition->Type;
                case ASTBlockKind::BLOCK_FOR:
                    return ((ASTForBlock *) Stmt)->Condition->Type;
            }
    }

    assert("This Stmt not contains an ASTType");
    return nullptr;
}
