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
 * Private constructor used only from Sema constructor
 * @param S
 */
SemaBuilder::SemaBuilder(Sema &S) : S(S), Context(new ASTContext()) {
    FLY_DEBUG("SemaBuilder", "SemaBuilder");
    Context->DefaultNameSpace = AddNameSpace(ASTNameSpace::DEFAULT);
}

/**
 * Builds the SemaBuilder Instance
 * @return
 */
bool
SemaBuilder::Build() {
    FLY_DEBUG("SemaBuilder", "Build");
    return S.Resolver->Resolve();
}

/**
 * Destroys SemaBuilder Instance
 */
void
SemaBuilder::Destroy() {
    FLY_DEBUG("SemaBuilder", "Destroy");
    delete Context;
}

/**
 * Creates an ASTNode
 * If NameSpace doesn't exists it will be created
 * @param Name
 * @param NameSpace
 * @return the ASTNode
 */
ASTNode *
SemaBuilder::CreateNode(const std::string &Name, std::string &NameSpace) {
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
 * Creates an ASTHeaderNode: only prototype declarations without definitions
 * For .fly.h file generation
 * @param Name
 * @param NameSpace
 * @return thee ASTHeaderNode
 */
ASTNode *
SemaBuilder::CreateHeaderNode(const std::string &Name, std::string &NameSpace) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateHeaderNode", "Name=" << Name << ", NameSpace=" << NameSpace);
    return new ASTNode(Name, Context, true);
}

ASTImport *
SemaBuilder::CreateImport(const SourceLocation &NameLoc, llvm::StringRef Name) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateImport",
                      "Loc=" << NameLoc.getRawEncoding() <<
                      ", Name=" << Name);
    std::string NameStr = Name.str();
    return new ASTImport(NameLoc, NameStr);
}

ASTImport *
SemaBuilder::CreateImport(const SourceLocation &NameLoc, llvm::StringRef Name,
                                     const SourceLocation &AliasLoc, llvm::StringRef Alias) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateImport",
                      "NameLoc=" << NameLoc.getRawEncoding() <<
                      ", Name=" << Name <<
                      ", AliasLoc=" << NameLoc.getRawEncoding() <<
                      ", Alias=" << Alias);
    std::string NameStr = Name.str();
    std::string AliasStr = Alias.str();
    return new ASTImport(NameLoc, NameStr, AliasLoc, AliasStr);
}

/**
 * Creates scopes used from ASTTopDef
 * @param Visibility
 * @param Constant
 * @return
 */
ASTTopScopes *
SemaBuilder::CreateTopScopes(ASTVisibilityKind Visibility, bool Constant) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateTopScopes",
                      "Visibility=" << (int)Visibility <<
                      ", Constant=" << (int)Constant);
    return new ASTTopScopes(Visibility, Constant);
}

/**
 * Creates an ASTGlobalVar
 * @param Node
 * @param Loc
 * @param Type
 * @param Name
 * @param Scopes
 * @return
 */
ASTGlobalVar *
SemaBuilder::CreateGlobalVar(ASTNode *Node, const SourceLocation &Loc, ASTType *Type,
                                           const std::string &Name, ASTTopScopes *Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateGlobalVar",
                      Logger().Attr("Node", Node)
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                      .Attr("Type", Type)
                      .Attr("Name", Name)
                      .Attr("Scopes", Scopes).End());
    return new ASTGlobalVar(Loc, Node, Type, Name, Scopes);
}

/**
 * Creates an ASTFunction
 * @param Node
 * @param Loc
 * @param Type
 * @param Name
 * @param Scopes
 * @return
 */
ASTFunction *
SemaBuilder::CreateFunction(ASTNode *Node, const SourceLocation &Loc, ASTType *Type,
                                         const std::string &Name, ASTTopScopes *Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateFunction",
                      Logger().Attr("Node", Node)
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                      .Attr("Type", Type)
                      .Attr("Name", Name)
                      .Attr("Scopes", Scopes).End());
    ASTFunction *F = new ASTFunction(Loc, Node, Type, Name, Scopes);
    F->Params = new ASTParams();
    F->Body = CreateBlock(nullptr, SourceLocation());
    F->Body->Top = F;
    return F;
}

/**
 * Creates an ASTClass
 * @param Node
 * @param Loc
 * @param Name
 * @param Scopes
 * @return
 */
