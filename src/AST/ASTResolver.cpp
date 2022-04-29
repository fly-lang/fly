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
#include "AST/ASTImport.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTNode.h"
#include "AST/ASTUnref.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTBlock.h"
#include "Sema/Sema.h"
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
    Success &= ResolveImports(NameSpace) && ResolveGlobalVars(NameSpace) && ResolveFuncCalls(NameSpace);
    NameSpace->Context->Diags.getClient()->EndSourceFile();
    return Success;
}

bool ASTResolver::Resolve(ASTNode *Node) {
    FLY_DEBUG_MESSAGE("ASTResolver", "Resolve",
                      "NameSpace=" << Node->NameSpace->str() << ", Node=" << Node->str());
    return ResolveGlobalVars(Node) && ResolveFuncCalls(Node);
}

/**
 * Resolve Imports with relative Namespace
 * Sync Un-references from Import to Namespace for next resolving
 * @param Node
 * @return
 */
bool ASTResolver::ResolveImports(ASTNameSpace *NameSpace) {
    bool Success = true;
    for (auto &NodeEntry : NameSpace->Nodes) {
        ASTNode *&Node = NodeEntry.getValue();
        for (auto &ImportEntry : Node->getImports()) {

            // Search Namespace of the Import
            auto &Import = ImportEntry.getValue();
            ASTNameSpace *NameSpaceFound = NameSpace->Context->NameSpaces.lookup(Import->getName());

            if (NameSpaceFound == nullptr) { // Error
                Success = false;
                NameSpace->Context->Diag(diag::err_namespace_notfound) << Import->getName();
            } else {
                FLY_DEBUG_MESSAGE("ASTResolver", "ResolveImports",
                                  "Import=" << Import->getName() <<
                                            ", NameSpace=" << NameSpaceFound->getName());
                Import->setNameSpace(NameSpaceFound);

                // Sync Un-referenced GlobalVars
                for (auto &UnrefGlobalVar: Import->UnrefGlobalVars) {
                    NameSpaceFound->UnrefGlobalVars.push_back(UnrefGlobalVar);
                }

                // Sync Un-referenced FunctionCalls
                for (auto &UnrefFunctionCall: Import->UnrefFunctionCalls) {
                    NameSpaceFound->UnrefFunctionCalls.push_back(UnrefFunctionCall);
                }
            }
        }
    }
    return Success;
}

/**
 * Resolve GlobalVar into Node
 * @param Node
 * @return
 */
bool ASTResolver::ResolveGlobalVars(ASTNode *Node) {
    
    // Resolve Unreferenced Global Var (node internal)
    for (auto &Unref : Node->UnrefGlobalVars) {
        FLY_DEBUG_MESSAGE("ASTResolver", "ResolveGlobalVars",
                          "Node=" << Node->str() <<
                          ", VarRef=" << Unref->getVarRef().str());
        const auto &It = Node->GlobalVars.find(Unref->getVarRef().getName());
        if (It == Node->GlobalVars.end()) {
            // of the current NameSpace
            Node->NameSpace->UnrefGlobalVars.push_back(Unref);
        } else {
            ASTVarRef &VarRef = Unref->getVarRef();
            ASTGlobalVar *Var = It->getValue();
            VarRef.setDecl(Var);
        }
    }

    return true;
}

/**
 * Resolve GlobalVar into NameSpace
 * @param NameSpace
 * @return
 */
