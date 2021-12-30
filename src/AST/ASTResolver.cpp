//
// Created by marco on 10/14/21.
//
//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTResolver.cpp - Resolve AST
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include <AST/ASTContext.h>
#include "AST/ASTResolver.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTNode.h"
#include "AST/ASTUnref.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTBlock.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"
#include "llvm/ADT/StringMap.h"

using namespace fly;

/**
 * Take all unreferenced Global Variables from Functions and try to resolve them
 * into all NameSapces
 * @return true if no error occurs, otherwise false
 */
bool fly::ASTResolver::Resolve(ASTNameSpace *NameSpace) {
    FLY_DEBUG_MESSAGE("ASTResolver", "Resolve","NameSpace=" << NameSpace->Name);
    bool Success = true;

    NameSpace->Context->Diags.getClient()->BeginSourceFile();
    Success &= ResolveGlobalVar(NameSpace) && ResolveFuncCall(NameSpace);
    NameSpace->Context->Diags.getClient()->EndSourceFile();
    return Success;
}

bool ASTResolver::Resolve(ASTNode *Node) {
    FLY_DEBUG_MESSAGE("ASTResolver", "Resolve",
                      "NameSpace=" << Node->NameSpace->str() << ", Node=" << Node->str());
    return ResolveGlobalVar(Node) && ResolveFuncCall(Node);
}


/**
 * Resolve GlobalVar into Node
 * @param Node
 * @return
 */
bool ASTResolver::ResolveGlobalVar(ASTNode *Node) {
    // Resolve Unreferenced Global Var (node internal)
    for (auto &Unref : Node->UnrefGlobalVars) {
        FLY_DEBUG_MESSAGE("ASTResolver", "ResolveGlobalVar",
                          "Node=" << Node->str() <<
                          ", VarRef=" << Unref->getVarRef().str());
        const auto &It = Node->GlobalVars.find(Unref->getVarRef().getName());
        if (It == Node->GlobalVars.end()) {
            Node->NameSpace->UnrefGlobalVars.push_back(Unref);
        } else {
            ASTVarRef &VarRef = Unref->getVarRef();
            VarRef.setDecl(It->getValue());
        }
    }

    return true;
}

/**
 * Resolve GlobalVar into NameSpace
 * @param NameSpace
 * @return
 */
bool ASTResolver::ResolveGlobalVar(ASTNameSpace *NameSpace) {
    bool Success = true;

    // Resolve Unreferenced Global Var (node internal)
    for (auto &Unref : NameSpace->UnrefGlobalVars) {
        FLY_DEBUG_MESSAGE("ASTResolver", "ResolveGlobalVar",
                          "NameSpace=" << NameSpace->str() <<
                          ", VarRef=" << Unref->getVarRef().str());
        const auto &It = NameSpace->GlobalVars.find(Unref->getVarRef().getName());
        if (It == NameSpace->GlobalVars.end()) {
            NameSpace->Context->Diag(Unref->getVarRef().getLocation(), diag::err_gvar_notfound)
                        << Unref->getVarRef().getName();
        } else {
            Unref->getVarRef().setDecl(It->getValue());
            Unref->getNode()->AddExternalGlobalVar(It->getValue());
        }
    }

    return Success;
}

/**
 * Resolve Function into Node
 * @param Function
 * @return
 */
bool ASTResolver::ResolveFuncCall(ASTNode *Node) {
    bool Success = true;

    // Skip Function Reference to libc
    bool IsBaseLib = Node->getNameSpace()->getName().find("fly.base.") == 0;

    // Resolve Unreferenced Function Calls (node internal)
    for (auto *UnrefFunctionCall : Node->UnrefFunctionCalls) {
        FLY_DEBUG_MESSAGE("ASTResolver", "ResolveFuncCall",
                          "Node=" << Node->str() <<
                          ", UnrefFunctionCall=" << UnrefFunctionCall->getCall()->str());
        if (IsBaseLib && UnrefFunctionCall->getCall()->getName().find("c__") == 0) {
            continue; // TODO UnrefFunctionCalls can be checked with all possible libc functions
        } else {
            const auto &It = Node->FunctionCalls.find(UnrefFunctionCall->getCall()->getName());
            if (It == Node->FunctionCalls.end()) {
                Node->NameSpace->UnrefFunctionCalls.push_back(UnrefFunctionCall);
            } else {
                for (auto &FunctionCall: It->getValue()) {
                    if (FunctionCall->isUsable(UnrefFunctionCall->getCall())) {
                        UnrefFunctionCall->getCall()->setDecl(FunctionCall->getDecl());
                    } else {
                        Success = false;
                    }
                }
                if (!Success) { // Not Found
                    Node->NameSpace->Context->Diag(UnrefFunctionCall->getCall()->getLocation(), diag::err_unref_call)
                            << UnrefFunctionCall->getCall()->getName();
                }
            }
        }
    }
    return Success;
}

/**
 * Resolve Function into NameSpace
 * @param Function
 * @return
 */