ASTClass *
SemaBuilder::CreateClass(ASTNode *Node, const SourceLocation &Loc, const std::string &Name,
                                   ASTTopScopes *Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClass",
                      Logger().Attr("Node", Node)
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                      .Attr("Name", Name)
                      .Attr("Scopes", Scopes).End());
    return new ASTClass(Loc, Node, Name, Scopes);
}

/**
 * Creates a Scope for Classes
 * @param Visibility
 * @param Constant
 * @return
 */
ASTClassScopes *
SemaBuilder::CreateClassScopes(ASTClassVisibilityKind Visibility, bool Constant) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassScopes",
                      "Visibility=" << (int) Visibility <<
                              ", Constant=" << Constant);
    return new ASTClassScopes(Visibility, Constant);
}

/**
 * Creates an ASTClassVar
 * @param Class
 * @param Loc
 * @param Type
 * @param Name
 * @param Scopes
 * @return
 */
ASTClassVar *
SemaBuilder::CreateClassVar(ASTClass *Class, SourceLocation &Loc, ASTType *Type, std::string Name,
                                         ASTClassScopes *Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassVar",
                      Logger().Attr("Class", Class)
                                            .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                                            .Attr("Type=", Type)
                                            .Attr("Name", Name)
                                            .Attr("Scopes", Scopes).End());
    ASTClassVar *ClassVar = new ASTClassVar(Loc, Class, Scopes, Type, Name);
    if (Class->Vars.insert(std::pair<std::string, ASTClassVar *>(Name, ClassVar)).second) {
        return ClassVar;
    }

    S.Diag(Loc, diag::err_sema_class_field_redeclare);
    return nullptr;
}

/**
 * Creates a bool type
 * @param Loc
 * @return
 */
ASTBoolType *
SemaBuilder::CreateBoolType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateBoolType", "Loc=" << Loc.getRawEncoding());
    return new ASTBoolType(Loc);
}

/**
 * Creates an byte type
 * @param Loc
 * @return
 */
ASTByteType *
SemaBuilder::CreateByteType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateByteType", "Loc=" << Loc.getRawEncoding());
    return new ASTByteType(Loc);
}

/**
 * Creates an unsigned short type
 * @param Loc
 * @return
 */
ASTUShortType *
SemaBuilder::CreateUShortType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateUShortType", "Loc=" << Loc.getRawEncoding());
    return new ASTUShortType(Loc);
}

/**
 * Create a short type
 * @param Loc
 * @return
 */
ASTShortType *
SemaBuilder::CreateShortType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateShortType", "Loc=" << Loc.getRawEncoding());
    return new ASTShortType(Loc);;
}

/**
 * Creates an unsigned int type
 * @param Loc
 * @return
 */
ASTUIntType *
SemaBuilder::CreateUIntType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateUIntType", "Loc=" << Loc.getRawEncoding());
    return new ASTUIntType(Loc);
}

/**
 * Creates an int type
 * @param Loc
 * @return
 */
ASTIntType *
SemaBuilder::CreateIntType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateIntType", "Loc=" << Loc.getRawEncoding());
    return new ASTIntType(Loc);
}

/**
 * Creates an unsigned long type
 * @param Loc
 * @return
 */
ASTULongType *
SemaBuilder::CreateULongType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateULongType", "Loc=" << Loc.getRawEncoding());
    return new ASTULongType(Loc);
}

/**
 * Creates a long type
 * @param Loc
 * @return
 */
ASTLongType *
SemaBuilder::CreateLongType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLongType", "Loc=" << Loc.getRawEncoding());
    return new ASTLongType(Loc);
}

/**
 * Creates a float type
 * @param Loc
 * @return
 */
ASTFloatType *
SemaBuilder::CreateFloatType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateFloatType", "Loc=" << Loc.getRawEncoding());
    return new ASTFloatType(Loc);
}

/**
 * Creates a double type
 * @param Loc
 * @return
 */
ASTDoubleType *
SemaBuilder::CreateDoubleType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateDoubleType", "Loc=" << Loc.getRawEncoding());
    return new ASTDoubleType(Loc);
}

/**
 * Creates a void type
 * @param Loc
 * @return
 */
ASTVoidType *
SemaBuilder::CreateVoidType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateVoidType", "Loc=" << Loc.getRawEncoding());
    return new ASTVoidType(Loc);
}

/**
 * Creates an array type
 * @param Loc
 * @param Type
 * @param Size
 * @return
 */
