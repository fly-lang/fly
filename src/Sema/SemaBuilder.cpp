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
#include "AST/ASTValue.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"

using namespace fly;

/**
 * Private constructor used from static Build()
 * @param S
 */
SemaBuilder::SemaBuilder(Sema &S) : S(S) {
    FLY_DEBUG("SemaBuilder", "SemaBuilder");
    S.Context = new ASTContext();
    AddNameSpace(ASTNameSpace::DEFAULT);
}

/**
 * Build the SemaBuilder for AST creation statically
 * @return
 */
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
 * @param DiagID
 * @return
 */
DiagnosticBuilder SemaBuilder::Diag(unsigned DiagID) const {
    return S.Diag(DiagID);
}

/**
 * Add an ASTNameSpace to the ASTContext if not exists yet
 * @param Name
 * @param ExternLib
 * @return the created or retrieved ASTNameSpace
 */
ASTNameSpace *SemaBuilder::AddNameSpace(const std::string &Name, bool ExternLib) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddNameSpace", "Name=" << Name << ", ExternLib=" << ExternLib);
    // Check if Name exist or add it
    ASTNameSpace *NameSpace = S.Context->NameSpaces.lookup(Name);
    if (NameSpace == nullptr) {
        NameSpace = new ASTNameSpace(Name, S.Context, ExternLib);
        S.Context->NameSpaces.insert(std::make_pair(Name, NameSpace));
    }
    return NameSpace;
}

/**
 * Create an ASTHeaderNode: only prototype declarations without definitions
 * For .fly.h file generation
 * @param Name
 * @param NameSpace
 * @return thee ASTHeaderNode
 */
ASTNode *SemaBuilder::CreateHeaderNode(const std::string &Name, std::string &NameSpace) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateHeaderNode", "Name=" << Name << ", NameSpace=" << NameSpace);
    return new ASTNode(Name, S.Context, true);
}

/**
 * Create an ASTNode with basic mandatory details: Name and NameSpace
 * If NameSpace doesn't exists it will be created
 * @param Name
 * @param NameSpace
 * @return the ASTNode
 */
ASTNode *SemaBuilder::CreateNode(const std::string &Name, const std::string &NameSpace) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateNode", "Name=" << Name << ", NameSpace=" << NameSpace);
    ASTNode *Node = new ASTNode(Name, S.Context, false);
    Node->NameSpace = AddNameSpace(NameSpace);
    return Node;
}

/**
 * Add an ASTNode to the ASTContext and in its ASTNameSpace
 * @param Node
 * @return true if no error occurs, otherwise false
 */
bool SemaBuilder::AddNode(ASTNode *Node) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddNode", "Node=" << Node->str());

    // Add to Nodes
    auto Pair = std::make_pair(Node->getName(), Node);
    // TODO check duplicated in namespace and context
    return Node->NameSpace->Nodes.insert(Pair).second && S.Context->Nodes.insert(Pair).second;
}

bool SemaBuilder::AddImport(ASTNode *Node, ASTImport * Import) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddImport", "Node=" << Node->str() << ", Import=" << Import->str());

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

    return false;
}

ASTGlobalVar * SemaBuilder::CreateGlobalVar(ASTNode *Node, SourceLocation Loc, ASTType *Type, const std::string &Name,
                                            VisibilityKind &Visibility, bool &Constant) {
    return new ASTGlobalVar(Loc, Node, Type, Name, Visibility, Constant);
}

bool SemaBuilder::AddGlobalVar(ASTNode *Node, ASTGlobalVar *GlobalVar, ASTValueExpr *Expr) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddGlobalVar",
                      "Node=" << Node->str() << ", GlobalVar=" << GlobalVar->str() << ", Expr=" << Expr->str());

    // Set Value Expr
    GlobalVar->Expr = Expr;

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
        // TODO check duplicate in namespace and node
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
        // TODO check duplicate in node
        return Node->GlobalVars.insert(Pair).second;
    }

    assert(0 && "Unknown GlobalVar Visibility");
}