bool ASTResolver::ResolveFuncCall(ASTNameSpace *NameSpace) {
    bool Success = true;

    // Resolve Unreferenced Function Calls (node internal)
    for (auto *UnrefFunctionCall : NameSpace->UnrefFunctionCalls) {
        FLY_DEBUG_MESSAGE("ASTResolver", "ResolveFuncCall",
                          "NameSpace=" << NameSpace->Name <<
                          ", UnrefFunctionCall=" << UnrefFunctionCall->getCall()->str());
        const auto &It = NameSpace->FunctionCalls.find(UnrefFunctionCall->getCall()->getName());
        if (It == NameSpace->FunctionCalls.end()) {
            NameSpace->Context->Diag(UnrefFunctionCall->getCall()->getLocation(), diag::err_unref_call)
                    << UnrefFunctionCall->getCall()->getName();
            Success = false; // collects other errors
        } else {
            for (auto &FunctionCall : It->getValue()) {
                if (FunctionCall->isUsable(UnrefFunctionCall->getCall())) {
                    UnrefFunctionCall->getCall()->setDecl(FunctionCall->getDecl());
                    UnrefFunctionCall->getNode()->AddExternalFunction(FunctionCall->getDecl());
                } else {
                    Success = false;
                }
            }
            if (!Success) { // Not Found
                NameSpace->Context->Diag(UnrefFunctionCall->getCall()->getLocation(), diag::err_unref_call)
                        << UnrefFunctionCall->getCall()->getName();
            }
        }
    }
    return Success;
}

ASTType *ASTResolver::ResolveExprType(ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("ASTResolver", "ResolveExprType",
                      "Expr=" << Expr->str());
    switch (Expr->getKind()) {

        case EXPR_VALUE:
            return ((ASTValueExpr *) Expr)->getValue().getType();
        case EXPR_REF_VAR:
            return ((ASTVarRefExpr *) Expr)->getVarRef()->getDecl()->getType();
        case EXPR_REF_FUNC:
            return ((ASTFuncCallExpr *) Expr)->getCall()->getDecl()->getType();
        case EXPR_GROUP:
            return ResolveExprType(((ASTGroupExpr *) Expr)->getGroup().at(0));
    }
    return nullptr;
}

/**
 * Search a VarRef into declared Block's vars
 * If found set LocalVar
 * @param Block
 * @param LocalVar
 * @param VarRef
 * @return the found LocalVar
 */
ASTLocalVar *ASTResolver::FindVarDecl(const ASTBlock *Block, ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("ASTResolver", "FindVarDecl", "VarRef=" << VarRef->str());
    const auto &It = Block->getDeclVars().find(VarRef->getName());
    if (It != Block->getDeclVars().end()) { // Search into this Block
        FLY_DEBUG_MESSAGE("ASTResolver", "FindVarDecl", "Found=" << It->getValue()->str());
        return It->getValue();
    } else if (Block->getParent()) { // Traverse Parent Block to find the right VarDeclStmt
        return FindVarDecl(Block->getParent(), VarRef);
    }
    return nullptr;
}

/**
 * Resolve a VarRef with its declaration
 * @param VarRef
 * @return true if no error occurs, otherwise false
 */
bool ASTResolver::ResolveVarRef(const ASTBlock *Block, ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("ASTResolver", "ResolveVarRef", "VarRef=" << VarRef->str());
    // Search into parameters
    for (auto &Param : Block->getTop()->getHeader()->getParams()) {
        if (VarRef->getName() == Param->getName()) {
            // Resolve with Param
            VarRef->setDecl(Param);
            break;
        }
    }

    // If VarRef is not resolved with parameters, search into declaration
    if (VarRef->getDecl() == nullptr) {
        // Search recursively into current Block or in one of Parents
        ASTLocalVar *LocalVar = FindVarDecl(Block, VarRef);
        // Check if var declaration var is resolved
        if (LocalVar != nullptr) {
            VarRef->setDecl(LocalVar); // Resolved
        } else {
            Block->getTop()->getNode()->AddUnrefGlobalVar(VarRef); // Resolve Later by searching into Node GlobalVars
        }
    }
    return true;
}

/**
 * Resolve Expr contents
 * @param Expr
 * @return true if no error occurs, otherwise false
 */
bool ASTResolver::ResolveExpr(const ASTBlock *Block, ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("ASTResolver", "ResolveExpr", "Expr=" << Expr->str());
    switch (Expr->getKind()) {
        case EXPR_REF_VAR: {
            ASTVarRef *Var = ((ASTVarRefExpr *)Expr)->getVarRef();
            return Var->getDecl() || ResolveVarRef(Block, Var);
        }
        case EXPR_REF_FUNC: {
            ASTFuncCall *Call = ((ASTFuncCallExpr *)Expr)->getCall();
            return Call->getDecl() || Block->getTop()->getNode()->AddUnrefCall(Call);
        }
        case EXPR_GROUP: {
            bool Result = true;
            for (auto &GroupExpr : ((ASTGroupExpr *)Expr)->getGroup()) {
                Result &= ResolveExpr(Block, GroupExpr);
            }
            return Result;
        }
        case EXPR_VALUE:
            return true;
        case EXPR_OPERATOR:
            return true;
    }

    assert(0 && "Invalid ASTExprKind");
}