ASTArrayType *
SemaBuilder::CreateArrayType(const SourceLocation &Loc, ASTType *Type, ASTExpr *Size) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateArrayType",
                      "Loc=" << Loc.getRawEncoding() <<
                      ", Size=" << Size);
    return new ASTArrayType(Loc, Type, Size);
}

/**
 * Creates a class type without definition
 * @param Loc
 * @param Name
 * @param NameSpace
 * @return
 */
ASTClassType *
SemaBuilder::CreateClassType(const SourceLocation &Loc, llvm::StringRef Name, llvm::StringRef NameSpace) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassType",
                      "Loc=" << Loc.getRawEncoding() <<
                      ", Name=" << Name <<
                      ", NameSPace=" << NameSpace);
    return new ASTClassType(Loc, Name.str(), NameSpace.str());
}

/**
 * Creates a class type with definition
 * @param Class
 * @return
 */
ASTClassType *
SemaBuilder::CreateClassType(ASTClass *Class) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassType", Logger().Attr("Class", Class).End());
    ASTClassType *ClassType = new ASTClassType(Class->Location, Class->Name, Class->NameSpace->Name);
    ClassType->Def = Class;
    return ClassType;
}

/**
 * Creates a null value
 * @param Loc
 * @return
 */
ASTNullValue *
SemaBuilder::CreateNullValue(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateNullValue", "Loc=" << Loc.getRawEncoding());
    return new ASTNullValue(Loc);
}

/**
 * Creates a bool value
 * @param Loc
 * @param Val
 * @return
 */
ASTBoolValue *
SemaBuilder::CreateBoolValue(const SourceLocation &Loc, bool Val) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateBoolValue",
                      "Loc=" << Loc.getRawEncoding() <<
                      ", Val=" << Val);
    return new ASTBoolValue(Loc, Val);
}

/**
 * Creates an integer value
 * @param Loc
 * @param Val
 * @param Negative
 * @return
 */
ASTIntegerValue *
SemaBuilder::CreateIntegerValue(const SourceLocation &Loc, uint64_t Val, bool Negative) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateIntegerValue",
                      "Loc=" << Loc.getRawEncoding() <<
                      ", Val=" << Val <<
                      ", Negative=" << Negative);
    return new ASTIntegerValue(Loc, Val, Negative);
}

/**
 * Creates a char value
 * @param Loc
 * @param Val
 * @return
 */
ASTIntegerValue *
SemaBuilder::CreateCharValue(const SourceLocation &Loc, char Val) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateCharValue",
                      "Loc=" << Loc.getRawEncoding() <<
                      ", Val=" << Val);
    return CreateIntegerValue(Loc, Val, false);
}

/**
 * Creates a floating point value
 * @param Loc
 * @param Val
 * @return
 */
ASTFloatingValue *
SemaBuilder::CreateFloatingValue(const SourceLocation &Loc, std::string Val) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateFloatingValue",
                      "Loc=" << Loc.getRawEncoding() <<
                      ", Val=" << Val);
    return new ASTFloatingValue(Loc, Val);
}

/**
 * Creates a floating point value
 * @param Loc
 * @param Val
 * @return
 */
ASTFloatingValue *
SemaBuilder::CreateFloatingValue(const SourceLocation &Loc, double Val) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateFloatingValue",
                      "Loc=" << Loc.getRawEncoding() <<
                      ", Val=" << Val);
    std::string StrVal = std::to_string(Val);
    return new ASTFloatingValue(Loc, StrVal);
}

/**
 * Create an array value
 * @param Loc
 * @return
 */
ASTArrayValue *
SemaBuilder::CreateArrayValue(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateArrayValue",
                      "Loc=" << Loc.getRawEncoding());
    return new ASTArrayValue(Loc);
}

/**
 * Creates a default value by type
 * @param Type
 * @return
 */
ASTValue *
SemaBuilder::CreateDefaultValue(ASTType *Type) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateDefaultValue",
                      Logger().Attr("Type", Type).End());
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
 * Create an ASTFunctionCall without definition
 * @param Location
 * @param Name
 * @param NameSpace
 * @return
 */
ASTFunctionCall *
SemaBuilder::CreateFunctionCall(ASTStmt *Stmt, const SourceLocation &Loc, std::string &Name, std::string &NameSpace) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateDefFunctionCall",
                      Logger().Attr("Stmt", Stmt)
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                      .Attr("Name", Name)
                      .Attr("NameSpace=", NameSpace).End());
    ASTFunctionCall *Call = new ASTFunctionCall(Loc, NameSpace, Name);
    Call->Stmt = Stmt;
    return Call;
}

