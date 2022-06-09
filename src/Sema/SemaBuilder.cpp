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

/**
 * Create an ASTFunctionCall
 * @param Location
 * @param Name
 * @param NameSpace
 * @return
 */
ASTFunctionCall *SemaBuilder::CreateFunctionCall(ASTStmt *Stmt, const SourceLocation &Loc, std::string &Name, std::string &NameSpace) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateDefFunctionCall",
                      "Name=" << Name << ", NameSpace=" << NameSpace);
    ASTFunctionCall *Call = new ASTFunctionCall(Loc, NameSpace, Name);
    Call->Stmt = Stmt;
    return Call;
}

ASTArg *SemaBuilder::CreateArg(const SourceLocation &Loc) {
    ASTArg *Arg = new ASTArg(Loc);
    return Arg;
}

ASTParam *SemaBuilder::CreateParam(const SourceLocation &Loc, ASTType *Type, const std::string &Name, bool Constant) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateParam",
                      "Type=" << Type->str() << ", Name=" << Name << ", Constant=" << Constant);
    ASTParam *Param = new ASTParam(Loc, Type, Name, Constant);
    return Param;
}

ASTLocalVar *SemaBuilder::CreateLocalVar(const SourceLocation &Loc, ASTType *Type,
                                         const std::string &Name, bool Constant) {
    ASTLocalVar *LocalVar = new ASTLocalVar(Loc, Type, Name, Constant);
    if (Type->getKind() == TYPE_ARRAY) {
        LocalVar->Expr = new ASTValueExpr(new ASTArrayValue(Loc));
    }

    return LocalVar;
}

ASTVarAssign *SemaBuilder::CreateVarAssign(ASTVarRef *VarRef) {
    ASTVarAssign *VarAssign = new ASTVarAssign(VarRef->getLocation(), VarRef);
    return VarAssign;
}

ASTReturn *SemaBuilder::CreateReturn(const SourceLocation &Loc) {
    ASTReturn *Return = new ASTReturn(Loc);
    return Return;
}

ASTBreak *SemaBuilder::CreateBreak(const SourceLocation &Loc) {
    ASTBreak *Break = new ASTBreak(Loc);
    return Break;
}

ASTContinue *SemaBuilder::CreateContinue(const SourceLocation &Loc) {
    ASTContinue *Continue = new ASTContinue(Loc);
    return Continue;
}

ASTExprStmt *SemaBuilder::CreateExprStmt(const SourceLocation &Loc) {
    ASTExprStmt *ExprStmt = new ASTExprStmt(Loc);
    return ExprStmt;
}

ASTVarRef *SemaBuilder::CreateVarRef(const SourceLocation &Loc, StringRef Name, StringRef NameSpace) {
    ASTVarRef *VarRef = new ASTVarRef(Loc, std::string(Name), std::string(NameSpace));
    return VarRef;
}

ASTVarRef *SemaBuilder::CreateVarRef(ASTLocalVar *LocalVar) {
    ASTVarRef *VarRef = new ASTVarRef(LocalVar->Location, LocalVar->Name);
    VarRef->Def = LocalVar;
    return VarRef;
}

ASTVarRef *SemaBuilder::CreateVarRef(ASTGlobalVar *GlobalVar) {
    ASTVarRef *VarRef = new ASTVarRef(GlobalVar->Location, GlobalVar->Name, GlobalVar->NameSpace->getName());
    VarRef->Def = GlobalVar;
    return VarRef;
}

ASTEmptyExpr *SemaBuilder::CreateExpr(const SourceLocation &Loc) {
    return new ASTEmptyExpr(Loc);
}

ASTValueExpr *SemaBuilder::CreateExpr(ASTStmt *Stmt, ASTValue *Value) {
    ASTValueExpr *Expr = new ASTValueExpr(Value);
    Expr->Stmt = Stmt;
    return Expr;
}

ASTFunctionCallExpr *SemaBuilder::CreateExpr(ASTStmt *Stmt, ASTFunctionCall *Call) {
    ASTFunctionCallExpr *Arg = new ASTFunctionCallExpr(Call);
    Arg->Stmt = Stmt;
    return Arg;
}

