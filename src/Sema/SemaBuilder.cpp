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
#include "Sema/SemaNumber.h"
#include "CodeGen/CodeGen.h"
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
    S.Context->DefaultNameSpace = AddNameSpace(ASTNameSpace::DEFAULT);
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
ASTNode *SemaBuilder::CreateNode(const std::string &Name, std::string &NameSpace) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateNode", "Name=" << Name << ", NameSpace=" << NameSpace);
    ASTNode *Node = new ASTNode(Name, S.Context, false);

    // Fix empty namespace with Default
    if (NameSpace.empty()) {
        FLY_DEBUG_MESSAGE("SemaBuilder", "CreateNode", "set NameSpace to Default");
        NameSpace = ASTNameSpace::DEFAULT;
    }

    Node->NameSpace = AddNameSpace(NameSpace);
    return Node;
}

ASTImport *SemaBuilder::CreateImport(const SourceLocation &NameLoc, llvm::StringRef Name) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateImport", "Name=" << Name);
    std::string NameStr = Name.str();
    return new ASTImport(NameLoc, NameStr);
}

ASTImport *SemaBuilder::CreateImport(const SourceLocation &NameLoc, llvm::StringRef Name,
                                     const SourceLocation &AliasLoc, llvm::StringRef Alias) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateImport", "Name=" << Name << ", Alias=" << Alias);
    std::string NameStr = Name.str();
    std::string AliasStr = Alias.str();
    return new ASTImport(NameLoc, NameStr, AliasLoc, AliasStr);
}

/**
 * Create an ASTGlobalVar
 * @param Node
 * @param Loc
 * @param Type
 * @param Name
 * @param Visibility
 * @param Constant
 * @return
 */
ASTGlobalVar *SemaBuilder::CreateGlobalVar(ASTNode *Node, const SourceLocation Loc, ASTType *Type, const std::string &Name,
                                            VisibilityKind &Visibility, bool &Constant) {
    return new ASTGlobalVar(Loc, Node, Type, Name, Visibility, Constant);
}

/**
 * Create an ASTFunction
 * @param Node
 * @param Loc
 * @param Type
 * @param Name
 * @param Visibility
 * @return
 */
ASTFunction *SemaBuilder::CreateFunction(ASTNode *Node, const SourceLocation Loc, ASTType *Type,
                                         const std::string &Name, VisibilityKind &Visibility) {
    ASTFunction *F = new ASTFunction(Loc, Node, Type, Name, Visibility);
    F->Body->Top = F;
    return F;
}

/**
 * Create an ASTClass
 * @param Node
 * @param Loc
 * @param Name
 * @param Visibility
 * @param Constant
 * @return
 */
ASTClass *SemaBuilder::CreateClass(ASTNode *Node, const SourceLocation Loc, const std::string &Name,
                                   VisibilityKind &Visibility, bool &Constant) {
    return new ASTClass(Loc, Node, Name, Visibility, Constant);
}

/**
 * Create an ASTFunctionCall
 * @param Location
 * @param Name
 * @param NameSpace
 * @return
 */
ASTFunctionCall *SemaBuilder::CreateFunctionCall(const SourceLocation &Loc, std::string &Name, std::string &NameSpace) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateFunctionCall",
                      "Name=" << Name << ", NameSpace=" << NameSpace);
    return new ASTFunctionCall(Loc, NameSpace, Name);
}

/**
 * Create an ASTFunctionCall from ASTFunction
 * @param Function
 * @return
 */
ASTFunctionCall *SemaBuilder::CreateFunctionCall(ASTFunction *Function) {
    ASTFunctionCall *Call = new ASTFunctionCall(SourceLocation(),
                                                Function->getNameSpace()->getName(),
                                                Function->getName());
    Call->Def = Function;
    for (auto &Param : Function->Params->List) {
        ASTExpr * Arg = CreateExpr(CreateDefaultValue(Param->getType()));
        AddCallArg(Call, Arg);
    }
    return Call;
}