/**
 * Creates an ASTFunctionCall with definition
 * @param Stmt
 * @param Function
 * @return
 */
ASTFunctionCall *
SemaBuilder::CreateFunctionCall(ASTStmt *Stmt, ASTFunction *Function) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateDefFunctionCall",
                      Logger().Attr("Stmt", Stmt)
                      .Attr("Function=", Function).End());
    ASTFunctionCall *Call = new ASTFunctionCall(Stmt->Location, Function->NameSpace->Name, Function->Name);
    Call->Stmt = Stmt;
    Call->Def = Function;
    return Call;
}

/**
 * Creates an ASTArg
 * @param Call
 * @param Loc
 * @return
 */
ASTArg *
SemaBuilder::CreateArg(ASTFunctionCall *Call, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateArg",
                      Logger().Attr("Call", Call).Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTArg *Arg = new ASTArg(Call->Stmt, Loc);
    return Arg;
}

/**
 * Creates an ASTParam
 * @param Function
 * @param Loc
 * @param Type
 * @param Name
 * @param Constant
 * @return
 */
ASTParam *
SemaBuilder::CreateParam(ASTFunction *Function, const SourceLocation &Loc, ASTType *Type, const std::string &Name, bool Constant) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateParam",
                      Logger().Attr("Function", Function)
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                      .Attr("Type", Type)
                      .Attr("Name", Name)
                      .Attr("Constant", Constant)
                      .End());
    ASTParam *Param = new ASTParam(Function, Loc, Type, Name, Constant);
    return Param;
}

/**
 * Creates an ASTLocalVar
 * @param Parent
 * @param Loc
 * @param Type
 * @param Name
 * @param Constant
 * @return
 */
ASTLocalVar *
SemaBuilder::CreateLocalVar(ASTBlock *Parent, const SourceLocation &Loc, ASTType *Type,
                                         const std::string &Name, bool Constant) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLocalVar",
                      Logger().Attr("Parent", Parent).End());
    ASTLocalVar *LocalVar = new ASTLocalVar(Parent, Loc, Type, Name, Constant);
    if (Type->getKind() == ASTTypeKind::TYPE_ARRAY) {
        LocalVar->Expr = CreateExpr(LocalVar, CreateArrayValue(Loc));
    }

    return LocalVar;
}

/**
 * Creates an ASTVarAssign
 * @param Parent
 * @param VarRef
 * @return
 */
ASTVarAssign *
SemaBuilder::CreateVarAssign(ASTBlock *Parent, ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLocalVar",
                      Logger().Attr("Parent", Parent).End());
    ASTVarAssign *VarAssign = new ASTVarAssign(Parent, VarRef->getLocation(), VarRef);
    return VarAssign;
}

/**
 * Creates an ASTReturn
 * @param Parent
 * @param Loc
 * @return
 */
ASTReturn *
SemaBuilder::CreateReturn(ASTBlock *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLocalVar", Logger()
                      .Attr("Parent", Parent)
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTReturn *Return = new ASTReturn(Parent, Loc);
    return Return;
}

ASTBreak *
SemaBuilder::CreateBreak(ASTBlock *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLocalVar", Logger()
                        .Attr("Parent", Parent)
                        .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTBreak *Break = new ASTBreak(Parent, Loc);
    return Break;
}

ASTContinue *
SemaBuilder::CreateContinue(ASTBlock *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLocalVar", Logger()
                      .Attr("Parent", Parent)
                      .Attr("Loc", (uint64_t)Loc.getRawEncoding()).End());
    ASTContinue *Continue = new ASTContinue(Parent, Loc);
    return Continue;
}

ASTExprStmt *
SemaBuilder::CreateExprStmt(ASTBlock *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLocalVar", Logger()
                      .Attr("Parent", Parent)
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTExprStmt *ExprStmt = new ASTExprStmt(Parent, Loc);
    return ExprStmt;
}

ASTVarRef *
SemaBuilder::CreateVarRef(const SourceLocation &Loc, StringRef Name, StringRef NameSpace) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLocalVar", Logger()
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                      .Attr("Name", Name)
                      .Attr("NameSpace", NameSpace).End());
    ASTVarRef *VarRef = CreateVarRef(Loc, Name, "", NameSpace);
    return VarRef;
}

