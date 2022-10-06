//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/Sema.cpp - The Sema Builder
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaBuilder.h"
#include "Sema/Sema.h"
#include "Sema/SemaResolver.h"
#include "Sema/SemaValidator.h"
#include "CodeGen/CodeGen.h"
#include "AST/ASTContext.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTNode.h"
#include "AST/ASTImport.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTFunctionCall.h"
#include "AST/ASTParams.h"
#include "AST/ASTBlock.h"
#include "AST/ASTIfBlock.h"
#include "AST/ASTWhileBlock.h"
#include "AST/ASTForBlock.h"
#include "AST/ASTSwitchBlock.h"
#include "AST/ASTVar.h"
#include "AST/ASTVarAssign.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTValue.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassVar.h"
#include "AST/ASTClassFunction.h"
#include "Basic/SourceLocation.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"

#include "llvm/ADT/StringMap.h"

using namespace fly;

/**
 * Private constructor used from static Build()
 * @param S
 */
SemaBuilder::SemaBuilder(Sema &S) : S(S), Context(new ASTContext()) {
    FLY_DEBUG("SemaBuilder", "SemaBuilder");
    Context->DefaultNameSpace = AddNameSpace(ASTNameSpace::DEFAULT);
}

/**
 * Build the SemaBuilder for AST creation statically
 * @return
 */
bool SemaBuilder::Build() {
    return S.Resolver->Resolve();
}