ASTType *SemaBuilder::CreateBoolType(const SourceLocation &Loc) {
    return new ASTBoolType(Loc);
}

ASTType *SemaBuilder::CreateByteType(const SourceLocation &Loc) {
    return new ASTByteType(Loc);
}

ASTType *SemaBuilder::CreateUShortType(const SourceLocation &Loc) {
    return new ASTUShortType(Loc);
}

ASTType *SemaBuilder::CreateShortType(const SourceLocation &Loc) {
    return new ASTShortType(Loc);;
}

ASTType *SemaBuilder::CreateUIntType(const SourceLocation &Loc) {
    return new ASTUIntType(Loc);
}

ASTType *SemaBuilder::CreateIntType(const SourceLocation &Loc) {
    return new ASTIntType(Loc);
}

ASTType *SemaBuilder::CreateULongType(const SourceLocation &Loc) {
    return new ASTULongType(Loc);
}

ASTType *SemaBuilder::CreateLongType(const SourceLocation &Loc) {
    return new ASTLongType(Loc);
}

ASTType *SemaBuilder::CreateFloatType(const SourceLocation &Loc) {
    return new ASTFloatType(Loc);
}

ASTType *SemaBuilder::CreateDoubleType(const SourceLocation &Loc) {
    return new ASTDoubleType(Loc);
}

ASTType *SemaBuilder::CreateVoidType(const SourceLocation &Loc) {
    return new ASTVoidType(Loc);
}

ASTArrayType *SemaBuilder::CreateArrayType(const SourceLocation &Loc, ASTType *Type, ASTExpr *Size) {
    return new ASTArrayType(Loc, Type, Size);
}

ASTClassType *SemaBuilder::CreateClassType(const SourceLocation &Loc, StringRef Name, StringRef NameSpace) {
    return new ASTClassType(Loc, Name.str(), NameSpace.str());
}

ASTClassType *SemaBuilder::CreateClassType(ASTClass *Class) {
    ASTClassType *ClassType = new ASTClassType(Class->Location, Class->Name, Class->NameSpace->Name);
    ClassType->Def = Class;
    return ClassType;
}

ASTLocalVar *SemaBuilder::CreateLocalVar(const SourceLocation &Loc, ASTType *Type,
                                         const std::string &Name, bool Constant, ASTExpr *Expr) {
    ASTLocalVar *LocalVar = new ASTLocalVar(Loc, Type, Name, Constant, Expr);
    if (Type->getKind() == TYPE_ARRAY) {
        LocalVar->Expr = new ASTValueExpr(new ASTArrayValue(Loc));
    }
    return LocalVar;
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

bool SemaBuilder::AddGlobalVar(ASTNode *Node, ASTGlobalVar *GlobalVar, ASTExpr *Expr) {
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


bool SemaBuilder::AddUnrefGlobalVar(ASTNode *Node, ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("ASTNode", "AddUnrefGlobalVar",
                      "Node=" << Node->str() << ", VarRef=" << VarRef->str());
    ASTUnrefGlobalVar *Unref = new ASTUnrefGlobalVar(Node, *VarRef);
    ASTImport *Import;
    if (VarRef->getNameSpace().empty()) { // Add to Unref of current Node
        Node->UnrefGlobalVars.push_back(Unref);
    } else if (VarRef->getNameSpace() == Node->NameSpace->getName()) { // Add to Unref of current NameSpace
        Node->NameSpace->UnrefGlobalVars.push_back(Unref);
    } else if ((Import = Node->FindImport(VarRef->getNameSpace()))) { // Add to Unref of current Import
        Import->UnrefGlobalVars.push_back(Unref);
    } else {
        Diag(VarRef->getLocation(), diag::err_unref_var);
        return false;
    }
    return true;
}

bool SemaBuilder::AddUnrefCall(ASTNode *Node, ASTFunctionCall *Call) {
    FLY_DEBUG_MESSAGE("ASTNode", "AddUnrefCall",
                      "Node=" << Node->str() << ", Call=" << Call->str());
    ASTUnrefCall *Unref = new ASTUnrefCall(Node, Call);
    ASTImport *Import;
    if (Call->getNameSpace().empty()) {
        Node->UnrefFunctionCalls.push_back(Unref); // Unref of Node or from a Namespace not specified
    } else if (Call->getNameSpace() == Node->NameSpace->getName()) { // call must be resolved into current namespace
        Node->NameSpace->UnrefFunctionCalls.push_back(Unref);
    } else if ((Import = Node->FindImport(Call->getNameSpace()))) {
        Import->UnrefFunctionCalls.push_back(Unref);
    } else {
        Diag(Call->getLocation(), diag::err_unref_call);
        return false;
    }
    return true;
}

bool SemaBuilder::AddComment(ASTTopDef *Top, std::string &C) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddComment", "Top=" << Top->str() << ", Comment=" << C);
    if (!C.empty()) {
        const char *t = " \t\n\r\f\v";
        C = C.substr(2, C.size() - 4);
        C = C.erase(0, C.find_first_not_of(t)); // TODO Check
        Top->Comment = C.erase(C.find_last_not_of(t) + 1);
        return true;
    }
    return true;
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

bool SemaBuilder::AddFunctionParam(ASTFunction *Function, ASTParam *Param) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddExternalFunction",
                      "Function=" << Function->str() << ", Param=" << Param->str());
    // TODO Check duplicate
    Function->Params->List.push_back(Param);
    return AddLocalVar(Function->Body, Param, false);
}