ASTVarRef *
SemaBuilder::CreateVarRef(const SourceLocation &Loc, StringRef Name, StringRef Class, StringRef NameSpace) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLocalVar",
                      "Loc=" << Loc.getRawEncoding() <<
                      ", Name=" << Name <<
                      ", Class=" << Class <<
                      ", NameSpace=" << NameSpace);
    ASTVarRef *VarRef = new ASTVarRef(Loc, std::string(Name), std::string(Class), std::string(NameSpace));
    return VarRef;
}

ASTVarRef *
SemaBuilder::CreateVarRef(ASTLocalVar *LocalVar) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateVarRef",
                      Logger().Attr("LocalVar", LocalVar).End());
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

ASTVarRef *
SemaBuilder::CreateVarRef(ASTGlobalVar *GlobalVar) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateVarRef",
                      Logger().Attr("GlobalVar", GlobalVar).End());
    ASTVarRef *VarRef = CreateVarRef(GlobalVar->Location, GlobalVar->Name, GlobalVar->NameSpace->getName());
    VarRef->Def = GlobalVar;
    return VarRef;
}

ASTVarRef *
SemaBuilder::CreateVarRef(ASTClassVar *ClassVar) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateVarRef",
                      Logger().Attr("ClassVar", ClassVar).End());
    ASTVarRef *VarRef = CreateVarRef(ClassVar->getLocation(), ClassVar->Name, ClassVar->Class->Name, ClassVar->Class->NameSpace->getName());
    VarRef->Def = ClassVar;
    return VarRef;
}

ASTEmptyExpr *
SemaBuilder::CreateExpr(ASTStmt *Stmt) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateExpr",
                      Logger().Attr("Stmt", Stmt).End());
    ASTEmptyExpr *Expr = new ASTEmptyExpr(SourceLocation());
    AddExpr(Stmt, Expr);
    return Expr;
}

ASTValueExpr *
SemaBuilder::CreateExpr(ASTStmt *Stmt, ASTValue *Value) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateExpr",
                      Logger().Attr("Stmt", Stmt).Attr("Value", Value).End());

    ASTValueExpr *ValueExpr = new ASTValueExpr(Value);
    ValueExpr->Stmt = Stmt; // TODO add ASTExpr() constructor
    AddExpr(Stmt, ValueExpr);
    return ValueExpr;
}

ASTFunctionCallExpr *
SemaBuilder::CreateExpr(ASTStmt *Stmt, ASTFunctionCall *FunctionCall) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateExpr",
                      Logger().Attr("Stmt", Stmt).Attr("FunctionCall", FunctionCall).End());

    ASTFunctionCallExpr *FunctionCallExpr = new ASTFunctionCallExpr(FunctionCall);
    FunctionCallExpr->Stmt = Stmt;
    AddExpr(Stmt, FunctionCallExpr);
    return FunctionCallExpr;
}

ASTVarRefExpr *
SemaBuilder::CreateExpr(ASTStmt *Stmt, ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateExpr",
                      Logger().Attr("Stmt", Stmt).Attr("VarRef", Stmt).End());

    ASTVarRefExpr *VarRefExpr = new ASTVarRefExpr(VarRef);
    VarRefExpr->Stmt = Stmt;
    AddExpr(Stmt, VarRefExpr);
    return VarRefExpr;
}

ASTUnaryGroupExpr *
SemaBuilder::CreateUnaryExpr(ASTStmt *Stmt, const SourceLocation &Loc, ASTUnaryOperatorKind Kind,
                             ASTUnaryOptionKind OptionKind, ASTVarRefExpr *First) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateUnaryExpr",
                      Logger().Attr("Stmt", Stmt)
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                      .Attr("Kind", (uint64_t) Kind)
                      .Attr("OptionKind", (uint64_t) OptionKind)
                      .Attr("First", First).End());

    ASTUnaryGroupExpr *UnaryExpr = new ASTUnaryGroupExpr(Loc, Kind, OptionKind, First);
    UnaryExpr->Stmt = Stmt;

    // Set Parent Expression
    First->Parent = UnaryExpr;

    AddExpr(Stmt, UnaryExpr);
    return UnaryExpr;
}