bool SemaBuilder::AddFunction(ASTNode *Node, ASTFunction *Function) {
    FLY_DEBUG_MESSAGE("ASTNode", "AddFunction",
                      "Node=" << Node->str() << ", Function=" << Function->str());

    // Lookup into namespace for public var
    if(Function->Visibility == VisibilityKind::V_PUBLIC || Function->Visibility == VisibilityKind::V_DEFAULT) {
        const auto &FuncIt = Node->NameSpace->getFunctions().find(Function);

        if (FuncIt != Node->NameSpace->getFunctions().end()) { // This NameSpace already contains this Function
            Diag(Function->getLocation(), diag::err_duplicate_func) << Function->getName();
            return false;
        }

        // TODO check duplicate in node and namespace

        // Add into NameSpace for global resolution
        // Add into Node for local resolution
        ASTFunctionCall *Call = CreateFunctionCall(Function);
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

        // TODO check duplicate in node

        // Add into Node for local resolution (Private)
        if (Node->Functions.insert(Function).second) {
            return AddFunctionCall(Node, CreateFunctionCall(Function));
        }

        Diag(Function->getLocation(), diag::err_add_func) << Function->getName();
        return false;
    }

    assert(0 && "Unknown Function Visibility");
}

bool SemaBuilder::AddClass(ASTNode *Node, ASTClass *Class) {
    FLY_DEBUG_MESSAGE("ASTNode", "AddFunction",
                      "Node=" << Node->str() << ", Class" << Class->str());

    // Lookup into namespace
    // TODO Class scope differences
    // TODO check duplicate in namespace
    ASTClass *LookupClass = Node->NameSpace->getClasses().lookup(Class->getName());
    if (LookupClass) { // This NameSpace already contains this Function
        Diag(LookupClass->Location, diag::err_duplicate_class)  << LookupClass->getName();
        return false;
    }

    return Node->NameSpace->Classes.insert(std::make_pair(Class->getName(), Class)).second;
}

bool SemaBuilder::AddUnrefCall(ASTNode *Node, ASTFunctionCall *Call) {
    FLY_DEBUG_MESSAGE("ASTNode", "AddUnrefCall",
                      "Node=" << Node->str() << ", Call=" << Call->str());
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
    FLY_DEBUG_MESSAGE("ASTNode", "AddUnrefGlobalVar",
                      "Node=" << Node->str() << ", VarRef=" << VarRef->str());
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
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddComment", "Top=" << Top->str() << ", Comment=" << C);
    const char* t = " \t\n\r\f\v";
    C = C.substr(2, C.size()-4);
    C = C.erase(0, C.find_first_not_of(t));
    Top->Comment = C.erase(C.find_last_not_of(t) + 1);
    return false;
}

bool SemaBuilder::AddExternalGlobalVar(ASTNode *Node, ASTGlobalVar *GlobalVar) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddExternalGlobalVar",
                      "Node=" << Node->str() << ", GlobalVar=" << GlobalVar->str());
    // TODO Check duplicate
    return Node->ExternalGlobalVars.insert(std::make_pair(GlobalVar->getName(), GlobalVar)).second;
}

bool SemaBuilder::AddExternalFunction(ASTNode *Node, ASTFunction *Call) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddExternalFunction",
                      "Node=" << Node->str() << ", Call=" << Call->str());
    // TODO Check duplicate
    return Node->ExternalFunctions.insert(Call).second;
}

ASTParam *SemaBuilder::CreateParam(const SourceLocation &Loc, ASTType *Type, const std::string &Name, bool Constant) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateParam",
                      "Type=" << Type->str() << ", Name=" << Name << ", Constant=" << Constant);
    return new ASTParam(Loc, Type, Name, Constant);
}

bool SemaBuilder::AddFunctionParam(ASTFunction *Function, ASTParam *Param) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddExternalFunction",
                      "Function=" << Function->str() << ", Param=" << Param->str());
    // TODO Check duplicate
    Function->Params->List.push_back(Param);
    return AddLocalVar(Function->Body, Param, nullptr);
}