void SemaBuilder::AddVarArgs(ASTFunction *Function, ASTParam *Param) {
    Function->Params->Ellipsis = Param;
}

bool SemaBuilder::AddFunctionCall(ASTNodeBase * Base, ASTFunctionCall *Call) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddFunctionCall",
                      "Base=" << Call->str() <<
                      ", Call=" << Call->str());
    const auto &It = Base->FunctionCalls.find(Call->getName());
    // Create a List of Function Call with same name
    if (It == Base->FunctionCalls.end()) {
        std::vector<ASTFunctionCall *> TmpFunctionCalls;
        TmpFunctionCalls.push_back(Call);
        return Base->FunctionCalls.insert(std::make_pair(Call->getName(), TmpFunctionCalls)).second;
    }

    // Add to existing one Function Call lists with same name
    It->getValue().push_back(Call);
    return true;
}

bool SemaBuilder::AddCallArg(ASTFunctionCall *Call, ASTExpr *Expr) {
    Call->Args.push_back(Expr);
    return Expr;
}

bool SemaBuilder::setVarExpr(ASTVar *Var, ASTExpr *Expr) {
    Var->Expr = Expr;
    return true;
}

ASTParam *SemaBuilder::CreateParam(const SourceLocation &Loc, ASTType *Type, const std::string &Name, bool Constant) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateParam",
                      "Type=" << Type->str() << ", Name=" << Name << ", Constant=" << Constant);
    return new ASTParam(Loc, Type, Name, Constant, nullptr);
}

ASTExprStmt *SemaBuilder::CreateExprStmt(const SourceLocation &Loc, ASTExpr *Expr) {
    return new ASTExprStmt(Loc, Expr);
}

ASTEmptyExpr *SemaBuilder::CreateExpr(const SourceLocation &Loc) {
    return new ASTEmptyExpr(Loc);
}

ASTValueExpr *SemaBuilder::CreateExpr(ASTValue *Value) {
    return new ASTValueExpr(Value);
}

ASTFuncCallExpr *SemaBuilder::CreateExpr(ASTFunctionCall *Call) {
    return new ASTFuncCallExpr(Call);
}

ASTVarRefExpr *SemaBuilder::CreateExpr(ASTVarRef *VarRef) {
    return new ASTVarRefExpr(VarRef);
}

ASTNullValue *SemaBuilder::CreateNullValue(const SourceLocation &Loc) {
    return new ASTNullValue(Loc);
}

ASTBoolValue *SemaBuilder::CreateBoolValue(const SourceLocation &Loc, bool Val) {
    return new ASTBoolValue(Loc, Val);
}