void SemaBuilder::Destroy() {
    delete Context;
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
    ASTNode *Node = new ASTNode(Name, Context, false);

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
    return new ASTNode(Name, Context, true);
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

ASTTopScopes *SemaBuilder::CreateTopScopes(ASTVisibilityKind Visibility, bool Constant) {
    return new ASTTopScopes(Visibility, Constant);
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
ASTGlobalVar *SemaBuilder::CreateGlobalVar(ASTNode *Node, const SourceLocation &Loc, ASTType *Type, const std::string &Name,
                                           ASTTopScopes *Scopes) {
    return new ASTGlobalVar(Loc, Node, Type, Name, Scopes);
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
ASTFunction *SemaBuilder::CreateFunction(ASTNode *Node, const SourceLocation &Loc, ASTType *Type,
                                         const std::string &Name, ASTTopScopes *Scopes) {
    ASTFunction *F = new ASTFunction(Loc, Node, Type, Name, Scopes);
    F->Params = new ASTParams();
    F->Body = CreateBlock(nullptr, SourceLocation());
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
ASTClass *SemaBuilder::CreateClass(ASTNode *Node, const SourceLocation &Loc, const std::string &Name,
                                   ASTTopScopes *Scopes) {
    return new ASTClass(Loc, Node, Name, Scopes);
}

ASTClassScopes *SemaBuilder::CreateClassScopes(ASTClassVisibilityKind Visibility, bool Constant) {
    return new ASTClassScopes(Visibility, Constant);
}

ASTClassVar *SemaBuilder::CreateClassVar(ASTClass *Class, SourceLocation &Loc, ASTType *Type, std::string Name,
                                         ASTClassScopes *Scopes) {
    ASTClassVar *ClassVar = new ASTClassVar(Loc, Class, Scopes, Type, Name);
    if (Class->Vars.insert(std::pair<std::string, ASTClassVar *>(Name, ClassVar)).second) {
        return ClassVar;
    }

    S.Diag(Loc, diag::err_sema_class_field_redeclare);
    return nullptr;
}

ASTBoolType *SemaBuilder::CreateBoolType(const SourceLocation &Loc) {
    return new ASTBoolType(Loc);
}

ASTByteType *SemaBuilder::CreateByteType(const SourceLocation &Loc) {
    return new ASTByteType(Loc);
}

ASTUShortType *SemaBuilder::CreateUShortType(const SourceLocation &Loc) {
    return new ASTUShortType(Loc);
}

ASTShortType *SemaBuilder::CreateShortType(const SourceLocation &Loc) {
    return new ASTShortType(Loc);;
}

ASTUIntType *SemaBuilder::CreateUIntType(const SourceLocation &Loc) {
    return new ASTUIntType(Loc);
}

ASTIntType *SemaBuilder::CreateIntType(const SourceLocation &Loc) {
    return new ASTIntType(Loc);
}

ASTULongType *SemaBuilder::CreateULongType(const SourceLocation &Loc) {
    return new ASTULongType(Loc);
}

ASTLongType *SemaBuilder::CreateLongType(const SourceLocation &Loc) {
    return new ASTLongType(Loc);
}

ASTFloatType *SemaBuilder::CreateFloatType(const SourceLocation &Loc) {
    return new ASTFloatType(Loc);
}

ASTDoubleType *SemaBuilder::CreateDoubleType(const SourceLocation &Loc) {
    return new ASTDoubleType(Loc);
}

ASTVoidType *SemaBuilder::CreateVoidType(const SourceLocation &Loc) {
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

ASTFloatingValue *SemaBuilder::CreateFloatingValue(const SourceLocation &Loc, std::string Val) {
    return new ASTFloatingValue(Loc, Val);
}

ASTFloatingValue *SemaBuilder::CreateFloatingValue(const SourceLocation &Loc, double Val) {
    std::string StrVal = std::to_string(Val);
    return new ASTFloatingValue(Loc, StrVal);
}

ASTArrayValue *SemaBuilder::CreateArrayValue(const SourceLocation &Loc) {
    return new ASTArrayValue(Loc);
}

ASTValue *SemaBuilder::CreateDefaultValue(ASTType *Type) {
    ASTValue *Value;
    if (Type->isBool()) {
        Value = CreateBoolValue(Type->getLocation(), false);
    } else if (Type->isInteger()) {
        Value = CreateIntegerValue(Type->getLocation(), 0);
    } else if (Type->isFloatingPoint()) {
        Value = CreateFloatingValue(Type->getLocation(), 0.0);
    }else if (Type->isArray()) {
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

ASTFunctionCall *SemaBuilder::CreateFunctionCall(ASTStmt *Stmt, ASTFunction *Function) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateDefFunctionCall",
                      "Stmt=" << Stmt << ", Function=" << Function);
    ASTFunctionCall *Call = new ASTFunctionCall(Stmt->Location, Function->NameSpace->Name, Function->Name);
    Call->Stmt = Stmt;
    Call->Def = Function;
    return Call;
}

ASTArg *SemaBuilder::CreateArg(ASTFunctionCall *Call, const SourceLocation &Loc) {
    ASTArg *Arg = new ASTArg(Call->Stmt, Loc);
    return Arg;
}

ASTParam *SemaBuilder::CreateParam(ASTFunction *Function, const SourceLocation &Loc, ASTType *Type, const std::string &Name, bool Constant) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateParam",
                      "Type=" << Type->str() << ", Name=" << Name << ", Constant=" << Constant);
    ASTParam *Param = new ASTParam(Function, Loc, Type, Name, Constant);
    return Param;
}

ASTLocalVar *SemaBuilder::CreateLocalVar(ASTBlock *Parent, const SourceLocation &Loc, ASTType *Type,
                                         const std::string &Name, bool Constant) {
    ASTLocalVar *LocalVar = new ASTLocalVar(Parent, Loc, Type, Name, Constant);
    if (Type->getKind() == TypeKind::TYPE_ARRAY) {
        LocalVar->Expr = CreateExpr(LocalVar, CreateArrayValue(Loc));
    }

    return LocalVar;
}

ASTVarAssign *SemaBuilder::CreateVarAssign(ASTBlock *Parent, ASTVarRef *VarRef) {
    ASTVarAssign *VarAssign = new ASTVarAssign(Parent, VarRef->getLocation(), VarRef);
    return VarAssign;
}

ASTReturn *SemaBuilder::CreateReturn(ASTBlock *Parent, const SourceLocation &Loc) {
    ASTReturn *Return = new ASTReturn(Parent, Loc);
    return Return;
}

ASTBreak *SemaBuilder::CreateBreak(ASTBlock *Parent, const SourceLocation &Loc) {
    ASTBreak *Break = new ASTBreak(Parent, Loc);
    return Break;
}

ASTContinue *SemaBuilder::CreateContinue(ASTBlock *Parent, const SourceLocation &Loc) {
    ASTContinue *Continue = new ASTContinue(Parent, Loc);
    return Continue;
}

ASTExprStmt *SemaBuilder::CreateExprStmt(ASTBlock *Parent, const SourceLocation &Loc) {
    ASTExprStmt *ExprStmt = new ASTExprStmt(Parent, Loc);
    return ExprStmt;
}

ASTVarRef *SemaBuilder::CreateVarRef(const SourceLocation &Loc, StringRef Name, StringRef NameSpace) {
    ASTVarRef *VarRef = CreateVarRef(Loc, Name, "", NameSpace);
    return VarRef;
}

ASTVarRef *SemaBuilder::CreateVarRef(const SourceLocation &Loc, StringRef Name, StringRef Class, StringRef NameSpace) {
    ASTVarRef *VarRef = new ASTVarRef(Loc, std::string(Name), std::string(Class), std::string(NameSpace));
    return VarRef;
}

ASTVarRef *SemaBuilder::CreateVarRef(ASTLocalVar *LocalVar) {
    assert(LocalVar->Top && "Var without a Top declaration");

    ASTNameSpace *NameSpace = S.FindNameSpace(LocalVar->Top);
    std::string Class = "";
    if (LocalVar->Top->Kind == ASTFunctionKind::CLASS_FUNCTION) {
        Class = ((ASTClassFunction *) LocalVar->Top)->getClass()->Name;
    }
    ASTVarRef *VarRef = CreateVarRef(LocalVar->Location, LocalVar->Name, Class, NameSpace->getName());

    VarRef->Def = LocalVar;
    return VarRef;
}

ASTVarRef *SemaBuilder::CreateVarRef(ASTGlobalVar *GlobalVar) {
    ASTVarRef *VarRef = CreateVarRef(GlobalVar->Location, GlobalVar->Name, GlobalVar->NameSpace->getName());
    VarRef->Def = GlobalVar;
    return VarRef;
}

ASTVarRef *SemaBuilder::CreateVarRef(ASTClassVar *ClassVar) {
    ASTVarRef *VarRef = CreateVarRef(ClassVar->getLocation(), ClassVar->Name, ClassVar->Class->Name, ClassVar->Class->NameSpace->getName());
    VarRef->Def = ClassVar;
    return VarRef;
}

ASTEmptyExpr *SemaBuilder::CreateExpr(ASTStmt *Stmt) {
    ASTEmptyExpr *Expr = new ASTEmptyExpr(SourceLocation());
    AddExpr(Stmt, Expr);
    return Expr;
}

ASTValueExpr *SemaBuilder::CreateExpr(ASTStmt *Stmt, ASTValue *Value) {
    ASTValueExpr *ValueExpr = new ASTValueExpr(Value);
    ValueExpr->Stmt = Stmt; // TODO add ASTExpr() constructor
    AddExpr(Stmt, ValueExpr);
    return ValueExpr;
}

ASTFunctionCallExpr *SemaBuilder::CreateExpr(ASTStmt *Stmt, ASTFunctionCall *Call) {
    ASTFunctionCallExpr *FunctionCallExpr = new ASTFunctionCallExpr(Call);
    FunctionCallExpr->Stmt = Stmt;
    AddExpr(Stmt, FunctionCallExpr);
    return FunctionCallExpr;
}

ASTVarRefExpr *SemaBuilder::CreateExpr(ASTStmt *Stmt, ASTVarRef *VarRef) {
    ASTVarRefExpr *VarRefExpr = new ASTVarRefExpr(VarRef);
    VarRefExpr->Stmt = Stmt;
    AddExpr(Stmt, VarRefExpr);
    return VarRefExpr;
}

ASTUnaryGroupExpr *SemaBuilder::CreateUnaryExpr(ASTStmt *Stmt, const SourceLocation &Loc, UnaryOpKind Kind,
                                                UnaryOptionKind OptionKind, ASTVarRefExpr *First) {
    ASTUnaryGroupExpr *UnaryExpr = new ASTUnaryGroupExpr(Loc, Kind, OptionKind, First);
    UnaryExpr->Stmt = Stmt;

    // Set Parent Expression
    First->Parent = UnaryExpr;

    AddExpr(Stmt, UnaryExpr);
    return UnaryExpr;
}

ASTBinaryGroupExpr *SemaBuilder::CreateBinaryExpr(ASTStmt *Stmt, const SourceLocation &OpLoc, BinaryOpKind Kind,
                                                  ASTExpr *First, ASTExpr *Second) {
    ASTBinaryGroupExpr *BinaryExpr = new ASTBinaryGroupExpr(OpLoc, Kind, First, Second);
    BinaryExpr->Stmt = Stmt;

    // Set Parent Expression
    First->Parent = BinaryExpr;
    Second->Parent = BinaryExpr;

    AddExpr(Stmt, BinaryExpr);
    return BinaryExpr;
}

ASTTernaryGroupExpr *SemaBuilder::CreateTernaryExpr(ASTStmt *Stmt, ASTExpr *First, const SourceLocation &IfLoc,
                                                    ASTExpr *Second, const SourceLocation &ElseLoc, ASTExpr *Third) {
    ASTTernaryGroupExpr *TernaryExpr = new ASTTernaryGroupExpr(First, IfLoc, Second, ElseLoc, Third);
    TernaryExpr->Stmt = Stmt;

    // Set Parent Expression
    First->Parent = TernaryExpr;
    Second->Parent = TernaryExpr;
    Third->Parent = TernaryExpr;

    AddExpr(Stmt, TernaryExpr);
    return TernaryExpr;
}

ASTBlock* SemaBuilder::CreateBlock(ASTBlock *Parent, const SourceLocation &Loc) {
    ASTBlock *Block = new ASTBlock(Parent, Loc);
    return Block;
}

ASTBlock* SemaBuilder::getBlock(ASTFunction *Function) {
    return Function->Body;
}

ASTIfBlock *SemaBuilder::CreateIfBlock(ASTBlock *Parent, const SourceLocation &Loc) {
    ASTIfBlock *Block = new ASTIfBlock(Parent, Loc);
    return Block;
}

ASTElsifBlock *SemaBuilder::CreateElsifBlock(ASTIfBlock *IfBlock, const SourceLocation &Loc) {
    ASTElsifBlock *Block = new ASTElsifBlock(IfBlock, Loc);
    return Block;
}

ASTElseBlock *SemaBuilder::CreateElseBlock(ASTIfBlock *IfBlock, const SourceLocation &Loc) {
    ASTElseBlock *Block = new ASTElseBlock(IfBlock, Loc);
    return Block;
}

ASTSwitchBlock *SemaBuilder::CreateSwitchBlock(ASTBlock *Parent, const SourceLocation &Loc) {
    ASTSwitchBlock *Block = new ASTSwitchBlock(Parent, Loc);
    return Block;
}

ASTSwitchCaseBlock *SemaBuilder::CreateSwitchCaseBlock(ASTSwitchBlock *SwitchBlock, const SourceLocation &Loc) {
    ASTSwitchCaseBlock *Block = new ASTSwitchCaseBlock(SwitchBlock, Loc);
    return Block;
}

ASTSwitchDefaultBlock *SemaBuilder::CreateSwitchDefaultBlock(ASTSwitchBlock *SwitchBlock, const SourceLocation &Loc) {
    ASTSwitchDefaultBlock *Block = new ASTSwitchDefaultBlock(SwitchBlock, Loc);
    return Block;
}

ASTWhileBlock *SemaBuilder::CreateWhileBlock(ASTBlock *Parent, const SourceLocation &Loc) {
    ASTWhileBlock *Block = new ASTWhileBlock(Parent, Loc);
    return Block;
}

/**
 * Create an ASTForBlock
 * @param Loc
 * @param Condition
 * @param PostBlock
 * @param LoopBlock
 * @return
 */
ASTForBlock *SemaBuilder::CreateForBlock(ASTBlock *Parent, const SourceLocation &Loc) {
    ASTForBlock *Block = new ASTForBlock(Parent, Loc);
    return Block;
}

ASTForLoopBlock *SemaBuilder::CreateForLoopBlock(ASTForBlock *ForBlock, const SourceLocation &Loc) {
    ASTForLoopBlock *Block = new ASTForLoopBlock(ForBlock, Loc);
    return Block;
}

ASTForPostBlock *SemaBuilder::CreateForPostBlock(ASTForBlock *ForBlock, const SourceLocation &Loc) {
    ASTForPostBlock *Block = new ASTForPostBlock(ForBlock, Loc);
    return Block;
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
    ASTNameSpace *NameSpace = Context->NameSpaces.lookup(Name);
    if (NameSpace == nullptr) {
        NameSpace = new ASTNameSpace(Name, Context, ExternLib);
        Context->NameSpaces.insert(std::make_pair(Name, NameSpace));
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
    return Node->NameSpace->Nodes.insert(Pair).second && Context->Nodes.insert(Pair).second;
}

bool SemaBuilder::AddImport(ASTNode *Node, ASTImport * Import) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddImport", "Node=" << Node->str() << ", Import=" << Import->str());

    if (S.Validator->CheckImport(Node, Import)) {
        std::string Name = Import->getAlias().empty() ? Import->getName() : Import->getAlias();

        // Check if this Node already own this Import
        if (Node->Imports.lookup(Name)) {
            S.Diag(Import->getNameLocation(), diag::err_conflict_import) << Name;
            return false;
        }

        // Add Import to Node
        auto Pair = std::make_pair(Name, Import);
        return Node->Imports.insert(Pair).second;
    }

    return false;
}

bool SemaBuilder::AddClass(ASTNode *Node, ASTClass *Class) {
    FLY_DEBUG_MESSAGE("ASTNode", "AddFunction",
                      "Node=" << Node->str() << ", Class" << Class->str());

    /**
    * CLASS_STRUCT has only Fields
    * CLASS_INTERFACE has only prototype Methods
    * CLASS_ABSTRACT has at least one prototype Method
    * CLASS_STANDARD has no prototype Methods
    */
    if (Class->Methods.empty()) {
     Class->ClassKind = ASTClassKind::CLASS_STRUCT;
    }

    // Lookup into namespace
    Node->Class = Class;

    bool Success = true;
    if (Class->Scopes->Visibility == ASTVisibilityKind::V_PUBLIC || Class->Scopes->Visibility == ASTVisibilityKind::V_DEFAULT) {
        ASTClass *LookupClass = Node->NameSpace->getClasses().lookup(Class->getName());
        if (LookupClass) { // This NameSpace already contains this Function
            S.Diag(LookupClass->Location, diag::err_duplicate_class) << LookupClass->getName();
            return false;
        }
        Success = Node->NameSpace->Classes.insert(std::make_pair(Class->getName(), Class)).second;
    }

    return Success;
}

bool SemaBuilder::AddGlobalVar(ASTNode *Node, ASTGlobalVar *GlobalVar, ASTValue *Value) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddGlobalVar",
                      "Node=" << Node->str() << ", GlobalVar=" << GlobalVar->str() << ", Value=" << Value->str());

    // Set the Expr with ASTValueExpr
    return AddGlobalVar(Node, GlobalVar, CreateExpr(nullptr, Value));
}

bool SemaBuilder::AddGlobalVar(ASTNode *Node, ASTGlobalVar *GlobalVar, ASTExpr *Expr) {

    // Only ASTExprValue
    if (Expr && Expr->getExprKind() != ASTExprKind::EXPR_VALUE) {
        S.Diag(Expr->getLocation(),diag::err_sema_generic);
        return false;
    }

    GlobalVar->Expr = Expr;

    // Lookup into namespace for public var
    if(GlobalVar->Scopes->Visibility == ASTVisibilityKind::V_PUBLIC ||
        GlobalVar->Scopes->Visibility == ASTVisibilityKind::V_DEFAULT) {
        ASTGlobalVar *LookupVar = Node->NameSpace->getGlobalVars().lookup(GlobalVar->getName());

        if (LookupVar) { // This NameSpace already contains this GlobalVar
            S.Diag(LookupVar->getLocation(), diag::err_duplicate_gvar) << LookupVar->getName();
            return false;
        }

        // Add into NameSpace for global resolution
        // Add into Node for local resolution
        auto Pair = std::make_pair(GlobalVar->getName(), GlobalVar);
        // TODO check duplicate in namespace and node
        return Node->GlobalVars.insert(Pair).second && Node->NameSpace->GlobalVars.insert(Pair).second;
    }

    // Lookup into node for private var
    if(GlobalVar->Scopes->Visibility == ASTVisibilityKind::V_PRIVATE) {
        ASTGlobalVar *LookupVar = Node->GlobalVars.lookup(GlobalVar->getName());

        if (LookupVar) { // This Node already contains this Function
            S.Diag(LookupVar->getLocation(), diag::err_duplicate_gvar) << LookupVar->getName();
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
    if(Function->Scopes->Visibility == ASTVisibilityKind::V_PUBLIC ||
        Function->Scopes->Visibility == ASTVisibilityKind::V_DEFAULT) {

        // Add into NameSpace for global resolution
        // Add into Node for local resolution
        return InsertFunction(Node->NameSpace->Functions, Function) && InsertFunction(Node->Functions, Function);
    }

    // Lookup into node for private var
    if (Function->Scopes->Visibility == ASTVisibilityKind::V_PRIVATE) {

        // Add into Node for local resolution
        return InsertFunction(Node->Functions, Function);
    }

    assert(0 && "Unknown Function Visibility");
}


bool SemaBuilder::InsertFunction(llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> &Functions,
                                 ASTFunction *Function) {
    // Functions is llvm::StringMap<std::map <uint64_t, llvm::SmallVector <ASTFunction *, 4>>>
    const auto &StrMapIt = Functions.find(Function->getName());

    // This Node not contains a Function with this Function->Name
    if (StrMapIt == Functions.end()) {

        // add to llvm::SmallVector
        llvm::SmallVector<ASTFunction *, 4> Vect;
        Vect.push_back(Function);

        // add to std::map
        std::map<uint64_t, llvm::SmallVector<ASTFunction *, 4>> IntMap;
        IntMap.insert(std::make_pair(Function->Params->getSize(),Vect));

        // add to llvm::StringMap
        return Functions.insert(std::make_pair(Function->getName(), IntMap)).second;
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
                        if (!S.Validator->isEquals(NodeFuncParam, Param)) {
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
                S.Diag(Function->getLocation(), diag::err_duplicate_func) << Function->getName();
                return false;
            }
        }
    }
}

bool SemaBuilder::AddParam(ASTParam *Param) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddExternalFunction",
                      "Param=" << Param->str());
    // TODO Check duplicate
    Param->Top->Params->List.push_back(Param);
    Param->Parent = Param->Top->Body;

    Param->Top->LocalVars.push_back(Param); //Useful for Alloca into CodeGen
    return Param->Top->Body->LocalVars
        .insert(std::pair<std::string, ASTLocalVar *>(Param->getName(), Param)).second;
}