ASTVarRefExpr *SemaBuilder::CreateExpr(ASTStmt *Stmt, ASTVarRef *VarRef) {
    ASTVarRefExpr *VarRefExpr = new ASTVarRefExpr(VarRef);
    VarRefExpr->Stmt = Stmt;
    return VarRefExpr;
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
ASTForBlock *SemaBuilder::CreateForBlock(const SourceLocation &Loc, ASTBlock *PostBlock,
                                         ASTBlock *LoopBlock) {
    ASTForBlock *ForBlock = new ASTForBlock(Loc);
    ForBlock->Post = PostBlock;
    ForBlock->Loop = LoopBlock;
    return ForBlock;
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

    // Set Var Expr
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

bool SemaBuilder::InsertFunction(ASTNodeBase *Base, ASTFunction *Function) {
    // Functions is llvm::StringMap<std::map <uint64_t, llvm::SmallVector <ASTFunction *, 4>>>
    const auto &StrMapIt = Base->getFunctions().find(Function->getName());

    // This Node not contains a Function with this Function->Name
    if (StrMapIt == Base->getFunctions().end()) {

        // add to llvm::SmallVector
        llvm::SmallVector<ASTFunction *, 4> Vect;
        Vect.push_back(Function);

        // add to std::map
        std::map<uint64_t, llvm::SmallVector<ASTFunction *, 4>> IntMap;
        IntMap.insert(std::make_pair(Function->Params->getSize(),Vect));

        // add to llvm::StringMap
        return Base->Functions.insert(std::make_pair(Function->getName(), IntMap)).second;
    } else { // This Node contains a Function with this Function->Name
        const auto &IntMapIt = StrMapIt->getValue().find(Function->Params->getSize());
        if (IntMapIt == StrMapIt->getValue().end()) { // but not have the same number of Params

            // add to llvm::SmallVector
            llvm::SmallVector<ASTFunction *, 4> Vect;
            Vect.push_back(Function);

            // add to std::map
            std::pair<uint64_t, SmallVector<ASTFunction *, 4>> IntMapPair = std::make_pair(
                    Function->Params->getSize(), Vect);

            std::map<uint64_t, llvm::SmallVector<ASTFunction *, 4>> IntMap = StrMapIt->getValue();
            return IntMap.insert(std::make_pair(Function->Params->getSize(),Vect)).second;
        } else { // This Node contains a Function with this Function->Name and same number of Params

            bool DifferentParamTypes = true;
            for (auto &NodeFunc : IntMapIt->second) {
                for (auto &NodeFuncParam : NodeFunc->getParams()->List) {
                    for (auto &Param : Function->getParams()->getList()) {
                        if (!S.isEquals(NodeFuncParam, Param)) {
                            DifferentParamTypes = false;
                            break;
                        }
                    }
                }
            }

            // Check Parameter Types
            if (DifferentParamTypes) { // Add the new Function
                SmallVector<ASTFunction *, 4> Vect = IntMapIt->second;
                Vect.push_back(Function);
                return true;
            } else { // Function is duplicated
                // This Node already contains this Function
                Diag(Function->getLocation(), diag::err_duplicate_func) << Function->getName();
                return false;
            }
        }
    }
}

bool SemaBuilder::AddFunction(ASTNode *Node, ASTFunction *Function) {
    FLY_DEBUG_MESSAGE("ASTNode", "AddFunction",
                      "Node=" << Node->str() << ", Function=" << Function->str());

    // Lookup into namespace for public var
    if(Function->Visibility == VisibilityKind::V_PUBLIC || Function->Visibility == VisibilityKind::V_DEFAULT) {

        // Add into NameSpace for global resolution
        // Add into Node for local resolution
        return InsertFunction(Node->NameSpace, Function) && InsertFunction(Node, Function);
    }

    // Lookup into node for private var
    if (Function->Visibility == VisibilityKind::V_PRIVATE) {

        // Add into Node for local resolution
        return InsertFunction(Node, Function);
    }

    assert(0 && "Unknown Function Visibility");
}

bool SemaBuilder::AddFunctionParam(ASTFunction *Function, ASTParam *Param, ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddExternalFunction",
                      "Function=" << Function->str() << ", Param=" << Param->str());
    // TODO Check duplicate
    Param->Expr = Expr;
    if (Expr && Expr->Stmt)
        Expr->Stmt = Param;
    Function->Params->List.push_back(Param);
    Param->Parent = Function->Body;

    Function->LocalVars.push_back(Param); //Useful for Alloca into CodeGen
    return Function->Body->LocalVars
        .insert(std::pair<std::string, ASTLocalVar *>(Param->getName(), Param)).second;
}

void SemaBuilder::AddFunctionVarParams(ASTFunction *Function, ASTParam *Param) {
    Function->Params->Ellipsis = Param;
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

bool SemaBuilder::AddArrayValue(ASTArrayValue *Array, ASTValue *Value) {
    Array->Values.push_back(Value);
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

/**
 * Add ExprStmt to Content
 * @param ExprStmt
 * @return true if no error occurs, otherwise false
 */
bool SemaBuilder::AddStmt(ASTBlock *Block, ASTStmt *Stmt) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddStmt", "ExprStmt=" << Stmt->str());
    Stmt->Parent = Block;
    Block->Content.push_back(Stmt);
    return true;
}

bool SemaBuilder::AddStmt(ASTBlock *Block, ASTExprStmt *ExprStmt, ASTExpr *Expr) {
    ExprStmt->Expr = Expr;
    Expr->Stmt = ExprStmt;
    return AddStmt(Block, ExprStmt);
}

/**
 * Add Local Var
 * @param LocalVar
 * @return true if no error occurs, otherwise false
 */
bool SemaBuilder::AddLocalVar(ASTBlock *Block, ASTLocalVar *LocalVar, ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddLocalVar", "LocalVar=" << LocalVar->str());

    // Check Undefined Var: if LocalVar have an Expression assigned
    if (!Expr) {  // No Expression: add to Undefined Vars, will be removed on AddLocalVarRef()
        Block->UndefVars.insert(std::pair<std::string, ASTLocalVar *>(LocalVar->getName(), LocalVar));
    } else {
        LocalVar->Expr = Expr;
        Expr->Stmt = LocalVar;
    }

    // Add LocalVar
    AddStmt(Block, LocalVar);

    Block->Top->LocalVars.push_back(LocalVar); //Useful for Alloca into CodeGen
    return Block->LocalVars.insert(std::pair<std::string, ASTLocalVar *>(LocalVar->getName(), LocalVar)).second;
}

bool SemaBuilder::AddFunctionCallArg(ASTFunctionCall *Call, ASTArg *Arg, ASTExpr *Expr) {
    if (Expr) {
        Arg->Index = Call->getArgs().empty() ? 0 : Call->getArgs().size();
        Arg->Call = Call;
        Arg->Expr = Expr;
        Expr->Stmt = Arg;
        Call->Args.push_back(Arg);
        return Arg;
    }
    return false;
}

/**
 * Add ASTIfBlock
 * @param Loc
 * @param Expr
 * @return
 */
bool SemaBuilder::AddIfBlock(ASTBlock *Parent, ASTIfBlock *IfBlock) {
    // The UndefVars are copied from Parent to this if block
    IfBlock->Parent = Parent;
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
    ElsifBlock->Parent = IfBlock;
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
    ElseBlock->Parent = IfBlock;
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
    SwitchBlock->Parent = Parent;
    Parent->Content.push_back(SwitchBlock);
    return SwitchBlock;
}

bool SemaBuilder::AddSwitchCaseBlock(ASTSwitchBlock * SwitchBlock, ASTSwitchCaseBlock * CaseBlock) {
    CaseBlock->Parent = SwitchBlock;
    SwitchBlock->Cases.push_back(CaseBlock);
    return CaseBlock;
}

bool SemaBuilder::setSwitchDefaultBlock(ASTSwitchBlock * SwitchBlock, ASTSwitchDefaultBlock *DefaultBlock) {
    DefaultBlock->Parent = SwitchBlock;
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
    WhileBlock->Parent = Parent;
    Parent->Content.push_back(WhileBlock);
    return WhileBlock;
}

/**
 * Add ASTForBlock
 * @param Loc
 * @param Expr
 * @return
 */
bool SemaBuilder::AddForBlock(ASTBlock *Parent, ASTForBlock *ForBlock, ASTExpr *Condition) {
    ForBlock->Parent = Parent;
    Parent->Content.push_back(ForBlock);
    ForBlock->Condition = Condition;
    Condition->Stmt = ForBlock;
    return true;
}

/**
 *
 * @return
 */
bool SemaBuilder::OnCloseBlock(ASTBlock *Block) {
    return false;
}