ASTIntegerValue *SemaBuilder::CreateIntegerValue(const SourceLocation &Loc, uint64_t Val, bool Negative) {
    return new ASTIntegerValue(Loc, Val, Negative);
}

ASTIntegerValue *SemaBuilder::CreateCharValue(const SourceLocation &Loc, char Val) {
    return CreateIntegerValue(Loc, Val, false);
}

ASTFloatingValue *SemaBuilder::CreateFloatingValue(const SourceLocation &Loc, double Val) {
    std::string StrVal = std::to_string(Val);
    return new ASTFloatingValue(Loc, StrVal);
}

ASTValue *SemaBuilder::CreateValue(const SourceLocation &Loc, std::string &Val) {
    return SemaNumber::fromString(Loc, Val);
}

ASTArrayValue *SemaBuilder::CreateArrayValue(const SourceLocation &Loc) {
    return new ASTArrayValue(Loc);
}

ASTValue *SemaBuilder::CreateDefaultValue(ASTType *Type) {
    ASTValue *Value;
    if (Type->isBool()) {
        Value = CreateBoolValue(Type->getLocation(), false);
    } else if (Type->isNumber()) {
        std::string Zero = "0";
        Value = CreateValue(Type->getLocation(), Zero);
    } else if (Type->isArray()) {
        Value = CreateArrayValue(Type->getLocation());
    } else if (Type->isClass()) {
        Value = CreateNullValue(Type->getLocation());
    } else {
        assert("Unknown type");
    }
    return Value;
}

ASTVarAssign *SemaBuilder::CreateVarAssign(const SourceLocation &Loc, ASTVarRef * VarRef, ASTExpr *Expr) {
    return new ASTVarAssign(Loc, VarRef, Expr);
}