void SemaBuilder::AddFunctionVarParams(ASTFunction *Function, ASTParam *Param) {
    Function->Params->Ellipsis = Param;
}

std::string getComment(std::string &C) {
    if (C.empty()) {
        return C;
    }
    const char *t = " \t\n\r\f\v";
    C = C.substr(2, C.size() - 4);
    C = C.erase(0, C.find_first_not_of(t)); // TODO Check
    return C.erase(C.find_last_not_of(t) + 1);
}

bool SemaBuilder::AddComment(ASTTopDef *Top, std::string &Comment) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddComment", "Top=" << Top->str() << ", Comment=" << Comment);
    Top->Comment = getComment(Comment);
    return true;
}

bool SemaBuilder::AddComment(ASTClassVar *Field, std::string &Comment) {
    Field->Comment = getComment(Comment);
    return true;
}

bool SemaBuilder::AddComment(ASTClassFunction *Method, std::string &Comment) {
    Method->Comment = getComment(Comment);
    return true;
}

bool SemaBuilder::AddExternalGlobalVar(ASTNode *Node, ASTGlobalVar *GlobalVar) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddExternalGlobalVar",
                      "Node=" << Node->str() << ", GlobalVar=" << GlobalVar->str());
    // TODO Check duplicate
    return Node->ExternalGlobalVars.insert(std::make_pair(GlobalVar->getName(), GlobalVar)).second;
}

