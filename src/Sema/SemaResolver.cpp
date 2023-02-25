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
#include "Sema/SemaValidator.h"
#include "AST/ASTContext.h"
#include "AST/ASTClassFunction.h"
#include "AST/ASTClassVar.h"
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
        Success &= ResolveImports(Node) & // resolve Imports with NameSpaces
                ResolveFunctions(Node) &  // resolve ASTBlock of Body Functions
                ResolveClass(Node);       // resolve Class attributes and methods
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
                              "Import=" << Import->getName() <<
                                        ", NameSpace=" << NameSpaceFound->getName());
            Import->setNameSpace(NameSpaceFound);

        } else {
            // Error: NameSpace not found
            Success = false;
            S.Diag(Import->NameLocation, diag::err_namespace_notfound) << Import->getName();
        }
    }

    return Success;
}

bool SemaResolver::ResolveClass(ASTNode *Node) {
    bool Success = true;
    if (Node->Class) {

        // Constructors
        for (auto &IntMap: Node->Class->Constructors) {
            for (auto &F: IntMap.second) {
                Success &= ResolveBlock(F->Body);
            }
        }

        // Methods
        for (auto &StrMapEntry: Node->Class->Methods) {
            for (auto &IntMap: StrMapEntry.getValue()) {
                for (auto &F: IntMap.second) {
                    Success &= ResolveBlock(F->Body);
                }
            }
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
                Success &= ResolveType(Block->getTop(), LocalVar->getType());

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
                    auto It = Block->UnInitVars.find(VarAssign->getVarRef()->getName());
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
            S.Diag(UnInitVar.second->getLocation(), diag::err_sema_uninit_var);
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
    bool Success = ResolveExpr(SwitchBlock->getParent(), SwitchBlock->Expr) && S.Validator->CheckMacroType(SwitchBlock->Expr->Type, ASTMacroTypeKind::MACRO_TYPE_INTEGER);
    for (ASTSwitchCaseBlock *Case : SwitchBlock->Cases) {
        Success &= ResolveExpr(SwitchBlock, Case->Expr) &&
                   S.Validator->CheckMacroType(SwitchBlock->Expr->Type, ASTMacroTypeKind::MACRO_TYPE_INTEGER) && ResolveBlock(Case);
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

bool SemaResolver::ResolveIdentifier(ASTNode *Node, ASTIdentifier *Identifier, ASTNameSpace *&NameSpace) {
    const llvm::StringRef &Name = Identifier->getName();
    // Search Identifier in Namespaces
    if (NameSpace == nullptr) {
        NameSpace = Node->getNameSpace();
        if (Name != Node->getNameSpace()->getName()) {
            ASTImport *Import = S.FindImport(Node, Name);
            if (Import) {
                NameSpace = Import->getNameSpace();
            }
        }
        return true;
    }
    return false;
}

bool SemaResolver::ResolveIdentifier(ASTNode *Node, ASTIdentifier *Identifier, ASTNameSpace *NameSpace,
                                     ASTClassType *&ClassType) {
    const llvm::StringRef &Name = Identifier->getName();
    // Search Identifier in ClasTypes
    if (ClassType == nullptr) {
        ASTClass *Class = S.FindClass(Name, NameSpace ? NameSpace : Node->getNameSpace());
        if (Class) {
            ClassType = S.Builder->CreateClassType(Class);
            return true;
        }
    }
    return false;
}

bool SemaResolver::ResolveIdentifiers(ASTBlock *Block, ASTReference *Ref) {
    ASTFunctionBase *Base = Block->getTop();
    const auto &Node = S.FindNode(Base);

    ASTIdentifier *Identifier = Ref->getIdentifier();
    ASTIdentifier *Current = Identifier->getIndex() > 0 ? Identifier->getRoot() : Identifier;
    ASTIdentifier *Previous = nullptr;

    ASTNameSpace *NameSpace = nullptr;
    ASTClassType *ClassType = nullptr;

    ASTVar *Var = nullptr;
    ASTFunctionBase *Function = nullptr;

    uint32_t I = Current->getIndex();
    uint32_t Size = Identifier->getIndex() + 1;
    while (I < Size) {

        if (Current->isCall()) {
            if (Var == nullptr && Function == nullptr) {
                if (Base->getKind() == ASTFunctionKind::FUNCTION &&
                    (ResolveCall(Block, Current->getCall(), Node->Functions) ||
                        ResolveCall(Block, Current->getCall(), Node->getNameSpace()->Functions))) {
                    // func()
                    Function = Current->getCall()->getDef();
                } else if (Base->getKind() == ASTFunctionKind::CLASS_FUNCTION &&
                        ResolveCall(Block, Current->getCall(), ((ASTClassFunction *) Base)->getClass()->Methods)) {
                    // method()
                    Function = Current->getCall()->getDef();
                }
            } else if (Var != nullptr && Var->getType()->isClass() &&
                       ResolveCall(Block, Current->getCall(), ((ASTClassType *) Var->getType())->getDef()->Methods)) {
                // var.method()
                Current->getCall()->Instance = S.Builder->CreateVarRef(Var);
                Function = Current->getCall()->getDef();
            } else if (Function != nullptr && Function->getType()->isClass() &&
                    ResolveCall(Block, Current->getCall(), ((ASTClassType *) Function->getType())->getDef()->Methods)) {
                // method().method()
                Current->getCall()->Instance = Previous->getCall();
                Function = Current->getCall()->getDef();
            } else if (ResolveIdentifier(Node, Current, NameSpace, ClassType) &&
                       ResolveCall(Block, Current->getCall(), ClassType->getDef()->Methods)) {
                // NameSpace.ClassType.func()
                Function = Current->getCall()->getDef();
            } else if (ResolveIdentifier(Node, Current, NameSpace) &&
                    (ResolveCall(Block, Current->getCall(), Node->Functions) ||
                        ResolveCall(Block, Current->getCall(), NameSpace->Functions))) {
                // NameSpace.func()
                Function = Current->getCall()->getDef();
            } else {
                S.Diag(Current->getLocation(), diag::err_sema_invalid_identifier);
                return false;
            }

            if (Function == nullptr) {
                S.Diag(Current->getLocation(), diag::err_sema_invalid_identifier);
                return false;
            }

        } else {
            if (Var == nullptr && Function == nullptr) {
                // Search for LocalVar
                Var = S.FindLocalVar(Block, Current);

                // Search for ClassVars
                if (Var == nullptr && Base->getKind() == ASTFunctionKind::CLASS_FUNCTION) {
                    Var = ((ASTClassFunction *) Base)->getClass()->Vars.lookup(Current->getName());
                }

                // Search for GlobalVars
                if (Var == nullptr)
                    Var = Node->getNameSpace()->GlobalVars.lookup(Current->getName());
            } else if (Var != nullptr && Var->getType()->isClass()) {
                // var.var
                Current->getVarRef()->Instance = S.Builder->CreateVarRef(Var);
                Var = ((ASTClassType *) Var->getType())->getDef()->Vars.lookup(Current->getName());
            } else if (Function != nullptr && Function->getType()->isClass()) {
                // method().var
                Current->getVarRef()->Instance = Previous->getCall();
                Var = ((ASTClassType *) Function->getType())->getDef()->Vars.lookup(Current->getName());
            } else if (ResolveIdentifier(Node, Current, NameSpace, ClassType)) {
                // NameSpace.ClassType.var
                Var = ClassType->getDef()->Vars.lookup(Current->getName());
            } else if (ResolveIdentifier(Node, Current, NameSpace)) {
                // NameSpace.var
                Var = NameSpace->GlobalVars.lookup(Current->getChild()->getName());
            } else {
                S.Diag(Current->getLocation(), diag::err_sema_invalid_identifier);
                return false;
            }

            if (Var == nullptr) {
                S.Diag(Current->getLocation(), diag::err_sema_invalid_identifier);
                return false;
            }
        }

        // set ClassType from Var or Function return
        if (Var) {
            Current->getVarRef()->Def = Var;
            if (Var->getType()->isClass())
                ClassType = (ASTClassType *) Var->getType();
        } else if (Function && Function->getType()->isClass()) {
            ClassType = (ASTClassType *) Function->getType();
        }

        // Go next Identifier or break
        Previous = Current;
        Current = Current->getChild();
        I++;
    }

    return true;
}

bool SemaResolver::ResolveType(ASTFunctionBase *FunctionBase, ASTType * Type) {
    ASTClassType * ClassType;
    if (Type->isClass())
        ClassType = (ASTClassType *) Type;
    else if (Type->isArray() && ((ASTArrayType *) Type)->getType()->isArray()) // FIXME nested array
        ClassType = ((ASTClassType *) ((ASTArrayType *) Type)->getType());
    else // is primitive type
        return true;

    // Take Node
    const auto &Node = S.FindNode(FunctionBase);

    // Resolve Identifier
    if (ClassType->getIdentifier()) {

        if (ClassType->getIdentifier()->isCall()) {
            S.Diag(ClassType->getLocation(), diag::err_sema_invalid_identifier);
            return false;
        }

        // Check only ClassType with only 1 Parent: NameSpace.ClassName
        if (ClassType->getIdentifier()->getIndex() > 1) {
            S.Diag(ClassType->getLocation(), diag::err_sema_invalid_identifier);
            return false;
        }

        llvm::StringRef ClassName = ClassType->getIdentifier()->getName();
        ASTNameSpace *NameSpace = Node->NameSpace; // No namespace defined
        if (ClassType->getIdentifier()->getRoot()) { // with namespace
            llvm::StringRef NS = ClassType->getIdentifier()->getRoot()->getName(); // NameSpace.ClassName
            NameSpace = S.FindNameSpace(NS);
        }
        ClassType->Def = S.FindClass(ClassName, NameSpace);
        delete ClassType->Identifier;
    }

    if (!ClassType->Def) {
        S.Diag(ClassType->getLocation(), diag::err_unref_type);
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
            S.Diag(VarRef->getLocation(), diag::err_sema_invalid_identifier);
            return false;
        }

        ResolveIdentifiers(Block, VarRef);
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
            S.Diag(Call->getLocation(), diag::err_sema_invalid_identifier);
            return false;
        }

        ResolveIdentifiers(Block, Call);

        // visibility error
//        ASTVisibilityKind Visibility = ((ASTFunction *) Call->getDef())->getScopes()->getVisibility();
//        if (Visibility == ASTVisibilityKind::V_PRIVATE) {
//            S.Diag(Call->getLocation(), diag::err_sema_invalid_identifier);
//            return false;
//        }

        // class from instance TODO
//        ASTLocalVar *Instance = S.FindVarDef(Block, Call->getClassName());
//        if (Instance && Instance->getType()->isClass()) {
//            Call->Instance = Instance;
//
//            // Class i
//            // i.func()
//            if (ResolveCall(Block, Call, ((ASTClassType *) Instance->getType())->getDef()->Methods)) {
//                ASTClassVisibilityKind Visibility = ((ASTClassFunction *) Call->Def)->getScopes()->getVisibility();
//                if (Visibility == ASTClassVisibilityKind::CLASS_V_PUBLIC ||
//                    Visibility == ASTClassVisibilityKind::CLASS_V_DEFAULT) {
//                    return true;
//                }
//            }
//        }
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
            if ((VarRef->getDef() || ResolveVarRef(Block, VarRef)) && S.Validator->CheckUninitialized(Block, VarRef)) {
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
                Expr->Type = Call->Def->Type;
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

                    if (Binary->First->Kind == ASTExprKind::EXPR_EMPTY) {
                        // Error: Binary cannot contain ASTEmptyExpr
                        S.Diag(Binary->First->Loc, diag::err_sema_empty_expr);
                        return false;
                    }

                    Success = ResolveExpr(Block, Binary->First) && ResolveExpr(Block, Binary->Second);
                    if (Success) {
                        switch(Binary->getOptionKind()) {

                            case ASTBinaryOptionKind::BINARY_ARITH: {
                                Success = S.Validator->CheckArithTypes(Binary->OpLoc, Binary->First->Type, Binary->Second->Type);

                                // Selects the largest data Type
                                Expr->Type = Binary->First->Type->Kind > Binary->Second->Type->Kind ?
                                             Binary->First->Type :
                                             Binary->Second->Type;

                                // Promotes First or Second Expr Types in order to be equal
                                Binary->First->Type = Expr->Type;
                                Binary->Second->Type = Expr->Type;
                                break;
                            }

                            case ASTBinaryOptionKind::BINARY_LOGIC: {
                                Success = S.Validator->CheckLogicalTypes(Binary->OpLoc,
                                                              Binary->First->Type, Binary->Second->Type);
                                Binary->Type = SemaBuilder::CreateBoolType(Expr->Loc);
                                break;
                            }

                            case ASTBinaryOptionKind::BINARY_COMPARISON: {
                                Binary->Type = SemaBuilder::CreateBoolType(Expr->Loc);

                                // Better Case
                                if (Binary->First->Type->Kind == Binary->Second->Type->Kind) {
                                    Success = true;
                                    break;
                                }

                                if (Binary->First->Type->MacroKind == Binary->Second->Type->MacroKind) {
                                    // 1 == 20 (unsigned, unsigned)
                                    // -1 == 20 (signed, unsigned)
                                    // 1 == -20 (unsigned, signed)
                                    // -1 == -20 (signed, signed)
                                    if (Binary->First->Kind == ASTExprKind::EXPR_VALUE && Binary->Second->Kind == ASTExprKind::EXPR_VALUE) {
                                        // chose between the biggest type
                                        Binary->First->Type = Binary->Second->Type =
                                                Binary->First->Type->Kind > Binary->Second->Type->Kind ?
                                                Binary->First->Type : Binary->Second->Type;
                                        Success = true;
                                        break;
                                    }

                                    // 1 == a
                                    else if (Binary->First->Kind == ASTExprKind::EXPR_VALUE && Binary->Second->Kind != ASTExprKind::EXPR_VALUE &&
                                            Binary->First->Type->Kind < Binary->Second->Type->Kind) {
                                        Binary->First->Type = Binary->Second->Type;
                                        Success = true;
                                        break;
                                    }

                                    // a == 1
                                    else if (Binary->First->Kind != ASTExprKind::EXPR_VALUE && Binary->Second->Kind == ASTExprKind::EXPR_VALUE &&
                                            Binary->First->Type->Kind > Binary->Second->Type->Kind) {
                                        Binary->Second->Type = Binary->First->Type;
                                        Success = true;
                                        break;
                                    }
                                }

                                S.Diag(Binary->OpLoc, diag::err_sema_types_comparable)
                                        << Binary->First->Type->print()
                                        << Binary->Second->Type->print();
                                return false;
                            }
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
                    break;
                }
            }
            break;
        }
        default:
            assert(0 && "Invalid ASTExprKind");
    }

    // The last Expr before Stmt need a Check Type
    if (Success && !Expr->Parent && Expr->Stmt && Expr->Stmt->Kind != ASTStmtKind::STMT_EXPR && Expr->Type) {
        return S.Validator->CheckConvertibleTypes(Expr->Type, getType(Expr->Stmt));
    }

    return Success;
}

bool SemaResolver::ResolveValueExpr(ASTValueExpr *Expr) {
    const SourceLocation &Loc = Expr->Value->getLocation();
    
    switch (Expr->Value->getMacroKind()) {
        
        case ASTMacroTypeKind::MACRO_TYPE_BOOL:
            Expr->Type = SemaBuilder::CreateBoolType(Loc);
            break;
            
        case ASTMacroTypeKind::MACRO_TYPE_INTEGER: {
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
        
        case ASTMacroTypeKind::MACRO_TYPE_FLOATING_POINT:
            // Creating as Float on first but transform in Double if is contained into a Binary Expr with a Double Type
            Expr->Type = SemaBuilder::CreateDoubleType(Loc);
            break;
        
        case ASTMacroTypeKind::MACRO_TYPE_ARRAY:
            // TODO
            break;
        case ASTMacroTypeKind::MACRO_TYPE_CLASS:
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
