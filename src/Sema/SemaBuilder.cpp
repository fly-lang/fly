//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/Sema.cpp - GlobalVar Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaBuilder.h"
#include "Sema/SemaResolver.h"
#include "AST/ASTContext.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTNode.h"
#include "AST/ASTImport.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTFunctionCall.h"
#include "AST/ASTClass.h"
#include "AST/ASTParams.h"
#include "AST/ASTBlock.h"
#include "AST/ASTIfBlock.h"
#include "AST/ASTWhileBlock.h"
#include "AST/ASTForBlock.h"
#include "AST/ASTSwitchBlock.h"
#include "AST/ASTVar.h"
#include "AST/ASTVarAssign.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"

using namespace fly;

SemaBuilder::SemaBuilder(Sema &S) : S(S) {

}

bool SemaBuilder::Build() {
    return S.Resolver->Resolve();
}

/**
 * Write Diagnostics
 * @param Loc
 * @param DiagID
 * @return
 */
DiagnosticBuilder SemaBuilder::Diag(SourceLocation Loc, unsigned DiagID) const {
    return S.Diag(Loc, DiagID);
}

/**
 * Write Diagnostics
 * @param Loc
 * @param DiagID
 * @return
 */
DiagnosticBuilder SemaBuilder::Diag(unsigned DiagID) const {
    return S.Diag(DiagID);
}

bool SemaBuilder::AddImport(ASTNode *Node, ASTImport * Import) {
    FLY_DEBUG_MESSAGE("ASTNode", "AddImport", "Import=" << Import->str());

    if (S.CheckImport(Node, Import)) {
        std::string Name = Import->getAlias().empty() ? Import->getName() : Import->getAlias();

        // Check if this Node already own this Import
        if (Node->Imports.lookup(Name)) {
            Diag(Import->getNameLocation(), diag::err_conflict_import) << Name;
            return false;
        }

        // Add Import to Node
        auto Pair = std::make_pair(Name, Import);
        return Node->Imports.insert(Pair).second;

    }


}

bool SemaBuilder::AddGlobalVar(ASTNode *Node, ASTGlobalVar *GlobalVar) {
    assert(GlobalVar->Visibility && "Function Visibility is unset");
    FLY_DEBUG_MESSAGE("ASTNode", "AddGlobalVar", "Var=" << GlobalVar->str());

    // Lookup into namespace for public var
    if(GlobalVar->Visibility == VisibilityKind::V_PUBLIC || GlobalVar->Visibility == VisibilityKind::V_DEFAULT) {
        ASTGlobalVar *LookupVar = Node->NameSpace->getGlobalVars().lookup(GlobalVar->getName());
        if (LookupVar) { // This NameSpace already contains this GlobalVar
            Diag(LookupVar->getLocation(), diag::err_duplicate_gvar) << LookupVar->getName();
            return false;
        }

        // Add into NameSpace for global resolution
        // Add into Node for local resolution
        auto Pair = std::make_pair(GlobalVar->getName(), GlobalVar);
        return Node->GlobalVars.insert(Pair).second && Node->NameSpace->GlobalVars.insert(Pair).second;
    }

    // Lookup into node for private var
    if(GlobalVar->Visibility == VisibilityKind::V_PRIVATE) {
        ASTGlobalVar *LookupVar = Node->GlobalVars.lookup(GlobalVar->getName());
        if (LookupVar) { // This Node already contains this Function
            Diag(LookupVar->getLocation(), diag::err_duplicate_gvar) << LookupVar->getName();
            return false;
        }

        // Add into Node for local resolution
        auto Pair = std::make_pair(GlobalVar->getName(), GlobalVar);
        return Node->GlobalVars.insert(Pair).second;
    }

    assert(0 && "Error when adding GlobalVar");
}