bool SemaBuilder::AddExternalFunction(ASTNode *Node, ASTFunction *Function) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddExternalFunction",
                      "Node=" << Node->str() << ", Function=" << Function->str());
    // TODO Check duplicate
    return InsertFunction(Node->ExternalFunctions, Function);
}

bool SemaBuilder::AddArrayValue(ASTArrayValue *Array, ASTValue *Value) {
    Array->Values.push_back(Value);
    return true;
}

bool SemaBuilder::AddFunctionCallArg(ASTFunctionCall *Call, ASTArg *Arg) {
    Arg->Index = Call->getArgs().empty() ? 0 : Call->getArgs().size();
    Arg->Call = Call;
    Call->Args.push_back(Arg);
    return true;
}

bool SemaBuilder::AddExpr(ASTStmt *Stmt, ASTExpr *Expr) {
    if (!Expr) {
        return S.Diag(Expr->getLocation(), diag::err_sema_generic) && false;
    }

    FLY_DEBUG_MESSAGE("SemaBuilder", "AddExpr", "Expr=" << Expr->str());
    if (Stmt)
        switch (Stmt->getKind()) {
            case StmtKind::STMT_EXPR:
            case StmtKind::STMT_ARG:
            case StmtKind::STMT_VAR_DEFINE:
            case StmtKind::STMT_VAR_ASSIGN:
            case StmtKind::STMT_RETURN:
                ((ASTExprStmt *) Stmt)->Expr = Expr;
                break;
            case StmtKind::STMT_BLOCK:
                switch (((ASTBlock *) Stmt)->getBlockKind()) {
                    case ASTBlockKind::BLOCK_IF:
                        ((ASTIfBlock *) Stmt)->Condition = Expr;
                        break;
                    case ASTBlockKind::BLOCK_ELSIF:
                        ((ASTElsifBlock *) Stmt)->Condition = Expr;
                        break;
                    case ASTBlockKind::BLOCK_SWITCH:
                        ((ASTSwitchBlock *) Stmt)->Expr = Expr;
                        break;
                    case ASTBlockKind::BLOCK_SWITCH_CASE:
                        ((ASTSwitchCaseBlock *) Stmt)->Expr = Expr;
                        break;
                    case ASTBlockKind::BLOCK_WHILE:
                        ((ASTWhileBlock *) Stmt)->Condition = Expr;
                        break;
                    case ASTBlockKind::BLOCK_FOR:
                        ((ASTForBlock *) Stmt)->Condition = Expr;
                        break;
                    case ASTBlockKind::BLOCK:
                    case ASTBlockKind::BLOCK_ELSE:
                    case ASTBlockKind::BLOCK_SWITCH_DEFAULT:
                        assert("Cannot contain an Expr");
                        break;
                }
            case StmtKind::STMT_BREAK:
            case StmtKind::STMT_CONTINUE:
                assert("Cannot contain an Expr");
        }

    return true;
}