void SemaBuilder::AddVarArgs(ASTFunction *Function, ASTParam *Param) {
    Function->Params->Ellipsis = Param;
}

ASTFunctionCall *SemaBuilder::CreateFunctionCall(SourceLocation &Location, std::string &Name, std::string &NameSpace) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateFunctionCall",
                      "Name=" << Name << ", NameSpace=" << NameSpace);
    return new ASTFunctionCall(Location, Name, NameSpace);
}


ASTFunctionCall *SemaBuilder::CreateFunctionCall(ASTFunction *Function) {
    ASTFunctionCall *Call = new ASTFunctionCall(SourceLocation(),
                                                Function->getNameSpace()->getName(),
                                                Function->getName());
    Call->setDecl(Function);
    for (auto &Param : Function->Params->List) {
        ASTCallArg * Arg = CreateCallArg(Param->getType());
        AddCallArg(Call, Arg);
    }
    return Call;
}

bool SemaBuilder::AddFunctionCall(ASTNodeBase * Base, ASTFunctionCall *Call) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddFunctionCall", "Call=" << Call->str());
    const auto &It = Base->FunctionCalls.find(Call->getName());
    if (It == Base->FunctionCalls.end()) {
        std::vector<ASTFunctionCall *> TmpFunctionCalls;
        TmpFunctionCalls.push_back(Call);
        return Base->FunctionCalls.insert(std::make_pair(Call->getName(), TmpFunctionCalls)).second;
    }
    It->getValue().push_back(Call);
    return true;
}

/**
 * Add Call
 * @param Call
 * @return true if no error occurs, otherwise false
 */
bool SemaBuilder::AddFunctionCall(ASTBlock *Block, ASTFunctionCall *Call) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddFunctionCall", "Call=" << Call->str());
    ASTExprStmt *ExprStmt = new ASTExprStmt(Call->getLocation());
    ExprStmt->setExpr(new ASTFuncCallExpr(Call));
    Block->Content.push_back(ExprStmt);
    return Call->getDecl() || AddUnrefCall(Block->Top->getNode(), Call);
}

ASTCallArg *SemaBuilder::CreateCallArg(ASTType *Type) {
    return new ASTCallArg(Type);
}

ASTCallArg *SemaBuilder::CreateCallArg(ASTExpr *Expr) {
    return new ASTCallArg(Expr);
}

bool SemaBuilder::AddCallArg(ASTFunctionCall *Call, ASTCallArg *Arg) {
    Call->Args.push_back(Arg);
    return Arg;
}

bool SemaBuilder::setVarExpr(ASTVar *Var, ASTValueExpr *ValueExpr) {
    Var->Expr = ValueExpr;
    return false;
}

/**
 * Add ExprStmt to Content
 * @param ExprStmt
 * @return true if no error occurs, otherwise false
 */
bool SemaBuilder::AddExprStmt(ASTBlock *Block, ASTExprStmt *ExprStmt) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddExprStmt", "ExprStmt=" << ExprStmt->str());
    Block->Content.push_back(ExprStmt);
    return true;
}

ASTExpr *SemaBuilder::CreateExpr(const SourceLocation &Loc, ASTValue *Value) {
    return new ASTValueExpr(Value);
}

ASTExpr *SemaBuilder::CreateExpr(const SourceLocation &Loc, ASTFunctionCall *Call) {
    return new ASTFuncCallExpr(Call);
}

ASTExpr *SemaBuilder::CreateExpr(const SourceLocation &Loc, ASTVarRef *VarRef) {
    return new ASTVarRefExpr(VarRef);
}

ASTBoolValue *SemaBuilder::CreateValue(const SourceLocation &Loc, bool Val) {
    return new ASTBoolValue(Loc, Val);
}

ASTIntegerValue *SemaBuilder::CreateValue(const SourceLocation &Loc, int Val) {
    return new ASTIntegerValue(Loc, new ASTIntType(Loc), Val);
}