bool SemaBuilder::AddFunction(ASTNode *Node, ASTFunction *Function) {
    assert(Function->Visibility && "Function Visibility is unset");
    FLY_DEBUG_MESSAGE("ASTNode", "AddFunction", "Func=" << Function->str());

    // Lookup into namespace for public var
    if(Function->Visibility == VisibilityKind::V_PUBLIC || Function->Visibility == VisibilityKind::V_DEFAULT) {
        const auto &FuncIt = Node->NameSpace->getFunctions().find(Function);
        if (FuncIt != Node->NameSpace->getFunctions().end()) { // This NameSpace already contains this Function
            Diag(Function->getLocation(), diag::err_duplicate_func) << Function->getName();
            return false;
        }

        // Add into NameSpace for global resolution
        // Add into Node for local resolution
        ASTFunctionCall *Call = CreateCall(Function);
        if (Node->NameSpace->Functions.insert(Function).second &&
                AddFunctionCall(Node->NameSpace, Call) &&
                Node->Functions.insert(Function).second &&
                AddFunctionCall(Node, Call)) {
            return true;
        }

        Diag(Function->getLocation(), diag::err_add_func) << Function->getName();
        return false;
    }

    // Lookup into node for private var
    if (Function->Visibility == VisibilityKind::V_PRIVATE) {
        const auto &FuncIt = Node->Functions.find(Function);
        if (FuncIt != Node->Functions.end()) { // This Node already contains this Function
            Diag(Function->getLocation(), diag::err_duplicate_func) << Function->getName();
            return false;
        }

        // Add into Node for local resolution (Private)
        if (Node->Functions.insert(Function).second && AddFunctionCall(Node, CreateCall(Function))) {
            return true;
        }

        Diag(Function->getLocation(), diag::err_add_func) << Function->getName();
        return false;
    }

    assert(0 && "Error when adding Function");
}

bool SemaBuilder::AddClass(ASTNode *Node, ASTClass *Class) {
    FLY_DEBUG_MESSAGE("ASTNode", "AddFunction", "Class" << Class->str());

    // Lookup into namespace
    // TODO Class scope differences
    ASTClass *LookupClass = Node->NameSpace->getClasses().lookup(Class->getName());
    if (LookupClass) { // This NameSpace already contains this Function
        Diag(LookupClass->Location, diag::err_duplicate_class)  << LookupClass->getName();
        return false;
    }
    Node->NameSpace->Classes.insert(std::make_pair(Class->getName(), Class));
}

bool SemaBuilder::AddUnrefCall(ASTNode *Node, ASTFunctionCall *Call) {
    FLY_DEBUG_MESSAGE("ASTNode", "AddUnrefCall", "Node.Name=" << Node->Name <<
                                                              ", Call=" << Call->str());
    ASTUnrefCall *Unref = new ASTUnrefCall(Node, Call);
    if (Call->getNameSpace().empty()) {
        Node->UnrefFunctionCalls.push_back(Unref); // Unref of Node or from a Namespace not specified
    } else if (Call->getNameSpace() == Node->NameSpace->getName()) { // call must be resolved into current namespace
        Node->NameSpace->UnrefFunctionCalls.push_back(Unref);
    } else {
        ASTImport *Import = Node->FindImport(Call->getNameSpace());
        if (Import == nullptr) {
            Diag(Call->getLocation(), diag::err_import_notfound) << Call->getNameSpace();
            return false;
        }
        Import->UnrefFunctionCalls.push_back(Unref);
    }
    return true;
}

bool SemaBuilder::AddUnrefGlobalVar(ASTNode *Node, ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("ASTNode", "AddUnrefGlobalVar", "Node.Name=" << Node->Name <<
                                                                   ", VarRef=" << VarRef->str());
    ASTUnrefGlobalVar *Unref = new ASTUnrefGlobalVar(Node, *VarRef);
    if (VarRef->getNameSpace().empty()) {
        Node->UnrefGlobalVars.push_back(Unref); // Unref of Node or from a Namespace not specified
    } else if (VarRef->getNameSpace() == Node->NameSpace->getName()) {
        Node->NameSpace->UnrefGlobalVars.push_back(Unref);
    } else {
        ASTImport *Import = Node->FindImport(VarRef->getNameSpace());
        if (!Import) {
            Diag(VarRef->getLocation(), diag::err_import_notfound)
                    << VarRef->getNameSpace();
            return false;
        }
        Import->UnrefGlobalVars.push_back(Unref);
    }
    return true;
}

