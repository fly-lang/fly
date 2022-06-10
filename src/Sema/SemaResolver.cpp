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
#include "llvm/ADT/StringMap.h"

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
        Success &= //ResolveGlobalVars(Node) & // resolve Node UnrefGlobalVars
                //ResolveFunctions(Node) & // resolve Node UnrefFunctionCalls
                ResolveBodyFunctions(Node) & // resolve ASTBlock of Body Functions
                ResolveClass(Node);          // resolve Class attributes and methods
    }

    // Resolve NameSpaces
    for (auto &NSEntry : S.Context->NameSpaces) {
        auto &NameSpace = NSEntry.getValue();
        Success &= ResolveImports(NameSpace);  // resolve Imports
                //ResolveGlobalVars(NameSpace) &   // resolve NameSpace UnrefGlobalVars
                //ResolveFunctions(NameSpace); // resolve NameSpace UnrefFunctionCalls
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

            if (NameSpaceFound) {
                FLY_DEBUG_MESSAGE("Sema", "ResolveImports",
                                  "Import=" << Import->getName() <<
                                            ", NameSpace=" << NameSpaceFound->getName());
                Import->setNameSpace(NameSpaceFound);

            } else {
                // Error: NameSpace not found
                Success = false;
                Diag(Import->NameLocation, diag::err_namespace_notfound) << Import->getName();
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
//bool SemaResolver::ResolveGlobalVars(ASTNode *Node) {
//
//    // Resolve Unreferenced Global Var (node internal)
//    for (auto &Unref : Node->UnrefGlobalVars) {
//        FLY_DEBUG_MESSAGE("Sema", "ResolveGlobalVars",
//                          "Node=" << Node->str() <<
//                          ", VarRef=" << Unref->getVarRef().str());
//        const auto &It = Node->GlobalVars.find(Unref->getVarRef().getName());
//        if (It == Node->GlobalVars.end()) {
//            // of the current NameSpace
//            Node->NameSpace->UnrefGlobalVars.push_back(Unref);
//        } else {
//            ASTVarRef &VarRef = Unref->getVarRef();
//            ASTGlobalVar *Var = It->getValue();
//            VarRef.Def = Var;
//        }
//    }
//
//    return true;
//}

/**
 * Resolve GlobalVar into NameSpace
 * @param NameSpace
 * @return
 */
//bool SemaResolver::ResolveGlobalVars(ASTNameSpace *NameSpace) {
//    bool Success = true;
//
//    // Resolve Unreferenced Global Var (node internal)
//    for (auto &Unref : NameSpace->UnrefGlobalVars) {
//        FLY_DEBUG_MESSAGE("Sema", "ResolveGlobalVars",
//                          "NameSpace=" << NameSpace->str() <<
//                          ", VarRef=" << Unref->getVarRef().str());
//        const auto &It = NameSpace->GlobalVars.find(Unref->getVarRef().getName());
//        if (It == NameSpace->GlobalVars.end()) { // NameSpace not contains GlobalVar throw error
//            Diag(Unref->getVarRef().getLocation(), diag::err_gvar_notfound)
//                << Unref->getVarRef().getName();
//            Success = false;
//        } else {
//            Unref->getVarRef().Def = It->getValue();
//            Builder.AddExternalGlobalVar(Unref->getNode(), It->getValue());
//        }
//    }
//
//    return Success;
//}

/**
 * Resolve Function into Node
 * @param Function
 * @return
 */
//bool SemaResolver::ResolveFunctions(ASTNode *Node) {
//    bool Success = true;
//
//    // Skip Function Reference to libc
//    bool IsBaseLib = Node->getNameSpace()->getName().find("fly.base.") == 0;
//
//    // Resolve Unreferenced Function Calls (node internal)
//    for (auto *UnrefFunctionCall : Node->UnrefFunctionCalls) {
//        FLY_DEBUG_MESSAGE("Sema", "ResolveFunctions",
//                          "Node=" << Node->str() <<
//                          ", UnrefFunctionCall=" << UnrefFunctionCall->getCall()->str());
//        if (IsBaseLib && UnrefFunctionCall->getCall()->getName().find("c__") == 0) {
//            continue; // TODO UnrefFunctionCalls can be checked with all possible libc functions
//        } else {
//
//            // Search a callable Function by Name
//            const auto &StrMapIt = Node->Functions.find(UnrefFunctionCall->getCall()->getName());
//            if (StrMapIt == Node->Functions.end()) { // Node not contains Function with that name
//                // Search into NameSpace on next step
//                Node->NameSpace->UnrefFunctionCalls.push_back(UnrefFunctionCall);
//            } else {
//
//                std::map<uint64_t, llvm::SmallVector<ASTFunction *, 4>> &IntMap = StrMapIt->getValue();
//                const auto &IntMapIt = IntMap.find(UnrefFunctionCall->getCall()->getArgs().size());
//                if (IntMapIt == IntMap.end()) { // Node not contains Function with this size of args
//                    // Search into NameSpace on next step
//                    Node->NameSpace->UnrefFunctionCalls.push_back(UnrefFunctionCall);
//                } else {
//                    // Set Candidate Definitions for Call
//                    UnrefFunctionCall->getCall()->CandidateDefs = IntMapIt->second;
//                }
//            }
//        }
//    }
//    return Success;
//}

/**
 * Resolve Function into NameSpace
 * @param Function
 * @return
 */
//bool SemaResolver::ResolveFunctions(ASTNameSpace *NameSpace) {
//    bool Success = true;
//
//    // Resolve Unreferenced Function Calls (at namespace level)
//    for (auto *UnrefFunctionCall : NameSpace->UnrefFunctionCalls) {
//        FLY_DEBUG_MESSAGE("Sema", "ResolveFunctions",
//                          "NameSpace=" << NameSpace->Name <<
//                          ", UnrefFunctionCall=" << UnrefFunctionCall->getCall()->str());
//
//        // Auto resolve in Lib
//        if (NameSpace->isExternalLib()) {
//            // TODO
//            // need to to read the prototype for external function declaration
//            // UnrefFunctionCall->getCall()->setDecl(Func);
//            // UnrefFunctionCall->getNode()->AddExternalFunction(Func);
//        }
//
//        // Search a callable Function by Name
//        const auto &StrMapIt = NameSpace->Functions.find(UnrefFunctionCall->getCall()->getName());
//        if (StrMapIt == NameSpace->Functions.end()) { // NameSpace not contains Function with that name
//            Diag(UnrefFunctionCall->getCall()->getLocation(), diag::err_unref_call)
//                    << UnrefFunctionCall->getCall()->getName();
//            Success = false; // error
//        } else {
//
//            std::map<uint64_t, llvm::SmallVector<ASTFunction *, 4>> &IntMap = StrMapIt->getValue();
//            const auto &IntMapIt = IntMap.find(UnrefFunctionCall->getCall()->getArgs().size());
//            if (IntMapIt == IntMap.end()) { // Node not contains Function with this size of args
//                Diag(UnrefFunctionCall->getCall()->getLocation(), diag::err_unref_call)
//                        << UnrefFunctionCall->getCall()->getName();
//                Success = false; // error
//            } else {
//                // Set Candidate Definitions for Call
//                UnrefFunctionCall->getCall()->CandidateDefs = IntMapIt->second;
//
////                UnrefFunctionCall->getCall()->Def = FunctionCall->getDef();
////                // Call resolved with external function
////                Builder.AddExternalFunction(UnrefFunctionCall->getNode(), FunctionCall->getDef());
//            }
//        }
//    }
//
//    return Success;
//}

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
            case STMT_VAR_DEFINE: {
                ASTLocalVar *LocalVar = ((ASTLocalVar *) Stmt);
                if (LocalVar->getExpr())
                    Success &= ResolveExpr(LocalVar->getExpr());
                break;
            }
            case STMT_VAR_ASSIGN: {
                ASTVarAssign *VarAssign = ((ASTVarAssign *) Stmt);

                // Error: Expr cannot be null
                if (!VarAssign->getExpr()) {
                    Diag(VarAssign->getLocation(), diag::err_var_assign_empty) << VarAssign->getVarRef()->getName();
                    return false;
                }

                Success &= (VarAssign->getVarRef()->getDef() || ResolveVarRef(Block, VarAssign->getVarRef())) &&
                           ResolveExpr(VarAssign->getExpr());
                break;
            }
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
    if (!Call->Def) {
        ASTBlock *Block = getBlock(Call->Stmt);

        const auto &Node = Block->getTop()->getNode();
        ASTImport *Import;
        llvm::StringMapIterator<std::map<uint64_t, llvm::SmallVector<ASTFunction *, 4>>> StrMapIt;
        llvm::StringMapIterator<std::map<uint64_t, llvm::SmallVector<ASTFunction *, 4>>> NotExists;
        if (Call->getNameSpace().empty()) {
            // Find in current Node
            StrMapIt = Node->Functions.find(Call->getName());
            NotExists = Node->Functions.end();
        } else if (Call->getNameSpace() == Node->NameSpace->getName()) {
            // Find in current NameSpace
            StrMapIt = Node->NameSpace->Functions.find(Call->getName());
            NotExists = Node->NameSpace->Functions.end();
        } else if ((Import = Node->FindImport(Call->getNameSpace()))) {
            // Find in current Import
            StrMapIt = Import->NameSpace->Functions.find(Call->getName());
            NotExists = Import->NameSpace->Functions.end();
        }

        if (StrMapIt != NotExists) {
            std::map<uint64_t, llvm::SmallVector<ASTFunction *, 4>> &IntMap = StrMapIt->getValue();
            const auto &IntMapIt = IntMap.find(Call->getArgs().size());
            if (IntMapIt != IntMap.end()) { // Node contains Function with this size of args
                bool Success = true;
                for (ASTFunction *Function : IntMapIt->second) {
                    Success = true;
                    if (Function->getParams()->getSize() == Call->getArgs().size()) {
                        for (unsigned long i = 0; i < Function->getParams()->getSize(); i++) {
                            // Resolve Arg Expr on first
                            ASTArg *Arg = Call->getArgs().at(i);
                            ASTParam *Param = Function->getParams()->at(i);
                            Success = ResolveArg(Arg, Param);
                            if (!Success) break;
                        }
                        // Set Function definition for Call
                        if (Success)
                            Call->Def = Function;
                    }
                }
                return Success;
            }
        }

        Diag(Call->getLocation(), diag::err_unref_call);
        return false;
    }
    
    return true;
}

bool SemaResolver::ResolveArg(ASTArg *Arg, ASTParam *Param) {
    Arg->Def = Param;
    if (ResolveExpr(Arg->Expr)) {
        return S.isTypeDerivate(Param->Type, Arg->Expr->Type);
    }

    return false;
}

/**
 * Search a VarRef into declared Block's vars
 * If found set LocalVar
 * @param Block
 * @param LocalVar
 * @param VarRef
 * @return the found LocalVar
 */
ASTLocalVar *SemaResolver::FindVarDef(ASTBlock *Block, ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("Sema", "FindVarDef", "VarRef=" << VarRef->str());
    const auto &It = Block->getLocalVars().find(VarRef->getName());
    if (It != Block->getLocalVars().end()) { // Search into this Block
        FLY_DEBUG_MESSAGE("Sema", "FindVarDef", "Found=" << It->getValue()->str());
        return It->getValue();
    } else if (Block->getParent()) { // Traverse Parent Block to find the right VarDeclStmt
        return FindVarDef(Block->getParent(), VarRef);
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
        ASTLocalVar *LocalVar = FindVarDef(Block, VarRef);
        // Check if var declaration var is resolved
        if (LocalVar) {
            VarRef->Def = LocalVar; // Resolved
        } else {
            const auto &Node = Block->getTop()->getNode();
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
                Diag(VarRef->getLocation(), diag::err_unref_var);
                return false;
            }

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

    Diag(VarRef->getLocation(), diag::err_undef_var);
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
            ASTBlock *Block = getBlock(Expr->getStmt());
            ASTVarRef *VarRef = ((ASTVarRefExpr *)Expr)->getVarRef();
            bool Success = S.CheckUndef(Block, VarRef) &&
                   (VarRef->getDef() || ResolveVarRef(Block, VarRef));
            if (Success)
                Expr->Type = VarRef->Def->Type;
            return Success;
        }
        case EXPR_REF_FUNC: {
            ASTFunctionCall *Call = ((ASTFunctionCallExpr *)Expr)->getCall();
            bool Success = Call->getDef() || ResolveFunctionCall(Call);
            if (Success)
                Expr->Type = Call->Def->Type;
            return Success;
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
                case STMT_VAR_DEFINE: // int a = 1
                    Expr->Type = ((ASTLocalVar *)Expr->Stmt)->getType();
                    break;
                case STMT_VAR_ASSIGN: // a = 1
                    Expr->Type = ((ASTVarAssign *)Expr->Stmt)->getVarRef()->getDef()->getType();
                    break;
                case STMT_ARG: // func(a, 1)
                    Expr->Type = ((ASTArg *)Expr->Stmt)->getDef()->getType();
                    break;
                case STMT_BLOCK:
                case STMT_EXPR:
                case STMT_BREAK:
                case STMT_CONTINUE:
                    assert("Cannot contains an ASTExpr");
                case STMT_RETURN: // return 1
                    ASTBlock *Block = getBlock(Expr->Stmt);
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

ASTBlock *SemaResolver::getBlock(ASTStmt *Stmt) {
    switch (Stmt->getKind()) {

        case STMT_BLOCK:
            return (ASTBlock *) Stmt;
        case STMT_ARG:
            return (ASTBlock *) ((ASTArg *)Stmt)->Call->Stmt->Parent;
        case STMT_RETURN:
        case STMT_EXPR:
        case STMT_VAR_DEFINE:
        case STMT_VAR_ASSIGN:
            return (ASTBlock *) Stmt->getParent();
        case STMT_BREAK:
        case STMT_CONTINUE:
            assert("Unexpected parent for ASTExpr");
    }
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