ASTFloatingValue *SemaBuilder::CreateValue(const SourceLocation &Loc, std::string Val) {
    return new ASTFloatingValue(Loc, new ASTFloatType(Loc), Val);
}

ASTLocalVar *SemaBuilder::CreateLocalVar(ASTBlock *Block, const SourceLocation &Loc, ASTType *Type,
                                      const std::string &Name, bool Constant) {
    ASTLocalVar *LocalVar = new ASTLocalVar(Loc, Type, Name, Constant);
    if (Type->getKind() == TYPE_ARRAY) {
        LocalVar->Expr = new ASTValueExpr(new ASTArrayValue(Loc, ((ASTArrayType *)Type)->getType()));
    }
    return LocalVar;
}

/**
 * Add Local Var
 * @param LocalVar
 * @return true if no error occurs, otherwise false
 */
bool SemaBuilder::AddLocalVar(ASTBlock *Block, ASTLocalVar *LocalVar, ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddLocalVar", "LocalVar=" << LocalVar->str());

    // Check if LocalVar have an Expression assigned
    if (!LocalVar->getExpr()) {  // No Expression: add to Undefined Vars, will be removed on AddLocalVarRef()
        Block->UndefVars.insert(std::pair<std::string, ASTLocalVar *>(LocalVar->getName(), LocalVar));
    }

    //Set CodeGen
    CodeGenLocalVar *CGLV = new CodeGenLocalVar(Block->Top->getNode()->getCodeGen(), LocalVar);
    LocalVar->CodeGen =CGLV;

    // Set Expr
    LocalVar->Expr = Expr;

    // Add LocalVar
    Block->Content.push_back(LocalVar);
    Block->Top->LocalVars.push_back(LocalVar); //Useful for Alloca into CodeGen
    return Block->LocalVars.insert(std::pair<std::string, ASTLocalVar *>(LocalVar->getName(), LocalVar)).second;
}

ASTVarRef *SemaBuilder::CreateVarRef(ASTLocalVar *LocalVar) {
    ASTVarRef *VarRef = new ASTVarRef(LocalVar->Location, LocalVar->Name);
    VarRef->setDecl(LocalVar);
    return VarRef;
}

ASTVarRef *SemaBuilder::CreateVarRef(ASTGlobalVar *GlobalVar) {
    ASTVarRef *VarRef = new ASTVarRef(GlobalVar->Location, GlobalVar->Name, GlobalVar->NameSpace->getName());
    VarRef->setDecl(GlobalVar);
    return VarRef;
}

/**
 * Add Var Assign Stmt
 * @param VarAssign
 * @return true if no error occurs, otherwise false
 */
bool SemaBuilder::AddVarAssign(ASTBlock *Block, ASTVarAssign *VarAssign) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddVarAssign", "VarAssign=" << VarAssign->str());

    //TODO check null VarAssign->getVarRef()
    //tODo check null VarAssign->getExpr()

    // Add Var to Block Content
    Block->Content.push_back(VarAssign);
    // TODO add to Unrefs
    return true;
}

bool SemaBuilder::RemoveUndefVar(ASTBlock *Block, ASTVarRef *VarRef) {
    if (Block->UndefVars.lookup(VarRef->getName())) {
        return Block->UndefVars.erase(VarRef->getName());
    }
    return false;
}

/**
 * Add Return
 * @param Loc
 * @param Expr
 * @return true if no error occurs, otherwise false
 */
bool SemaBuilder::AddReturn(ASTBlock *Block, const SourceLocation &Loc, ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddReturn", "Expr=" << (Expr ? Expr->str() : ""));
    ASTReturn *Return = new ASTReturn(Loc, Expr);
    Block->Content.push_back(Return);
    return true;
}

/**
 *
 * @param Loc
 * @return true if no error occurs, otherwise false
 */
bool SemaBuilder::AddBreak(ASTBlock *Block, const SourceLocation &Loc) {
    FLY_DEBUG("SemaBuilder", "AddBreak");
    Block->Content.push_back(new BreakStmt(Loc));
    return true;
}