/**
 * Add ExprStmt to Content
 * @param ExprStmt
 * @return true if no error occurs, otherwise false
 */
bool SemaBuilder::AddStmt(ASTStmt *Stmt) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddStmt", "Stmt=" << Stmt->str());

    // Stmt->Parent = Block; // Already done on create

    ASTBlock *Parent = (ASTBlock *) Stmt->Parent;
    Parent->Content.push_back(Stmt);

    if (Stmt->getKind() == StmtKind::STMT_VAR_DEFINE) { // Stmt is ASTLocalVar

        ASTLocalVar *LocalVar = (ASTLocalVar *) Stmt;

        // Check Undefined Var: if LocalVar have an Expression assigned
        if (!LocalVar->Expr) {  // No Expression: add to Undefined Vars, will be removed on SemaResolver::ResolveVarRef()
            Parent->UndefVars.insert(std::pair<std::string, ASTLocalVar *>(LocalVar->getName(), LocalVar));
        }

        // Collects all LocalVars in the hierarchy Block
        if (Parent->LocalVars.insert(std::pair<std::string, ASTLocalVar *>(LocalVar->getName(), LocalVar)).second) {

            //Useful for Alloca into CodeGen
            Parent->Top->LocalVars.push_back(LocalVar);
            return true;
        }
    } else if (Stmt->getKind() == StmtKind::STMT_VAR_ASSIGN) {
        ASTVarAssign *VarAssign = (ASTVarAssign *) Stmt;

        // Remove from Undefined Var because now have an Expr assigned
        if (VarAssign->getVarRef()->getNameSpace().empty()) { // only for VarRef with empty NameSpace
            auto It = Parent->UndefVars.find(VarAssign->getVarRef()->getName());
            if (It != Parent->UndefVars.end())
                Parent->UndefVars.erase(It);
        }
    } else if (Stmt->getKind() == StmtKind::STMT_BLOCK) {
        return AddBlock((ASTBlock *) Stmt);
    }

    return true;
}