ASTBinaryGroupExpr *
SemaBuilder::CreateBinaryExpr(ASTStmt *Stmt, const SourceLocation &OpLoc, ASTBinaryOperatorKind Kind,
                              ASTExpr *First, ASTExpr *Second) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateBinaryExpr", Logger()
                        .Attr("Stmt", Stmt)
                        .Attr("OpLoc", (uint64_t) OpLoc.getRawEncoding())
                        .Attr("Kind", (uint64_t) Kind)
                        .Attr("First", First)
                        .Attr("Second", Second).End());

    ASTBinaryGroupExpr *BinaryExpr = new ASTBinaryGroupExpr(OpLoc, Kind, First, Second);
    BinaryExpr->Stmt = Stmt;

    // Set Parent Expression
    First->Parent = BinaryExpr;
    Second->Parent = BinaryExpr;

    AddExpr(Stmt, BinaryExpr);
    return BinaryExpr;
}

ASTTernaryGroupExpr *
SemaBuilder::CreateTernaryExpr(ASTStmt *Stmt, ASTExpr *First, const SourceLocation &IfLoc,
                               ASTExpr *Second, const SourceLocation &ElseLoc, ASTExpr *Third) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateTernaryExpr", Logger()
                      .Attr("Stmt", Stmt)
                      .Attr("First", First)
                      .Attr("IfLoc", (uint64_t) IfLoc.getRawEncoding())
                      .Attr("Second", Second)
                      .Attr("Loc", (uint64_t) ElseLoc.getRawEncoding())
                      .Attr("Third", Third).End());

    ASTTernaryGroupExpr *TernaryExpr = new ASTTernaryGroupExpr(First, IfLoc, Second, ElseLoc, Third);
    TernaryExpr->Stmt = Stmt;

    // Set Parent Expression
    First->Parent = TernaryExpr;
    Second->Parent = TernaryExpr;
    Third->Parent = TernaryExpr;

    AddExpr(Stmt, TernaryExpr);
    return TernaryExpr;
}

ASTBlock *
SemaBuilder::CreateBlock(ASTBlock *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateBlock", Logger()
                      .Attr("Parent", Parent)
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTBlock *Block = new ASTBlock(Parent, Loc);
    return Block;
}

ASTBlock *
SemaBuilder::getBlock(ASTFunction *Function) {
    return Function->Body;
}

ASTIfBlock *
SemaBuilder::CreateIfBlock(ASTBlock *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateIfBlock", Logger()
                      .Attr("Parent", Parent)
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTIfBlock *Block = new ASTIfBlock(Parent, Loc);
    return Block;
}

ASTElsifBlock *
SemaBuilder::CreateElsifBlock(ASTIfBlock *IfBlock, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateElsifBlock", Logger()
                      .Attr("IfBlock", IfBlock)
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTElsifBlock *Block = new ASTElsifBlock(IfBlock, Loc);
    return Block;
}

ASTElseBlock *
SemaBuilder::CreateElseBlock(ASTIfBlock *IfBlock, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateElseBlock", Logger()
                        .Attr("IfBlock", IfBlock)
                        .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTElseBlock *Block = new ASTElseBlock(IfBlock, Loc);
    return Block;
}

ASTSwitchBlock *
SemaBuilder::CreateSwitchBlock(ASTBlock *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateSwitchBlock", Logger()
                      .Attr("Parent", Parent)
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTSwitchBlock *Block = new ASTSwitchBlock(Parent, Loc);
    return Block;
}

ASTSwitchCaseBlock *
SemaBuilder::CreateSwitchCaseBlock(ASTSwitchBlock *SwitchBlock, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateSwitchCaseBlock", Logger()
                      .Attr("SwitchBlock", SwitchBlock)
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTSwitchCaseBlock *Block = new ASTSwitchCaseBlock(SwitchBlock, Loc);
    return Block;
}

ASTSwitchDefaultBlock *
SemaBuilder::CreateSwitchDefaultBlock(ASTSwitchBlock *SwitchBlock, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateSwitchDefaultBlock", Logger()
                      .Attr("SwitchBlock", SwitchBlock)
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTSwitchDefaultBlock *Block = new ASTSwitchDefaultBlock(SwitchBlock, Loc);
    return Block;
}