ASTVarRef *SemaBuilder::CreateVarRef(const SourceLocation &Loc, StringRef Name, StringRef NameSpace) {
    ASTVarRef *VarRef = new ASTVarRef(Loc, std::string(Name), std::string(NameSpace));
    return VarRef;
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

ASTBlock* SemaBuilder::CreateBlock(const SourceLocation &Loc) {
    return new ASTBlock(Loc);
}

ASTIfBlock *SemaBuilder::CreateIfBlock(const SourceLocation &Loc, ASTExpr *Condition) {
    ASTIfBlock *IfBlock = new ASTIfBlock(Loc, Condition);
    return IfBlock;
}

ASTElsifBlock *SemaBuilder::CreateElsifBlock(const SourceLocation &Loc, ASTExpr *Condition) {
    ASTElsifBlock *ElsifBlock = new ASTElsifBlock(Loc, Condition);
    return ElsifBlock;
}

ASTElseBlock *SemaBuilder::CreateElseBlock(const SourceLocation &Loc) {
    ASTElseBlock *ElseBlock = new ASTElseBlock(Loc);
    return ElseBlock;
}

ASTSwitchBlock *SemaBuilder::CreateSwitchBlock(const SourceLocation &Loc, ASTExpr *Expr) {
    return new ASTSwitchBlock(Loc, Expr);
}

ASTSwitchCaseBlock *SemaBuilder::CreateSwitchCaseBlock(const SourceLocation &Loc, ASTExpr *Condition) {
    return new ASTSwitchCaseBlock(Loc, Condition);
}

ASTSwitchDefaultBlock *SemaBuilder::CreateSwitchDefaultBlock(const SourceLocation &Loc) {
    return new ASTSwitchDefaultBlock(Loc);
}

ASTWhileBlock *SemaBuilder::CreateWhileBlock(const SourceLocation &Loc, ASTExpr *Condition) {
    return new ASTWhileBlock(Loc, Condition);
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
 * Add ExprStmt to Content
 * @param ExprStmt
 * @return true if no error occurs, otherwise false
 */
bool SemaBuilder::AddStmt(ASTBlock *Block, ASTStmt *Stmt) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddStmt", "ExprStmt=" << Stmt->str());
    Block->Content.push_back(Stmt);
    return true;
}

/**
 * Add Local Var
 * @param LocalVar
 * @return true if no error occurs, otherwise false
 */
bool SemaBuilder::AddLocalVar(ASTBlock *Block, ASTLocalVar *LocalVar, bool PushToContent) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddLocalVar", "LocalVar=" << LocalVar->str());

    // Check if LocalVar have an Expression assigned
    if (!LocalVar->getExpr()) {  // No Expression: add to Undefined Vars, will be removed on AddLocalVarRef()
        Block->UndefVars.insert(std::pair<std::string, ASTLocalVar *>(LocalVar->getName(), LocalVar));
    }

    // Add LocalVar
    if (PushToContent)
        Block->Content.push_back(LocalVar);
    Block->Top->LocalVars.push_back(LocalVar); //Useful for Alloca into CodeGen
    return Block->LocalVars.insert(std::pair<std::string, ASTLocalVar *>(LocalVar->getName(), LocalVar)).second;
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

/**
 * Add ASTIfBlock
 * @param Loc
 * @param Expr
 * @return
 */
bool SemaBuilder::AddIfBlock(ASTBlock *Parent, ASTIfBlock *IfBlock) {
    // The UndefVars are copied from Parent to this if block
    IfBlock->UndefVars = Parent->UndefVars;
    Parent->Content.push_back(IfBlock);
    return IfBlock;
}

bool SemaBuilder::AddElsifBlock(ASTIfBlock *IfBlock, ASTElsifBlock *ElsifBlock) {
    if (!IfBlock) {
        Diag(ElsifBlock->getLocation(), diag::err_missing_if_first);
        return false;
    }
    if (IfBlock->ElseBlock) {
        Diag(ElsifBlock->getLocation(), diag::err_elseif_after_else);
        return false;
    }
    IfBlock->ElsifBlocks.push_back(ElsifBlock);
    return !IfBlock->isEmpty();
}

bool SemaBuilder::AddElseBlock(ASTIfBlock *IfBlock, ASTElseBlock *ElseBlock) {
    if (!IfBlock) {
        Diag(ElseBlock->getLocation(), diag::err_missing_if_first);
        return false;
    }
    if (IfBlock->ElseBlock) {
        Diag(ElseBlock->getLocation(), diag::err_else_after_else);
        return false;
    }
    IfBlock->ElseBlock =  ElseBlock;
    return IfBlock->ElseBlock;
}

/**
 * Add ASTSwitchBlock
 * @param Loc
 * @param Expr
 * @return
 */
bool SemaBuilder::AddSwitchBlock(ASTBlock *Parent, ASTSwitchBlock * SwitchBlock) {
    if (SwitchBlock->Expr->getExprKind() != EXPR_REF_VAR && SwitchBlock->Expr->getExprKind() != EXPR_REF_FUNC) {
        Diag(SwitchBlock->getLocation(), diag::err_switch_expression);
        return false;
    }
    Parent->Content.push_back(SwitchBlock);
    return SwitchBlock;
}

bool SemaBuilder::AddSwitchCaseBlock(ASTSwitchBlock * SwitchBlock, ASTSwitchCaseBlock * CaseBlock) {
    SwitchBlock->Cases.push_back(CaseBlock);
    return CaseBlock;
}

bool SemaBuilder::setSwitchDefaultBlock(ASTSwitchBlock * SwitchBlock, ASTSwitchDefaultBlock *DefaultBlock) {
    SwitchBlock->Default = DefaultBlock;
    return DefaultBlock;
}

/**
 * Add ASTWhileBlock
 * @param Loc
 * @param Expr
 * @return
 */
bool SemaBuilder::AddWhileBlock(ASTBlock *Parent, ASTWhileBlock *WhileBlock) {
    Parent->Content.push_back(WhileBlock);
    return WhileBlock;
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

bool SemaBuilder::AddArrayValue(ASTArrayValue *Array, ASTValue *Value) {
    Array->Values.push_back(Value);
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