bool SemaBuilder::AddComment(ASTTopDecl *Top, std::string C) {
    const char* t = " \t\n\r\f\v";
    C = C.substr(2, C.size()-4);
    C = C.erase(0, C.find_first_not_of(t));
    Top->Comment = C.erase(C.find_last_not_of(t) + 1);
    return false;
}

bool SemaBuilder::AddFunctionParam(ASTFunction *Function, ASTParam *Param) {
    Function->Params->List.push_back(Param); // TODO verify duplicates
    return false;
}

ASTParam *SemaBuilder::AddParam(ASTFunction *Function, const SourceLocation &Loc, ASTType *Type,
                                const std::string &Name, bool Constant) {
    ASTParam *Param = new ASTParam(Loc, Function->Body, Type, Name, Constant);
    Function->Params->List.push_back(Param);
    return Param;
}

void SemaBuilder::setVarArg(ASTFunction *Function, ASTParam* VarArg) {
    Function->Params->Ellipsis = VarArg;
}

bool SemaBuilder::AddFunctionCall(ASTNodeBase * Base, ASTFunctionCall *Call) {
    const auto &It = Base->FunctionCalls.find(Call->getName());
    FLY_DEBUG_MESSAGE("ASTNodeBase", "AddFunctionCall", "Call=" << Call->str());
    if (It == Base->FunctionCalls.end()) {
        std::vector<ASTFunctionCall *> TmpFunctionCalls;
        TmpFunctionCalls.push_back(Call);
        return Base->FunctionCalls.insert(std::make_pair(Call->getName(), TmpFunctionCalls)).second;
    }
    It->getValue().push_back(Call);
    return true;
}


ASTFunctionCall *SemaBuilder::CreateCall(ASTFunction *Function) {
    ASTFunctionCall *FCall = new ASTFunctionCall(SourceLocation(), Function->getNameSpace()->getName(), Function->getName());
    FCall->setDecl(Function);
    for (auto &Param : Function->getParams()->getList()) {
        FCall->addArg(new ASTCallArg(nullptr, Param->getType()));
    }
    return FCall;
}

bool SemaBuilder::setVarExpr(ASTParam *Param, ASTValueExpr *ValueExpr) {
    return false;
}

/**
 * Add ExprStmt to Content
 * @param ExprStmt
 * @return true if no error occurs, otherwise false
 */
bool SemaBuilder::AddExprStmt(ASTBlock *Block, ASTExprStmt *ExprStmt) {
    FLY_DEBUG_MESSAGE("ASTBlock", "AddExprStmt", "ExprStmt=" << ExprStmt->str());
    Block->Content.push_back(ExprStmt);
    return true;
}


/**
 * Add Local Var
 * @param LocalVar
 * @return true if no error occurs, otherwise false
 */
bool SemaBuilder::AddLocalVar(ASTBlock *Block, ASTLocalVar *LocalVar) {
    FLY_DEBUG_MESSAGE("ASTBlock", "AddLocalVar", "LocalVar=" << LocalVar->str());

    // Check if LocalVar have an Expression assigned
    if (!LocalVar->getExpr()) {  // No Expression: add to Undefined Vars, will be removed on AddLocalVarRef()
        Block->UndefVars.insert(std::pair<std::string, ASTLocalVar *>(LocalVar->getName(), LocalVar));
    }

    //Set CodeGen
    CodeGenLocalVar *CGLV = new CodeGenLocalVar(Block->Top->getNode()->getCodeGen(), LocalVar);
    LocalVar->setCodeGen(CGLV);

    // Add LocalVar
    Block->Content.push_back(LocalVar);
    Block->Top->addLocalVar(LocalVar); //Useful for Alloca into CodeGen
    return Block->LocalVars.insert(std::pair<std::string, ASTLocalVar *>(LocalVar->getName(), LocalVar)).second;
}

/**
 * Add Var Assign Stmt
 * @param VarAssign
 * @return true if no error occurs, otherwise false
 */
bool SemaBuilder::AddVarAssign(ASTBlock *Block, ASTVarAssign *VarAssign) {
    assert(VarAssign->getExpr() && "Expr unset into VarStmt");
    FLY_DEBUG_MESSAGE("ASTBlock", "AddVarAssign", "LocalVarRef=" << VarAssign->str());

    // Add Var to Block Content
    Block->Content.push_back(VarAssign);
    return true;
}