ASTWhileBlock *
SemaBuilder::CreateWhileBlock(ASTBlock *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateWhileBlock", Logger()
                      .Attr("Parent", Parent)
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
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
ASTForBlock *
SemaBuilder::CreateForBlock(ASTBlock *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateForBlock", Logger()
                      .Attr("Parent", Parent)
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTForBlock *Block = new ASTForBlock(Parent, Loc);
    return Block;
}

ASTForLoopBlock *
SemaBuilder::CreateForLoopBlock(ASTForBlock *ForBlock, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateForLoopBlock", Logger()
                        .Attr("ForBlock", ForBlock)
                        .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTForLoopBlock *Block = new ASTForLoopBlock(ForBlock, Loc);
    return Block;
}

ASTForPostBlock *
SemaBuilder::CreateForPostBlock(ASTForBlock *ForBlock, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateForPostBlock", Logger()
                        .Attr("ForBlock", ForBlock)
                        .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTForPostBlock *Block = new ASTForPostBlock(ForBlock, Loc);
    return Block;
}

/**
 * Add an ASTNameSpace to the ASTContext if not exists yet
 * @param Name
 * @param ExternLib
 * @return the created or retrieved ASTNameSpace
 */
ASTNameSpace *
SemaBuilder::AddNameSpace(const std::string &Name, bool ExternLib) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddNameSpace",
                      "Name=" << Name <<
                      ", ExternLib=" << ExternLib);
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
bool
SemaBuilder::AddNode(ASTNode *Node) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddNode", Logger().Attr("Node", Node).End());

    // Add to Nodes
    auto Pair = std::make_pair(Node->getName(), Node);
    // TODO check duplicated in namespace and context
    return Node->NameSpace->Nodes.insert(Pair).second && Context->Nodes.insert(Pair).second;
}

bool
SemaBuilder::AddImport(ASTNode *Node, ASTImport * Import) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddImport", Logger()
                        .Attr("Node", Node)
                        .Attr("Import", Import).End());

    if (S.Validator->CheckImport(Node, Import)) {
        std::string Name = Import->getAlias().empty() ? Import->getName() : Import->getAlias();

        // Check if this Node already own this Import
        if (Node->Imports.lookup(Name)) {
            S.Diag(Import->getLocation(), diag::err_conflict_import) << Name;
            return false;
        }

        // Add Import to Node
        auto Pair = std::make_pair(Name, Import);
        return Node->Imports.insert(Pair).second;
    }

    return false;
}

bool
SemaBuilder::AddClass(ASTNode *Node, ASTClass *Class) {
    FLY_DEBUG_MESSAGE("ASTNode", "AddFunction", Logger()
                      .Attr("Node", Node)
                      .Attr("Class", Class).End());

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

bool
SemaBuilder::AddGlobalVar(ASTNode *Node, ASTGlobalVar *GlobalVar, ASTValue *Value) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddGlobalVar",
                      Logger().Attr("Node", Node).Attr("GlobalVar", GlobalVar).Attr("Value=", Value).End());

    // Set the Expr with ASTValueExpr
    return AddGlobalVar(Node, GlobalVar, CreateExpr(nullptr, Value));
}

bool
SemaBuilder::AddGlobalVar(ASTNode *Node, ASTGlobalVar *GlobalVar, ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddGlobalVar", Logger()
                      .Attr("Node", Node)
                      .Attr("GlobalVar", GlobalVar)
                      .Attr("Expr", Expr).End());

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

bool
SemaBuilder::AddFunction(ASTNode *Node, ASTFunction *Function) {
    FLY_DEBUG_MESSAGE("ASTNode", "AddFunction",
                      Logger().Attr("Node", Node).Attr("Function", Function).End());

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


bool
SemaBuilder::InsertFunction(llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> &Functions,
                                 ASTFunction *Function) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "InsertFunction",
                      Logger().Attr("Function", Function).End());
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

bool
SemaBuilder::AddParam(ASTParam *Param) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddParam", Logger().Attr("Param", Param).End());
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

std::string
getComment(std::string &C) {
    if (C.empty()) {
        return C;
    }
    const char *t = " \t\n\r\f\v";
    C = C.substr(2, C.size() - 4);
    C = C.erase(0, C.find_first_not_of(t)); // TODO Check
    return C.erase(C.find_last_not_of(t) + 1);
}

bool
SemaBuilder::AddComment(ASTTopDef *Top, std::string &Comment) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddComment", Logger()
                        .Attr("Top", Top)
                        .Attr("Comment", Comment).End());
    Top->Comment = getComment(Comment);
    return true;
}

