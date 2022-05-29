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
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenLocalVar.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"

using namespace fly;

SemaResolver::SemaResolver(Sema &S, SemaBuilder &Builder) : S(S), Builder(Builder) {

}

/**
 * Take all unreferenced Global Variables from Functions and try to resolve them
 * into all NameSpaces
 * @return
 */
bool SemaResolver::Resolve() {
    bool Success = true;

    // Resolve Nodes
    for (auto &NEntry : S.Context->getNodes()) {
        auto &Node = NEntry.getValue();
        Success &= ResolveGlobalVars(Node) & // resolve Node UnrefGlobalVars
                ResolveFunctions(Node) & // resolve Node UnrefFunctionCalls
                ResolveBodyFunctions(Node) & // resolve ASTBlock of Body Functions
                ResolveClass(Node);          // resolve Class attributes and methods
    }

    // Resolve NameSpaces
    for (auto &NSEntry : S.Context->NameSpaces) {
        auto &NameSpace = NSEntry.getValue();
        Success &= ResolveImports(NameSpace) &   // resolve Imports
                ResolveGlobalVars(NameSpace) &   // resolve NameSpace UnrefGlobalVars
                ResolveFunctions(NameSpace); // resolve NameSpace UnrefFunctionCalls
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
            VarRef.Def = Var;
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
        if (It == NameSpace->GlobalVars.end()) { // NameSpace not contains GlobalVar throw error
            Diag(Unref->getVarRef().getLocation(), diag::err_gvar_notfound)
                << Unref->getVarRef().getName();
            Success = false;
        } else {
            Unref->getVarRef().Def = It->getValue();
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
bool SemaResolver::ResolveFunctions(ASTNode *Node) {
    bool Success = true;

    // Skip Function Reference to libc
    bool IsBaseLib = Node->getNameSpace()->getName().find("fly.base.") == 0;

    // Resolve Unreferenced Function Calls (node internal)
    for (auto *UnrefFunctionCall : Node->UnrefFunctionCalls) {
        FLY_DEBUG_MESSAGE("Sema", "ResolveFunctions",
                          "Node=" << Node->str() <<
                          ", UnrefFunctionCall=" << UnrefFunctionCall->getCall()->str());
        if (IsBaseLib && UnrefFunctionCall->getCall()->getName().find("c__") == 0) {
            continue; // TODO UnrefFunctionCalls can be checked with all possible libc functions
        } else {

            // Search a callable Function by Name
            const auto &StrMapIt = Node->Functions.find(UnrefFunctionCall->getCall()->getName());
            if (StrMapIt == Node->Functions.end()) { // Node not contains Function with that name
                // Search into NameSpace on next step
                Node->NameSpace->UnrefFunctionCalls.push_back(UnrefFunctionCall);
            } else {

                std::map<uint64_t, llvm::SmallVector<ASTFunction *, 4>> &IntMap = StrMapIt->getValue();
                const auto &IntMapIt = IntMap.find(UnrefFunctionCall->getCall()->getArgs().size());
                if (IntMapIt == IntMap.end()) { // Node not contains Function with this size of args
                    // Search into NameSpace on next step
                    Node->NameSpace->UnrefFunctionCalls.push_back(UnrefFunctionCall);
                } else {
                    // Set Candidate Definitions for Call
                    UnrefFunctionCall->getCall()->CandidateDefs = IntMapIt->second;
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
bool SemaResolver::ResolveFunctions(ASTNameSpace *NameSpace) {
    bool Success = true;

    // Resolve Unreferenced Function Calls (at namespace level)
    for (auto *UnrefFunctionCall : NameSpace->UnrefFunctionCalls) {
        FLY_DEBUG_MESSAGE("Sema", "ResolveFunctions",
                          "NameSpace=" << NameSpace->Name <<
                          ", UnrefFunctionCall=" << UnrefFunctionCall->getCall()->str());

        // Auto resolve in Lib
        if (NameSpace->isExternalLib()) {
            // TODO
            // need to to read the prototype for external function declaration
            // UnrefFunctionCall->getCall()->setDecl(Func);
            // UnrefFunctionCall->getNode()->AddExternalFunction(Func);
        }

        // Search a callable Function by Name
        const auto &StrMapIt = NameSpace->Functions.find(UnrefFunctionCall->getCall()->getName());
        if (StrMapIt == NameSpace->Functions.end()) { // NameSpace not contains Function with that name
            Diag(UnrefFunctionCall->getCall()->getLocation(), diag::err_unref_call)
                    << UnrefFunctionCall->getCall()->getName();
            Success = false; // error
        } else {

            std::map<uint64_t, llvm::SmallVector<ASTFunction *, 4>> &IntMap = StrMapIt->getValue();
            const auto &IntMapIt = IntMap.find(UnrefFunctionCall->getCall()->getArgs().size());
            if (IntMapIt == IntMap.end()) { // Node not contains Function with this size of args
                Diag(UnrefFunctionCall->getCall()->getLocation(), diag::err_unref_call)
                        << UnrefFunctionCall->getCall()->getName();
                Success = false; // error
            } else {
                // Set Candidate Definitions for Call
                UnrefFunctionCall->getCall()->CandidateDefs = IntMapIt->second;

//                UnrefFunctionCall->getCall()->Def = FunctionCall->getDef();
//                // Call resolved with external function
//                Builder.AddExternalFunction(UnrefFunctionCall->getNode(), FunctionCall->getDef());
            }
        }
    }

    return Success;
}

bool SemaResolver::ResolveClass(ASTNode *Node) {
    return true;
}

bool SemaResolver::ResolveClass(ASTNameSpace *NameSpace) {
    return true;
}


bool SemaResolver::ResolveBodyFunctions(ASTNode *Node) {
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
    for (auto &Stmt : Block->getContent()) {
        switch (Stmt->getKind()) {

            case STMT_BLOCK:
                Success &= ResolveBlock((ASTBlock *) Stmt);
                break;
            case STMT_EXPR:
                Success &= ResolveExpr(((ASTExprStmt *) Stmt)->getExpr());
                break;
            case STMT_VAR: {
                ASTLocalVar *LocalVar = ((ASTLocalVar *) Stmt);
                Success &= ResolveExpr(LocalVar->getExpr());
                break;
            }
            case STMT_VAR_ASSIGN: {
                ASTVarAssign *VarAssign = ((ASTVarAssign *) Stmt);
                Success &= (!VarAssign->getVarRef()->getDef() || ResolveVarRef(Block, VarAssign->getVarRef())) &&
                           ResolveExpr(VarAssign->getExpr());
                break;
            }
            case STMT_FUNCTION_CALL:
                // Chose Def from the Candidate Def of the Call
                Success &= ResolveFunctionCall(((ASTFunctionCall *) Stmt));
                break;
            case STMT_RETURN:
                Success &= ResolveExpr(((ASTReturn *) Stmt)->getExpr());
                break;
            case STMT_BREAK:
            case STMT_CONTINUE:
                break;
            case STMT_ARG:
                assert("Block Stmt cannot have an ASTArg");
        }
    }
    return Success;
}

bool SemaResolver::ResolveFunctionCall(ASTFunctionCall *Call) {
    if (Call->CandidateDefs.empty()) {
        // TODO
        // Error: no candidate Function
        return false;
    }
    bool Success = true;
    for (ASTFunction *CF : Call->CandidateDefs) {
        Success = true;
        if (CF->getParams()->getSize() == Call->getArgs().size()) {
            for (unsigned long i = 0; i < CF->getParams()->getSize(); i++) {
                // Resolve Arg Expr on first
                ASTArg *Arg = Call->getArgs().at(i);
                ResolveExpr(Arg->Expr);
                Success = S.isTypeDerivate(CF->getParams()->at(i)->Type,Arg->Expr->Type);
                if (!Success) break;
            }
        }
    }
    return Success;
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
            VarRef->Def = Param;
            break;
        }
    }

    // If VarRef is not resolved with parameters, search into Block declarations
    if (!VarRef->getDef()) {
        // Search recursively into current Block or in one of Parents
        ASTLocalVar *LocalVar = FindVarDecl(Block, VarRef);
        // Check if var declaration var is resolved
        if (LocalVar) {
            VarRef->Def = LocalVar; // Resolved
        } else {
            Builder.AddUnrefGlobalVar(Block->getTop()->getNode(), VarRef); // Resolve Later by searching into Node GlobalVars
        }
    }

    if (VarRef->getDef()) {
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
bool SemaResolver::ResolveExpr(ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveExpr", "Expr=" << Expr->str());
    switch (Expr->getExprKind()) {
        case EXPR_REF_VAR: {
            ASTBlock *Block = (ASTBlock *) Expr->Stmt->getParent();
            ASTVarRef *Var = ((ASTVarRefExpr *)Expr)->getVarRef();
            return S.CheckUndefVar(Block, Var) &&
                (Var->getDef() || ResolveVarRef(Block, Var));
        }
        case EXPR_REF_FUNC: {
            ASTBlock *Block = (ASTBlock *) Expr->Stmt->getParent();
            ASTFunctionCall *Call = ((ASTFuncCallExpr *)Expr)->getCall();
            return Call->getDef() || Builder.AddUnrefFunctionCall(Block->getTop()->getNode(), Call);
        }
        case EXPR_GROUP: {
            ASTGroupExpr *GroupExpr = (ASTGroupExpr *) Expr;
            switch (GroupExpr->getGroupKind()) {

                case GROUP_UNARY:
                    return ResolveExpr((ASTExpr *)((ASTUnaryGroupExpr *)GroupExpr)->getFirst());
                case GROUP_BINARY:
                    return ResolveExpr(((ASTBinaryGroupExpr *)GroupExpr)->First) &&
                           ResolveExpr(((ASTBinaryGroupExpr *)GroupExpr)->Second);
                case GROUP_TERNARY:
                    return ResolveExpr(((ASTTernaryGroupExpr *)GroupExpr)->First) &&
                           ResolveExpr(((ASTTernaryGroupExpr *)GroupExpr)->Second) &&
                           ResolveExpr(((ASTTernaryGroupExpr *)GroupExpr)->Third);
            }
        }
        case EXPR_VALUE: // Resolve ASTExprValue Type
            switch (Expr->Stmt->getKind()) {
                case STMT_VAR: // a = 1
                    Expr->Type = ((ASTVarRef *)Expr->Stmt)->getDef()->getType();
                    break;
                case STMT_VAR_ASSIGN: // int a = 1
                    Expr->Type = ((ASTLocalVar *)Expr->Stmt)->getType();
                    break;
                case STMT_FUNCTION_CALL: // func(a, 1)
                    assert("Call cannot contain directly an ASTExpr but only by ASTArg");
                case STMT_ARG:
                    Expr->Type = ((ASTArg *)Expr->Stmt)->getDef()->getType();
                    break;
                case STMT_BLOCK:
                case STMT_EXPR:
                case STMT_BREAK:
                case STMT_CONTINUE:
                    assert("Cannot contains an ASTExpr");
                case STMT_RETURN: // return 1
                    ASTBlock *Block = (ASTBlock *) Expr->Stmt->getParent();
                    // take the ASTType from function() return type FIXME
                    Expr->Type = Block->Top->getType();
                    break;
            }
            return S.VerifyValueType((ASTValueExpr *)Expr, Expr->Type);
        case EXPR_EMPTY:
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
