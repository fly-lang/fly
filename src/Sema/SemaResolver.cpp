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
#include "AST/ASTNameSpace.h"
#include "AST/ASTNode.h"
#include "AST/ASTIfBlock.h"
#include "AST/ASTImport.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
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
        for (auto &StrMapEntry: Node->Class->Methods) {
            for (auto &IntMap: StrMapEntry.getValue()) {
                for (auto &Method: IntMap.second) {
                    Success &= ResolveBlock(Method->Body);
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
                Success &= ResolveType(LocalVar->getType());
                if (LocalVar->getExpr())
                    Success &= ResolveExpr(Block, LocalVar->getExpr());
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
    bool Success = ResolveBlock(ForBlock) && ResolveExpr(ForBlock->getParent(), ForBlock->Condition) &&
            S.Validator->CheckConvertibleTypes(ForBlock->Condition->Type, SemaBuilder::CreateBoolType(ForBlock->Condition->Loc));
    if (ForBlock->Post) {
        Success &= ResolveBlock(ForBlock->Post);
    }
    if (ForBlock->Loop) {
        Success &= ResolveBlock(ForBlock->Loop);
    }

    return Success;
}

bool SemaResolver::ResolveType(ASTType * Type) {
    ASTClassType * ClassType;
    if (Type->isClass())
        ClassType = (ASTClassType *) Type;
    if (Type->isArray() && ((ASTArrayType *) Type)->getType()->isArray())
        ClassType = ((ASTClassType *) ((ASTArrayType *) Type)->getType());
    else
        return true;

    ASTNameSpace *NameSpace = S.FindNameSpace(ClassType->getNameSpace());
    ASTClass *Class = S.FindClass(ClassType->getName(), NameSpace);
    if (!Class) {
        S.Diag(ClassType->getLocation(), diag::err_unref_type);
        return false;
    }

    return true;
}

bool SemaResolver::FindFunction(ASTBlock *Block, ASTCall *Call,
                                llvm::StringMapIterator<std::map<uint64_t, llvm::SmallVector<ASTFunction *, 4>>> StrMapIt) {
    std::map<uint64_t, llvm::SmallVector<ASTFunction *, 4>> &IntMap = StrMapIt->getValue();
    const auto &IntMapIt = IntMap.find(Call->getArgs().size());
    bool Success = false;
    if (IntMapIt != IntMap.end()) { // Node contains Function with this size of args
        for (ASTFunction *Function : IntMapIt->second) {
            Success = true;
            if (Function->getParams()->getSize() == Call->getArgs().size()) {
                for (unsigned long i = 0; i < Function->getParams()->getSize(); i++) {
                    // Resolve Arg Expr on first
                    ASTArg *Arg = Call->getArgs().at(i);
                    ASTParam *Param = Function->getParams()->at(i);
                    Success = ResolveArg(Block, Arg, Param);
                    if (!Success) continue; // try with next
                }
                // Set Function definition for Call
                if (Success) {
                    Call->Def = Function;
                    return true;
                }
            }
        }
    }
    return false;
}

bool SemaResolver::ResolveCall(ASTBlock *Block, ASTCall *Call) {
    if (!Call->Def) {
        assert(!Call->getNameSpace().empty() && "Call without NameSpace");
        const auto &Node = S.FindNode(Block->getTop());

        // Search into functions
        if (Call->getClassName().empty()) {
            
            // Find in current Node
            auto StrMapIt = Node->Functions.find(Call->getName());
            bool Success = FindFunction(Block, Call, StrMapIt);

            if (!Success && Call->getNameSpace() == Node->NameSpace->getName()) {
                // Find in current NameSpace
                StrMapIt = Node->NameSpace->Functions.find(Call->getName());

                if (FindFunction(Block, Call, StrMapIt)) {
                    ASTVisibilityKind Visibility = ((ASTFunction *) Call->getDef())->getScopes()->getVisibility();
                    Success = Visibility == ASTVisibilityKind::V_DEFAULT || Visibility == ASTVisibilityKind::V_PUBLIC;
                }
            }

            ASTImport *Import;
            if (!Success && (Import = Node->FindImport(Call->getNameSpace()))) {
                // Find in current Import
                StrMapIt = Import->NameSpace->Functions.find(Call->getName());

                if (FindFunction(Block, Call, StrMapIt)) {
                    ASTVisibilityKind Visibility = ((ASTFunction *) Call->getDef())->getScopes()->getVisibility();
                    Success = Visibility == ASTVisibilityKind::V_PUBLIC;
                }
            }

            if (!Success)
                S.Diag(Call->getLocation(), diag::err_unref_call);

            return Success;
        }
    }
    
    return true;
}

bool SemaResolver::ResolveArg(ASTBlock *Block, ASTArg *Arg, ASTParam *Param) {
    Arg->Def = Param;
    if (ResolveExpr(Block, Arg->Expr)) {
        return S.Validator->CheckConvertibleTypes(Arg->Expr->Type, Param->Type);
    }

    return false;
}

/**
 * Resolve a VarRef with its declaration
 * @param VarRef
 * @return true if no error occurs, otherwise false
 */
bool SemaResolver::ResolveVarRef(ASTBlock *Block, ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveVarRef", Logger().Attr("VarRef", VarRef).End());
    // Search into parameters // FIXME ?? Already present into Block->LocalVars
    for (auto &Param : Block->getTop()->getParams()->getList()) {
        if (VarRef->getName() == Param->getName()) {
            // Resolve with Param
            VarRef->Def = Param;
            break;
        }
    }

    // If VarRef is not resolved with parameters, search into Block declarations
    if (!VarRef->getDef()) {
        // Search recursively into current Block or in one of Parents
        ASTLocalVar *LocalVar = S.FindVarDef(Block, VarRef);
        // Check if var declaration var is resolved
        if (LocalVar) {
            VarRef->Def = LocalVar; // Resolved
        } else {
            const auto &Node = S.FindNode(Block->getTop());
            ASTImport *Import;
            if (VarRef->getNameSpace().empty()) {
                // Find in current Node
                VarRef->Def = Node->GlobalVars.lookup(VarRef->getName());
            } else if (VarRef->getNameSpace() == Node->NameSpace->getName()) {
                // Find in current NameSpace
                VarRef->Def = Node->NameSpace->GlobalVars.lookup(VarRef->getName());
            } else if ((Import = Node->FindImport(VarRef->getNameSpace()))) {
                // Find in current Import
                VarRef->Def = Import->getNameSpace()->getGlobalVars().lookup(VarRef->getName());
            }

            // Error: check unreferenced var
            // VarRef not found in node, namespace and node imports
            if (!VarRef->Def) {
                S.Diag(VarRef->getLocation(), diag::err_unref_var);
                return false;
            }

            Block->UndefVars.erase(VarRef->getName());
            return true;
        }
    }

    if (VarRef->getDef()) {

        // The Var is now well-defined: you can remove it from UndefVars
        if (Block->UndefVars.lookup(VarRef->getName())) {
            return Block->UndefVars.erase(VarRef->getName());
        }

        return true;
    }

    S.Diag(VarRef->getLocation(), diag::err_undef_var);
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
            ASTBlock *Block = getBlock(Expr->getStmt());
            ASTVarRef *VarRef = ((ASTVarRefExpr *)Expr)->getVarRef();
            if (S.Validator->CheckUndef(Block, VarRef) && (VarRef->getDef() || ResolveVarRef(Block, VarRef))) {
                Expr->Type = VarRef->getDef()->getType();
                Success = true;
                break;
            } else {
                return false;
            }
        }
        case ASTExprKind::EXPR_CALL: {
            ASTBlock *Block = getBlock(Expr->getStmt());
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
            return getBlock(Stmt)->Top->Type;
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