bool
SemaBuilder::AddComment(ASTClassVar *ClassVar, std::string &Comment) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddComment", Logger()
                      .Attr("ClassVar", ClassVar)
                      .Attr("Comment", Comment).End());
    ClassVar->Comment = getComment(Comment);
    return true;
}

bool
SemaBuilder::AddComment(ASTClassFunction *ClassFunction, std::string &Comment) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddComment", Logger()
                      .Attr("ClassFunction", ClassFunction)
                      .Attr("Comment", Comment).End());
    ClassFunction->Comment = getComment(Comment);
    return true;
}

bool
SemaBuilder::AddExternalGlobalVar(ASTNode *Node, ASTGlobalVar *GlobalVar) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddExternalGlobalVar", Logger()
                        .Attr("Node", Node)
                        .Attr("GlobalVar", GlobalVar).End());
    // TODO Check duplicate
    return Node->ExternalGlobalVars.insert(std::make_pair(GlobalVar->getName(), GlobalVar)).second;
}

bool
SemaBuilder::AddExternalFunction(ASTNode *Node, ASTFunction *Function) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddExternalFunction", Logger()
                        .Attr("Node", Node)
                        .Attr("Function", Function).End());
    // TODO Check duplicate
    return InsertFunction(Node->ExternalFunctions, Function);
}

bool
SemaBuilder::AddArrayValue(ASTArrayValue *ArrayValue, ASTValue *Value) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddArrayValue", Logger()
                      .Attr("ArrayValue", ArrayValue)
                      .Attr("Value", Value).End());
    ArrayValue->Values.push_back(Value);
    return true;
}

bool
SemaBuilder::AddFunctionCallArg(ASTFunctionCall *FunctionCall, ASTArg *Arg) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddFunctionCallArg", Logger()
                      .Attr("FunctionCall", FunctionCall)
                      .Attr("Arg", Arg).End());
    Arg->Index = FunctionCall->getArgs().empty() ? 0 : FunctionCall->getArgs().size();
    Arg->Call = FunctionCall;
    FunctionCall->Args.push_back(Arg);
    return true;
}

bool
SemaBuilder::AddExpr(ASTStmt *Stmt, ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddExpr",
                      Logger().Attr("Stmt", Stmt).Attr("Expr", Expr).End());
    if (!Expr) {
        return S.Diag(Expr->getLocation(), diag::err_sema_generic) && false;
    }
    if (Stmt)
        switch (Stmt->getKind()) {
            case ASTStmtKind::STMT_EXPR:
            case ASTStmtKind::STMT_ARG:
            case ASTStmtKind::STMT_VAR_DEFINE:
            case ASTStmtKind::STMT_VAR_ASSIGN:
            case ASTStmtKind::STMT_RETURN:
                ((ASTExprStmt *) Stmt)->Expr = Expr;
                break;
            case ASTStmtKind::STMT_BLOCK:
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
            case ASTStmtKind::STMT_BREAK:
            case ASTStmtKind::STMT_CONTINUE:
                assert("Cannot contain an Expr");
        }

    return true;
}

/**
 * Add ExprStmt to Content
 * @param ExprStmt
 * @return true if no error occurs, otherwise false
 */
bool
SemaBuilder::AddStmt(ASTStmt *Stmt) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddStmt", Logger().Attr("Stmt", Stmt).End());

    ASTBlock *Parent = (ASTBlock *) Stmt->Parent;
    Parent->Content.push_back(Stmt);

    if (Stmt->getKind() == ASTStmtKind::STMT_VAR_DEFINE) { // Stmt is ASTLocalVar

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
    } else if (Stmt->getKind() == ASTStmtKind::STMT_VAR_ASSIGN) {
        ASTVarAssign *VarAssign = (ASTVarAssign *) Stmt;

        // Remove from Undefined Var because now have an Expr assigned
        if (VarAssign->getVarRef()->getNameSpace().empty()) { // only for VarRef with empty NameSpace
            auto It = Parent->UndefVars.find(VarAssign->getVarRef()->getName());
            if (It != Parent->UndefVars.end())
                Parent->UndefVars.erase(It);
        }
    } else if (Stmt->getKind() == ASTStmtKind::STMT_BLOCK) {
        return AddBlock((ASTBlock *) Stmt);
    }

    return true;
}

bool
SemaBuilder::AddBlock(ASTBlock *Block) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddBlock",
                      Logger().Attr("Stmt", Block).End());

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

    assert("Invalid block type");
    return false;
}


