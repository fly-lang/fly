//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/Sema.cpp - GlobalVar Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaResolver.h"
#include "Sema/SemaBuilder.h"
#include "AST/ASTContext.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTNode.h"
#include "AST/ASTImport.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunctionCall.h"
#include "AST/ASTParams.h"
#include "AST/ASTBlock.h"
#include "AST/ASTVar.h"
#include "AST/ASTVarAssign.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"

using namespace fly;

SemaResolver::SemaResolver(Sema &S, SemaBuilder &Builder) : S(S), Builder(Builder) {

}

/**
 * Take all unreferenced Global Variables from Functions and try to resolve them
 * into all NameSapces
 * @return
 */
bool SemaResolver::Resolve() {
    bool Success = true;

    // Resolve Node UnRefGlobalVars and UnRefCalls
    for (auto &NEntry : S.Context->getNodes()) {
        auto &Node = NEntry.getValue();
        Success &= ResolveGlobalVars(Node) & ResolveFunctionCalls(Node) & ResolveFunctions(Node) & ResolveClass(Node);
    }

    // Resolve UnRefGlobalVars and UnRefCalls at NameSpace level
    for (auto &NSEntry : S.Context->NameSpaces) {
        auto &NameSpace = NSEntry.getValue();
        Success &= ResolveImports(NameSpace) & ResolveGlobalVars(NameSpace) & ResolveFunctionCalls(NameSpace);
    }

    // Now all Imports must be read
    for(auto &Import : S.Context->ExternalImports) {
        if (!Import.getValue()->getNameSpace()) {
            Diag(Import.getValue()->getNameLocation(), diag::err_unresolved_import);
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
bool SemaResolver::ResolveImports(ASTNameSpace *NameSpace) {
    bool Success = true;

    for (auto &NodeEntry : NameSpace->Nodes) {
        ASTNode *&Node = NodeEntry.getValue();
        for (auto &ImportEntry : Node->getImports()) {

            // Search Namespace of the Import
            auto &Import = ImportEntry.getValue();
            ASTNameSpace *NameSpaceFound = NameSpace->Context->NameSpaces.lookup(Import->getName());

            if (!NameSpaceFound) { // Error
                Success = false;
                Diag(diag::err_namespace_notfound) << Import->getName();
            } else {
                FLY_DEBUG_MESSAGE("Sema", "ResolveImports",
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
 * Resolve GlobalVar from Node
 * @param Node
 * @return
 */
bool SemaResolver::ResolveGlobalVars(ASTNode *Node) {

    // Resolve Unreferenced Global Var (node internal)
    for (auto &Unref : Node->UnrefGlobalVars) {
        FLY_DEBUG_MESSAGE("Sema", "ResolveGlobalVars",
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
bool SemaResolver::ResolveGlobalVars(ASTNameSpace *NameSpace) {
    bool Success = true;

    // Resolve Unreferenced Global Var (node internal)
    for (auto &Unref : NameSpace->UnrefGlobalVars) {
        FLY_DEBUG_MESSAGE("Sema", "ResolveGlobalVars",
                          "NameSpace=" << NameSpace->str() <<
                                       ", VarRef=" << Unref->getVarRef().str());
        const auto &It = NameSpace->GlobalVars.find(Unref->getVarRef().getName());
        if (It == NameSpace->GlobalVars.end()) {
            Diag(Unref->getVarRef().getLocation(), diag::err_gvar_notfound)
                    << Unref->getVarRef().getName();
            Success = false;
        } else {
            Unref->getVarRef().setDecl(It->getValue());
            Builder.AddExternalGlobalVar(Unref->getNode(), It->getValue());
        }
    }

    return Success;
}

/**
 * Resolve Function into Node
 * @param Function
 * @return
 */
bool SemaResolver::ResolveFunctionCalls(ASTNode *Node) {
    bool Success = true;

    // Skip Function Reference to libc
    bool IsBaseLib = Node->getNameSpace()->getName().find("fly.base.") == 0;

    // Resolve Unreferenced Function Calls (node internal)
    for (auto *UnrefFunctionCall : Node->UnrefFunctionCalls) {
        FLY_DEBUG_MESSAGE("Sema", "ResolveFunctionCalls",
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
                    if (S.isUsable(FunctionCall->getDecl(), UnrefFunctionCall->getCall())) {
                        UnrefFunctionCall->getCall()->setDecl(FunctionCall->getDecl());
                    } else {
                        Success = false;
                    }
                }
                if (!Success) { // Not Found
                    Diag(UnrefFunctionCall->getCall()->getLocation(), diag::err_unref_call)
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
bool SemaResolver::ResolveFunctionCalls(ASTNameSpace *NameSpace) {
    bool Success = true;

    // Resolve Unreferenced Function Calls (at namespace level)
    for (auto *UnrefFunctionCall : NameSpace->UnrefFunctionCalls) {
        FLY_DEBUG_MESSAGE("Sema", "ResolveFunctionCalls",
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
            Diag(UnrefFunctionCall->getCall()->getLocation(), diag::err_unref_call)
                    << UnrefFunctionCall->getCall()->getName();
            Success = false; // collects other errors
        } else {
            for (auto &FunctionCall : It->getValue()) {
                if (S.isUsable(FunctionCall->getDecl(), UnrefFunctionCall->getCall())) {
                    UnrefFunctionCall->getCall()->setDecl(FunctionCall->getDecl());
                    Builder.AddExternalFunction(UnrefFunctionCall->getNode(), FunctionCall->getDecl()); // Call resolved with external function
                } else {
                    Success = false;
                }
            }
            if (!Success) { // Not Found
                Diag(UnrefFunctionCall->getCall()->getLocation(), diag::err_unref_call)
                        << UnrefFunctionCall->getCall()->getName();
            }
        }
    }
    return Success;
}

bool SemaResolver::ResolveFunctions(ASTNode *Node) {
    bool Success = true;
    for (auto &Function : Node->Functions) {
        Success &= ResolveBlock(Function->Body);
    }
    return Success;
}

bool SemaResolver::ResolveBlock(ASTBlock *Block) {
    bool Success = true;
    for (auto &Stmt : Block->getContent()) {
        switch (Stmt->getKind()) {

            case STMT_BLOCK:
                switch (((ASTBlock *) Stmt)->getBlockKind()) {

                    case BLOCK_STMT:
                        Success &= ResolveBlock((ASTBlock *) Stmt);
                        break;
                    case BLOCK_STMT_IF:

                        break;
                    case BLOCK_STMT_ELSIF:
                    case BLOCK_STMT_ELSE:
                        break;
                    case BLOCK_STMT_SWITCH:
                        break;
                    case BLOCK_STMT_CASE:
                        break;
                    case BLOCK_STMT_DEFAULT:
                        break;
                    case BLOCK_STMT_WHILE:
                        break;
                    case BLOCK_STMT_FOR:
                        break;
                }
                break;
            case STMT_EXPR:
                Success &= ResolveExpr(Block, ((ASTExprStmt *) Stmt)->getExpr());
                break;
            case STMT_VAR:
                Success &= ResolveExpr(Block, ((ASTLocalVar *) Stmt)->getExpr()) &
                           S.CheckDuplicatedLocalVars(Block, ((ASTLocalVar *) Stmt));
                break;
            case STMT_VAR_ASSIGN: {
                ASTVarAssign *VarAssign = ((ASTVarAssign *) Stmt);
                Success &= (!VarAssign->getVarRef()->getDecl() || ResolveVarRef(Block, VarAssign->getVarRef())) &&
                           ResolveExpr(Block, VarAssign->getExpr());
                break;
            }
            case STMT_FUNCTION_CALL:
                break;
            case STMT_RETURN:
                Success &= ResolveExpr(Block, ((ASTReturn *) Stmt)->getExpr());
                break;
            case STMT_BREAK:
            case STMT_CONTINUE:
                break;
        }
    }
    return false;
}

bool SemaResolver::ResolveClass(ASTNode *Node) {
    return true;
}

ASTType *SemaResolver::ResolveExprType(ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveExprType","Expr=" << Expr->str());
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
ASTLocalVar *SemaResolver::FindVarDecl(ASTBlock *Block, ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("Sema", "FindVarDecl", "VarRef=" << VarRef->str());
    const auto &It = Block->getLocalVars().find(VarRef->getName());
    if (It != Block->getLocalVars().end()) { // Search into this Block
        FLY_DEBUG_MESSAGE("Sema", "FindVarDecl", "Found=" << It->getValue()->str());
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
bool SemaResolver::ResolveVarRef(ASTBlock *Block, ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveVarRef", "VarRef=" << VarRef->str());
    // Search into parameters
    for (auto &Param : Block->getTop()->getParams()->getList()) {
        if (VarRef->getName() == Param->getName()) {
            // Resolve with Param
            VarRef->setDecl(Param);
            break;
        }
    }

    // If VarRef is not resolved with parameters, search into Block declarations
    if (!VarRef->getDecl()) {
        // Search recursively into current Block or in one of Parents
        ASTLocalVar *LocalVar = FindVarDecl(Block, VarRef);
        // Check if var declaration var is resolved
        if (LocalVar) {
            VarRef->setDecl(LocalVar); // Resolved
        } else {
            Builder.AddUnrefGlobalVar(Block->getTop()->getNode(), VarRef); // Resolve Later by searching into Node GlobalVars
        }
    }

    if (VarRef->getDecl()) {
        // The Var is now well-defined: you can remove it from UndefVars
        return Builder.RemoveUndefVar(Block, VarRef);
    }
    return false;
}

/**
 * Resolve Expr contents
 * @param Expr
 * @return true if no error occurs, otherwise false
 */
bool SemaResolver::ResolveExpr(ASTBlock *Block, const ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveExpr", "Expr=" << Expr->str());
    switch (Expr->getExprKind()) {
        case EXPR_REF_VAR: {
            ASTVarRef *Var = ((ASTVarRefExpr *)Expr)->getVarRef();
            return S.CheckUndefVar(Block, Var) && (Var->getDecl() || ResolveVarRef(Block, Var));
        }
        case EXPR_REF_FUNC: {
            ASTFunctionCall *Call = ((ASTFuncCallExpr *)Expr)->getCall();
            return Call->getDecl() || Builder.AddUnrefCall(Block->getTop()->getNode(), Call);
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

/**
 * Write Diagnostics
 * @param Loc
 * @param DiagID
 * @return
 */
DiagnosticBuilder SemaResolver::Diag(SourceLocation Loc, unsigned DiagID) const {
    return S.Diag(Loc, DiagID);
}

/**
 * Write Diagnostics
 * @param Loc
 * @param DiagID
 * @return
 */
DiagnosticBuilder SemaResolver::Diag(unsigned DiagID) const {
    return S.Diag(DiagID);
}
