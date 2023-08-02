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
#include "AST/ASTClassFunction.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassVar.h"
#include "AST/ASTEnum.h"
#include "AST/ASTEnumVar.h"
#include "AST/ASTNameSpace.h"
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
    for (auto &NEntry : S.Builder->Context->getNodes()) {
        auto &Node = NEntry.getValue();
        Success &= ResolveImports(Node); // resolve Imports with NameSpaces
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
                                            S.Builder->InsertFunction(ISuperMethods, SuperMethod, true);
                                        } else {
                                            // Insert methods in the Super and if is ok also in the base Class
                                            if (S.Builder->InsertFunction(SuperMethods, SuperMethod, true)) {
                                                ASTClassFunction *M = S.Builder->CreateClassMethod(Class,
                                                                                                   SuperMethod->getLocation(),
                                                                                                   SuperMethod->getType(),
                                                                                                   SuperMethod->getName(),
                                                                                                   SuperMethod->getScopes());
                                                M->Params = SuperMethod->Params;
                                                M->Body = SuperMethod->Body;
                                                M->DerivedClass = Class;
                                                S.Builder->InsertFunction(Class->Methods, M, true);

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
                Success &= !LocalVar->getType()->isIdentity() || ResolveIdentityType(Node, (ASTIdentityType *&) LocalVar->Type);

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
    bool Success = ResolveExpr(SwitchBlock->getParent(), SwitchBlock->Expr) && S.Validator->CheckMacroType(SwitchBlock->Expr->Type, ASTTypeKind::TYPE_INTEGER);
    for (ASTSwitchCaseBlock *Case : SwitchBlock->Cases) {
        Success &= ResolveExpr(SwitchBlock, Case->Expr) &&
                   S.Validator->CheckMacroType(SwitchBlock->Expr->Type, ASTTypeKind::TYPE_INTEGER) && ResolveBlock(Case);
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

bool SemaResolver::ResolveIdentifier(ASTBlock *Block, ASTIdentifier *Identifier) {
    ASTFunctionBase *Top = Block->getTop();
    const auto &Node = S.FindNode(Top);

    ASTIdentifier *Current = Identifier->getIndex() > 0 ? Identifier->getRoot() : Identifier;
    ASTIdentifier *Previous = nullptr;

    ASTNameSpace *NameSpace = nullptr;
    ASTIdentityType *IdentityType = nullptr;

    ASTVar *Var = nullptr;
    ASTFunctionBase *Function = nullptr;

    uint32_t I = Current->getIndex();
    uint32_t Size = Identifier->getIndex() + 1;
    while (I < Size) {

        // Call
        if (Current->isCall()) {

            // only the first time
            if (Var == nullptr && Function == nullptr && IdentityType == nullptr && NameSpace == nullptr) { // only the first time
                ASTIdentity *Identity = S.FindIdentity(Current->getName(), Node->getNameSpace());
                if (Identity != nullptr && Identity->getKind() == ASTTopDefKind::DEF_CLASS &&
                    ResolveCall(Block, Current->getCall(), ((ASTClass *) Identity)->Constructors)) {

                    // constructor()
                    Function = Current->getCall()->getDef();
                } else if (ResolveCall(Block, Current->getCall(), Node->Functions) ||
                    ResolveCall(Block, Current->getCall(), Node->getNameSpace()->Functions)) {

                    // func()
                    Function = Current->getCall()->getDef();
                }
            } else if (Var != nullptr && Var->getType()->isIdentity() && ((ASTIdentityType *) Var->getType())->isClass() &&
                ResolveCall(Block, Current->getCall(), ((ASTClass *) ((ASTClassType *) Var->getType())->getDef())->Methods)) {

                // classInstance.method()
                // namespace.globalVar.method()
                Current->getCall()->Instance = S.Builder->CreateVarRef(Var);
                Function = Current->getCall()->getDef();
            } else if (Function != nullptr && Function->getType()->isIdentity() && ((ASTIdentityType *) Function->getType())->isClass() &&
                    ResolveCall(Block, Current->getCall(), ((ASTClass *) ((ASTClassType *) Function->getType())->getDef())->Methods)) {

                // method().method()
                Current->getCall()->Instance = Previous->getCall();
                Function = Current->getCall()->getDef();

                // namespace.function().function()
                // TODO
            } else if (IdentityType != nullptr && IdentityType->isClass()) {

                // ClassType.staticMethod() or NameSpace.ClassType.func()
                Var = ((ASTClass *) ((ASTClassType *) IdentityType)->getDef())->Vars.lookup(Current->getName());
                auto &Methods = ((ASTClass *) ((ASTClassType *) IdentityType)->getDef())->Methods;
                ResolveCall(Block, Current->getCall(), Methods);
                Function = Current->getCall()->getDef();
            } else {

                // NameSpace.func()
                ResolveCall(Block, Current->getCall(), Node->Functions) ||
                    ResolveCall(Block, Current->getCall(), NameSpace->Functions);
                Function = Current->getCall()->getDef();
            }

            if (Function == nullptr) {
                S.Diag(Current->getLocation(), diag::err_sema_resolve_identifier);
                return false;
            }

            Var = nullptr;
            // already resolved in ResolveCall()

        } else { // VarRef or IdentityType

            // only the first time
            if (Var == nullptr && Function == nullptr && IdentityType == nullptr && NameSpace == nullptr) {

                // Search for LocalVar
                Var = S.FindLocalVar(Block, Current);

                // Search for Class Vars if Var is Class Method
                if (Var == nullptr && Top->getKind() == ASTFunctionKind::CLASS_FUNCTION)
                    Var = ((ASTClassFunction *) Top)->getClass()->Vars.lookup(Current->getName());

                // Search for GlobalVars
                if (Var == nullptr)
                    Var = Node->getNameSpace()->GlobalVars.lookup(Current->getName());

                if (Var != nullptr) {
                    Current->Reference = S.Builder->CreateVarRef(Var);

                } else { // Current is IdentityType or GlobalVar

                    // Search for IdentityType
                    ASTIdentity *Identity = S.FindIdentity(Current->getName(), Node->getNameSpace());
                    if (Identity != nullptr) {
                        IdentityType = (ASTIdentityType *) Identity->getType();
                    } else {

                        // Search for NameSpace
                        ASTImport *Import = S.FindImport(Node, Identifier->getName());
                        if (Import) {
                            NameSpace = Import->getNameSpace();
                            Previous = Current;
                            Current = Current->getChild();
                            I++;
                            continue;
                        }
                    }
                }
            } else if (Var != nullptr && Var->getType()->isIdentity() && ((ASTIdentityType *) Var->getType())->isClass() &&
                ((ASTIdentityType *) Var->getType())->isClass()) {

                // classInstance.var
                Current->getVarRef()->Instance = S.Builder->CreateVarRef(Var);
                Var = ((ASTClass *) ((ASTClassType *) Var->getType())->getDef())->Vars.lookup(Current->getName());
                Current->getVarRef()->Def = Var;
            } else if (Function != nullptr && Function->getType()->isIdentity() &&
                ((ASTIdentityType *) Function->getType())->isClass() &&
                ((ASTIdentityType *) Function->getType())->isClass()) {

                // function().var
                Current->getVarRef()->Instance = Previous->getCall();
                Var = ((ASTClass *) ((ASTClassType *) Function->getType())->getDef())->Vars.lookup(Current->getName());
                Current->getVarRef()->Def = Var;
            } else if (IdentityType != nullptr) {

                // ClassType.STATIC_VAR
                // EnumType.ENUM_VAR
                // StructType.fieldVar
                Current->getVarRef()->Instance = nullptr;
                if (IdentityType->isClass()) {
                    Var = ((ASTClass *) ((ASTClassType *) IdentityType)->getDef())->Vars.lookup(Current->getName());
                } else if (IdentityType->isEnum()) {
                    Var = ((ASTEnum *) ((ASTEnumType *) IdentityType)->getDef())->Vars.lookup(Current->getName());
                }
                Current->getVarRef()->Def = Var;
                IdentityType = nullptr;
            } else {

                // NameSpace.GlobalVar
                Var = NameSpace->GlobalVars.lookup(Current->getChild()->getName());
                Current->getVarRef()->Def = Var;
            }

            // Resolve Var as IdentityType
//            if (Var == nullptr && ResolveIdentifier(Node, Current, NameSpace, IdentityType)) {
//                Previous = Current;
//                Current = Current->getChild();
//                I++;
//                continue;
//            }

            if (Var == nullptr && IdentityType == nullptr) {
                S.Diag(Current->getLocation(), diag::err_sema_resolve_identifier);
                return false;
            }

            Function = nullptr;
        }

        // Go next Identifier or break
        Previous = Current;
        Current = Current->getChild();
        I++;
    }

    return true;
}

bool SemaResolver::ResolveIdentityType(ASTNode *Node, ASTIdentityType *&IdentityType) {
    // Resolve Identifier
    if (IdentityType->getIdentifier()) {

        if (IdentityType->getIdentifier()->isCall()) {
            S.Diag(IdentityType->getLocation(), diag::err_sema_resolve_identifier);
            return false;
        }

        // Check only ClassType with only 1 Parent: NameSpace.IdentityName
        if (IdentityType->getIdentifier()->getIndex() > 1) {
            S.Diag(IdentityType->getLocation(), diag::err_sema_resolve_identifier);
            return false;
        }

        llvm::StringRef IdentityName = IdentityType->getIdentifier()->getName();
        ASTNameSpace *NameSpace = Node->NameSpace; // No namespace defined
        if (IdentityType->getIdentifier()->getRoot()) { // with namespace
            llvm::StringRef NS = IdentityType->getIdentifier()->getRoot()->getName(); // NameSpace.IdentityName
            NameSpace = S.FindNameSpace(NS);
        }
        ASTIdentity *Def = S.FindIdentity(IdentityName, NameSpace);
        delete IdentityType->Identifier;

        // Error: unreferenced class
        if (Def == nullptr) {
            S.Diag(diag::err_unref_class) << IdentityName;
            return false;
        }

        IdentityType = Def->getType();
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
    FLY_DEBUG_MESSAGE("Sema", "ResolveVarRef", Logger().Attr("VarRef", VarRef).End());
    
    if (VarRef->getIdentifier()) {

        if (VarRef->getIdentifier()->isCall()) {
            S.Diag(VarRef->getLocation(), diag::err_sema_resolve_identifier);
            return false;
        }

        // TODO simplify logic
        if (ResolveIdentifier(Block, VarRef->getIdentifier())) {
            VarRef->Def = VarRef->getIdentifier()->getVarRef()->Def;
        }
    }

    // VarRef not found in node, namespace and node imports
    if (VarRef->Def == nullptr) {
        S.Diag(VarRef->getLocation(), diag::err_unref_var);
        return false;
    }

    return true;
}

bool SemaResolver::ResolveCall(ASTBlock *Block, ASTCall *Call) {
    if (Call->getIdentifier()) {

        if (!Call->getIdentifier()->isCall()) {
            S.Diag(Call->getLocation(), diag::err_sema_resolve_identifier);
            return false;
        }

        ResolveIdentifier(Block, Call->getIdentifier());
    }

    if (!Call->Def) {
        S.Diag(Call->getLocation(), diag::err_unref_call);
        return false;
    }

    return true;
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
        for (T *Function: IntMapIt->second) {

            if (Function->getParams()->getSize() == Call->getArgs().size()) {
                bool Success = true; // if Params = Args = 0 skip for cycle
                for (unsigned long i = 0; i < Function->getParams()->getSize(); i++) {
                    // Resolve Arg Expr on first
                    ASTArg *Arg = Call->getArgs().at(i);
                    ASTParam *Param = Function->getParams()->at(i);
                    Success &= ResolveArg(Block, Arg, Param);
                }

                if (Success) {
                    if (Call->Def) { // Error: function defined more times
                        // TODO
                        return false;
                    }

                    Call->Def = Function;
                }
            }
        }
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
            if ((VarRef->getDef() || ResolveVarRef(Block, VarRef))) {
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
                        Call->Def->Type :
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
                            Success = S.Validator->CheckSameTypes(Binary->OpLoc, Binary->First->Type,
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
 * Get the Parent Block
 * @param Stmt
 * @return
 */
ASTBlock *SemaResolver::getBlock(ASTStmt *Stmt) {
    switch (Stmt->getKind()) {

        case ASTStmtKind::STMT_BLOCK:
            return (ASTBlock *) Stmt;
        case ASTStmtKind::STMT_RETURN:
        case ASTStmtKind::STMT_EXPR:
        case ASTStmtKind::STMT_VAR_DEFINE:
        case ASTStmtKind::STMT_VAR_ASSIGN:
            return (ASTBlock *) Stmt->getParent();
        case ASTStmtKind::STMT_BREAK:
        case ASTStmtKind::STMT_CONTINUE:
            assert("Unexpected parent for ASTExpr");
    }

    return nullptr;
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
            return ((ASTBlock *) Stmt->getParent())->Top->Type;
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