/**
 *
 * @param Loc
 * @return true if no error occurs, otherwise false
 */
bool SemaBuilder::AddContinue(ASTBlock *Block, const SourceLocation &Loc) {
    FLY_DEBUG("SemaBuilder", "AddContinue");
    Block->Content.push_back(new ContinueStmt(Loc));
    return true;
}

ASTBlock* SemaBuilder::CreateBlock(const SourceLocation &Loc) {
    return new ASTBlock(Loc);
}

/**
 * Add ASTIfBlock
 * @param Loc
 * @param Expr
 * @return
 */
ASTIfBlock* SemaBuilder::AddIfBlock(ASTBlock *Block, const SourceLocation &Loc, ASTExpr *Expr) {
    ASTIfBlock *IfBlock = new ASTIfBlock(Loc, Expr);
    // The UndefVars are copied from Parent to this if block
    IfBlock->UndefVars = Block->UndefVars;
    Block->Content.push_back(Block);
    return IfBlock;
}

ASTElsifBlock *SemaBuilder::AddElsifBlock(ASTIfBlock *IfBlock, const SourceLocation &Loc, ASTExpr *Expr) {
    if (!IfBlock) {
        Diag(Loc, diag::err_missing_if_first);
        return nullptr;
    }
    if (IfBlock->ElseBlock) {
        Diag(Loc, diag::err_elseif_after_else);
        return nullptr;
    }
    ASTElsifBlock *Elsif = new ASTElsifBlock(Loc, IfBlock, Expr);
    IfBlock->ElsifBlocks.push_back(Elsif);
    return Elsif;
}

ASTElseBlock *SemaBuilder::AddElseBlock(ASTIfBlock *IfBlock, const SourceLocation &Loc) {
    if (!IfBlock) {
        Diag(Loc, diag::err_missing_if_first);
        return nullptr;
    }
    if (IfBlock->ElseBlock) {
        Diag(Loc, diag::err_else_after_else);
        return nullptr;
    }
    IfBlock->ElseBlock = new ASTElseBlock(Loc, IfBlock);
    return IfBlock->ElseBlock;
}

/**
 * Add ASTSwitchBlock
 * @param Loc
 * @param Expr
 * @return
 */
ASTSwitchBlock *SemaBuilder::AddSwitchBlock(ASTBlock *Block, const SourceLocation &Loc, ASTExpr *Expr) {
    ASTSwitchBlock *SwitchBlock = new ASTSwitchBlock(Loc, Expr);
    if (Expr->getExprKind() != EXPR_REF_VAR && Expr->getExprKind() != EXPR_REF_FUNC) {
        Diag(Loc, diag::err_switch_expression);
        return nullptr;
    }
    Block->Content.push_back(SwitchBlock);
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
 * Create an ASTForBlock
 * @param Loc
 * @param Condition
 * @param PostBlock
 * @param LoopBlock
 * @return
 */
ASTForBlock *SemaBuilder::CreateForBlock(const SourceLocation &Loc, ASTExpr *Condition, ASTBlock *PostBlock,
                                         ASTBlock *LoopBlock) {
    ASTForBlock *ForBlock = new ASTForBlock(Loc);
    ForBlock->Condition = Condition;
    ForBlock->Post = PostBlock;
    ForBlock->Loop = LoopBlock;
    return ForBlock;
}

/**
 * Add ASTForBlock
 * @param Loc
 * @param Expr
 * @return
 */
bool SemaBuilder::AddForBlock(ASTBlock *Block, ASTForBlock *ForBlock) {
    Block->Content.push_back(ForBlock);
    return true;
}

/**
 *
 * @return
 */
bool SemaBuilder::OnCloseBlock(ASTBlock *Block) {
    return false;
}

ASTStmt *SemaBuilder::getLastStmt(ASTBlock *Block) {
    if (!Block->isEmpty()) { // search back if there is another if before this
        return Block->Content.at(Block->Content.size() - 1);
    }
    return nullptr;
}