bool SemaBuilder::AddBlock(ASTBlock *Block) {
    switch (Block->getBlockKind()) {

        case ASTBlockKind::BLOCK:
            return true;

        case ASTBlockKind::BLOCK_IF:
            Block->UndefVars = ((ASTIfBlock *) Block->getParent())->UndefVars;
            return true;

        case ASTBlockKind::BLOCK_ELSIF: {
            ASTElsifBlock *ElsifBlock = (ASTElsifBlock *) Block;
            if (!ElsifBlock->IfBlock) {
                S.Diag(ElsifBlock->getLocation(), diag::err_missing_if_first);
                return false;
            }
            if (ElsifBlock->IfBlock->ElseBlock) {
                S.Diag(ElsifBlock->getLocation(), diag::err_elseif_after_else);
                return false;
            }
            ElsifBlock->IfBlock->ElsifBlocks.push_back(ElsifBlock);
            return !ElsifBlock->IfBlock->ElsifBlocks.empty();
        }

        case ASTBlockKind::BLOCK_ELSE: {
            ASTElseBlock *ElseBlock = (ASTElseBlock *) Block;
            if (!ElseBlock->IfBlock) {
                S.Diag(ElseBlock->getLocation(), diag::err_missing_if_first);
                return false;
            }
            if (ElseBlock->IfBlock->ElseBlock) {
                S.Diag(ElseBlock->getLocation(), diag::err_else_after_else);
                return false;
            }
            ElseBlock->IfBlock->ElseBlock = ElseBlock;
            return true;
        }

        case ASTBlockKind::BLOCK_SWITCH: {
            ASTSwitchBlock *SwitchBlock = (ASTSwitchBlock *) Block;

            return true;
        }

        case ASTBlockKind::BLOCK_WHILE: {
            ASTWhileBlock *While = (ASTWhileBlock *) Block;

            return true;
        }

        case ASTBlockKind::BLOCK_FOR: {
            ASTForBlock *For = (ASTForBlock *) Block;

            return true;
        }
    }
}