bool ASTResolver::ResolveGlobalVars(ASTNameSpace *NameSpace) {
    bool Success = true;

    // Resolve Unreferenced Global Var (node internal)
    for (auto &Unref : NameSpace->UnrefGlobalVars) {
        FLY_DEBUG_MESSAGE("ASTResolver", "ResolveGlobalVars",
                          "NameSpace=" << NameSpace->str() <<
                          ", VarRef=" << Unref->getVarRef().str());
        const auto &It = NameSpace->GlobalVars.find(Unref->getVarRef().getName());
        if (It == NameSpace->GlobalVars.end()) {
            NameSpace->Context->Diag(Unref->getVarRef().getLocation(), diag::err_gvar_notfound)
                        << Unref->getVarRef().getName();
            Success = false;
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
bool ASTResolver::ResolveFuncCalls(ASTNode *Node) {
    bool Success = true;

    // Skip Function Reference to libc
    bool IsBaseLib = Node->getNameSpace()->getName().find("fly.base.") == 0;

    // Resolve Unreferenced Function Calls (node internal)
    for (auto *UnrefFunctionCall : Node->UnrefFunctionCalls) {
        FLY_DEBUG_MESSAGE("ASTResolver", "ResolveFuncCalls",
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
bool ASTResolver::ResolveFuncCalls(ASTNameSpace *NameSpace) {
    bool Success = true;

    // Resolve Unreferenced Function Calls (at namespace level)
    for (auto *UnrefFunctionCall : NameSpace->UnrefFunctionCalls) {
        FLY_DEBUG_MESSAGE("ASTResolver", "ResolveFuncCalls",
                          "NameSpace=" << NameSpace->Name <<
                          ", UnrefFunctionCall=" << UnrefFunctionCall->getCall()->str());
        // Auto resolve in Lib
        if (NameSpace->isExternalLib()) {
            // TODO
            // need to to read the prototype for external function declaration
            // UnrefFunctionCall->getCall()->setDecl(Func);
            // UnrefFunctionCall->getNode()->AddExternalFunction(Func);
        }

        // Resolve with Sources
        const auto &It = NameSpace->FunctionCalls.find(UnrefFunctionCall->getCall()->getName());
        if (It == NameSpace->FunctionCalls.end()) {
            NameSpace->Context->Diag(UnrefFunctionCall->getCall()->getLocation(), diag::err_unref_call)
                    << UnrefFunctionCall->getCall()->getName();
            Success = false; // collects other errors
        } else {
            for (auto &FunctionCall : It->getValue()) {
                if (FunctionCall->isUsable(UnrefFunctionCall->getCall())) {
                    UnrefFunctionCall->getCall()->setDecl(FunctionCall->getDecl());
                    UnrefFunctionCall->getNode()->AddExternalFunction(FunctionCall->getDecl()); // Call resolved with external function
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
    FLY_DEBUG_MESSAGE("ASTResolver", "ResolveExprType","Expr=" << Expr->str());
    return Expr->getType();
}

/**
 * Search a VarRef into declared Block's vars
 * If found set LocalVar
 * @param Block
 * @param LocalVar
 * @param VarRef
 * @return the found LocalVar
 */
ASTLocalVar *ASTResolver::FindVarDecl(ASTBlock *Block, ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("ASTResolver", "FindVarDecl", "VarRef=" << VarRef->str());
    const auto &It = Block->getLocalVars().find(VarRef->getName());
    if (It != Block->getLocalVars().end()) { // Search into this Block
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
bool ASTResolver::ResolveVarRef(ASTBlock *Block, ASTVarRef *VarRef) {
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
        if (LocalVar) {
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
bool ASTResolver::ResolveExpr(ASTBlock *Block, const ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("ASTResolver", "ResolveExpr", "Expr=" << Expr->str());
    switch (Expr->getKind()) {
        case EXPR_REF_VAR: {
            ASTVarRef *Var = ((ASTVarRefExpr *)Expr)->getVarRef();
            return Sema::CheckUndefVar(Block, Var) && (Var->getDecl() || ResolveVarRef(Block, Var));
        }
        case EXPR_REF_FUNC: {
            ASTFuncCall *Call = ((ASTFuncCallExpr *)Expr)->getCall();
            return Call->getDecl() || Block->getTop()->getNode()->AddUnrefCall(Call);
        }
        case EXPR_GROUP: {
            ASTGroupExpr *GroupExpr = (ASTGroupExpr *) Expr;
            switch (GroupExpr->getGroupKind()) {

                case GROUP_UNARY:
                    return ResolveExpr(Block, (ASTExpr *)((ASTUnaryGroupExpr *)GroupExpr)->getFirst());
                case GROUP_BINARY:
                    return ResolveExpr(Block, ((ASTBinaryGroupExpr *)GroupExpr)->getFirst()) &&
                            ResolveExpr(Block, ((ASTBinaryGroupExpr *)GroupExpr)->getSecond());
                case GROUP_TERNARY:
                    return ResolveExpr(Block, ((ASTTernaryGroupExpr *)GroupExpr)->getFirst()) &&
                           ResolveExpr(Block, ((ASTTernaryGroupExpr *)GroupExpr)->getSecond()) &&
                            ResolveExpr(Block, ((ASTTernaryGroupExpr *)GroupExpr)->getThird());
            }
        }
        case EXPR_VALUE:
            return true;
    }

    assert(0 && "Invalid ASTExprKind");
}