bool SemaBuilder::RemoveUndefVar(ASTBlock *Block, ASTVarRef *VarRef) {
    if (Block->UndefVars.lookup(VarRef->getName())) {
        return Block->UndefVars.erase(VarRef->getName());
    }
    return false;
}

/**
 * Add Call
 * @param Call
 * @return true if no error occurs, otherwise false
 */
bool SemaBuilder::AddCall(ASTBlock *Block, ASTFunctionCall *Call) {
    FLY_DEBUG_MESSAGE("ASTBlock", "AddBreak", "Call=" << Call->str());
    ASTExprStmt *ExprStmt = new ASTExprStmt(Call->getLocation(), Block);
    ExprStmt->setExpr(new ASTFuncCallExpr(Call));
    Block->Content.push_back(ExprStmt);
    return Call->getDecl() || Block->Top->getNode()->AddUnrefCall(Call);
}

/**
 * Add Return
 * @param Loc
 * @param Expr
 * @return true if no error occurs, otherwise false
 */
bool SemaBuilder::AddReturn(ASTBlock *Block, const SourceLocation &Loc, ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("ASTBlock", "AddReturn", "Expr=" << (Expr ? Expr->str() : ""));
    ASTReturn *Return = new ASTReturn(Loc, Block, Expr);
    Block->Content.push_back(Return);
    return true;
}

/**
 *
 * @param Loc
 * @return true if no error occurs, otherwise false
 */
bool SemaBuilder::AddBreak(ASTBlock *Block, const SourceLocation &Loc) {
    FLY_DEBUG("ASTBlock", "AddBreak");
    Block->Content.push_back(new BreakStmt(Loc, Block));
    return true;
}

/**
 *
 * @param Loc
 * @return true if no error occurs, otherwise false
 */
bool SemaBuilder::AddContinue(ASTBlock *Block, const SourceLocation &Loc) {
    FLY_DEBUG("ASTBlock", "AddContinue");
    Block->Content.push_back(new ContinueStmt(Loc, Block));
    return true;
}

/**
 * Add ASTIfBlock
 * @param Loc
 * @param Expr
 * @return
 */
ASTIfBlock* SemaBuilder::AddIfBlock(ASTBlock *Block, const SourceLocation &Loc, ASTExpr *Expr) {
    ASTIfBlock *IfBlock = new ASTIfBlock(Loc, Block, Expr);
    Block->Content.push_back(Block);
    return IfBlock;
}

/**
 * Add ASTSwitchBlock
 * @param Loc
 * @param Expr
 * @return
 */
ASTSwitchBlock *SemaBuilder::AddSwitchBlock(ASTBlock *Block, const SourceLocation &Loc, ASTExpr *Expr) {
    ASTSwitchBlock *SwitchBlock = new ASTSwitchBlock(Loc, Block, Expr);
    Block->Content.push_back(Block);
    return SwitchBlock;
}

/**
 * Add ASTWhileBlock
 * @param Loc
 * @param Expr
 * @return
 */
ASTWhileBlock *SemaBuilder::AddWhileBlock(ASTBlock *Block, const SourceLocation &Loc, ASTExpr *Expr) {
    ASTWhileBlock *WhileBlock = new ASTWhileBlock(Loc, Block, Expr);
    Block->Content.push_back(Block);
    return WhileBlock;
}

/**
 * Add ASTForBlock
 * @param Loc
 * @param Expr
 * @return
 */
ASTForBlock *SemaBuilder::AddForBlock(ASTBlock *Block, const SourceLocation &Loc) {
    ASTForBlock *ForBlock = new ASTForBlock(Loc, Block);
    Block->Content.push_back(Block);
    return ForBlock;
}

void SemaBuilder::setCondition(ASTForBlock *ForBlock, ASTExpr *Expr) {
    ASTExprStmt *ExprStmt = new ASTExprStmt(Expr->getLocation(), ForBlock->Cond);
    ExprStmt->setExpr(Expr);
    ForBlock->Cond->Clear();
    AddExprStmt(ForBlock->Cond, ExprStmt);
}

/**
 *
 * @return
 */
bool SemaBuilder::OnCloseBlock(ASTBlock *Block) {
    return false;
}
