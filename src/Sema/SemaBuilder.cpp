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
#include "AST/ASTIdentifier.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTFunctionBase.h"
#include "AST/ASTCall.h"
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
                                           const llvm::StringRef Name, ASTTopScopes *Scopes) {
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
                                         const llvm::StringRef Name, ASTTopScopes *Scopes) {
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
SemaBuilder::CreateClass(ASTNode *Node, ASTClassKind ClassKind, ASTTopScopes *Scopes,
                         const SourceLocation &Loc, const llvm::StringRef Name,
                         llvm::SmallVector<llvm::StringRef, 4> &ExtClasses) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClass",
                      Logger().Attr("Node", Node)
                      .Attr("Scopes", Scopes).End());
    ASTClass *Class = new ASTClass(Node, ClassKind, Scopes, Loc, Name, ExtClasses);
    Class->Type = CreateClassType(Class);

    // Create a default constructor
    if (ClassKind == ASTClassKind::CLASS || ClassKind == ASTClassKind::STRUCT) {
        ASTClassFunction *Constructor = CreateClassConstructor(Class, SourceLocation(),
                                                               new ASTClassScopes(
                                                                       ASTClassVisibilityKind::CLASS_V_PUBLIC, false));
        AddClassConstructor(Constructor);
        Class->autoDefaultConstructor = true;
    } else if (ClassKind == ASTClassKind::ENUM) {
        
        // Create EnumVar which contains the selection value
        ASTUIntType *UIntType = SemaBuilder::CreateUIntType(SourceLocation());
        ASTClassScopes *Scopes = CreateClassScopes(ASTClassVisibilityKind::CLASS_V_PRIVATE, true, false);
        ASTClassVar *EnumVar = new ASTClassVar(SourceLocation(), Class, Scopes, UIntType, "enum");

        // Create Constructor
        ASTClassFunction *Constructor = CreateClassConstructor(Class, SourceLocation(),
                                                               new ASTClassScopes(
                                                                       ASTClassVisibilityKind::CLASS_V_PRIVATE, false));
        ASTParam *Param = CreateParam(Constructor, SourceLocation(), UIntType, "enum");
        AddParam(Param);
        ASTBlock *Block = getBlock(Constructor);
        ASTVarAssign *EnumVarAssign = CreateVarAssign(Block, CreateVarRef(EnumVar));
        CreateExpr(EnumVarAssign, SemaBuilder::CreateIntegerValue(SourceLocation(), 0));
        AddStmt(EnumVarAssign);
        AddClassConstructor(Constructor);

        // Add EnumVar
        AddEnumClassVar(EnumVar);
    }
    
    return Class;
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
SemaBuilder::CreateClass(ASTNode *Node, ASTClassKind ClassKind, ASTTopScopes *Scopes,
                         const SourceLocation &Loc, const llvm::StringRef Name) {
    llvm::SmallVector<llvm::StringRef, 4> Classes;
    return CreateClass(Node, ClassKind, Scopes, Loc, Name, Classes);
}

/**
 * Creates a Scope for Classes
 * @param Visibility
 * @param Constant
 * @return
 */
ASTClassScopes *
SemaBuilder::CreateClassScopes(ASTClassVisibilityKind Visibility, bool Constant, bool Static) {
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
SemaBuilder::CreateClassVar(ASTClass *Class, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                                         ASTClassScopes *Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassVar",
                      Logger().Attr("Class", Class)
                                            .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                                            .Attr("Type=", Type)
                                            .Attr("Name", Name)
                                            .Attr("Scopes", Scopes).End());
    return new ASTClassVar(Loc, Class, Scopes, Type, Name);
}

ASTClassVar *
SemaBuilder::CreateEnumClassVar(ASTClass *Class, const SourceLocation &Loc, llvm::StringRef Name) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassVar",
                      Logger().Attr("Class", Class)
                              .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Name", Name).End());
    ASTClassScopes *Scopes = CreateClassScopes(ASTClassVisibilityKind::CLASS_V_DEFAULT, true, true);
    ASTClassVar *ClassVar = CreateClassVar(Class, Loc, Class->getType(), Name, Scopes);
    return ClassVar;
}

ASTClassFunction *
SemaBuilder::CreateClassConstructor(ASTClass *Class, const SourceLocation &Loc, ASTClassScopes *Scopes) {

    ASTClassFunction *F = CreateClassMethod(Class, Loc, CreateClassType(Class), Class->Name, Scopes);
    F->Constructor = true;
    return F;
}

ASTClassFunction *
SemaBuilder::CreateClassMethod(ASTClass *Class, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                               ASTClassScopes *Scopes) {
    ASTClassFunction *F = new ASTClassFunction(Loc, Class, Scopes, Type, Name);
    F->Params = new ASTParams();
    F->Body = CreateBlock(nullptr, SourceLocation());
    F->Body->Top = F;
    return F;
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
SemaBuilder::CreateClassType(ASTIdentifier *Identifier) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassType",
                      Logger().Attr("Identifier", Identifier).End());
    ASTClassType *ClassType = new ASTClassType(Identifier);
    return ClassType;
}

/**
 * Creates a class type with definition
 * @param Class
 * @return
 */
ASTClassType *
SemaBuilder::CreateClassType(ASTClass *Class) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassType", Logger().Attr("Class", Class).End());
    ASTClassType *ClassType = new ASTClassType(Class);
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
        Value = nullptr;
    }
    return Value;
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
SemaBuilder::CreateParam(ASTFunctionBase *Function, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, bool Constant) {
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
                            llvm::StringRef Name, bool Constant) {
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
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateContinue", Logger()
                      .Attr("Parent", Parent)
                      .Attr("Loc", (uint64_t)Loc.getRawEncoding()).End());
    ASTContinue *Continue = new ASTContinue(Parent, Loc);
    return Continue;
}

ASTExprStmt *
SemaBuilder::CreateExprStmt(ASTBlock *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateExprStmt", Logger()
                      .Attr("Parent", Parent)
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTExprStmt *ExprStmt = new ASTExprStmt(Parent, Loc);
    return ExprStmt;
}

/**
 * Create an ASTFunctionCall without definition
 * @param Location
 * @param Name
 * @param NameSpace
 * @return
 */
ASTCall *
SemaBuilder::CreateCall(ASTIdentifier *Identifier) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateCall",
                      Logger().Attr("Identifier", Identifier).End());
    ASTCall *Call = new ASTCall(Identifier);
    return Call;
}

/**
 * Creates an ASTFunctionCall with definition
 * @param Stmt
 * @param Function
 * @return
 */
ASTCall *
SemaBuilder::CreateCall(ASTFunctionBase *Function) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateCall",
                      Logger().Attr("Function=", Function).End());
    ASTCall *Call = new ASTCall(Function);
    return Call;
}

ASTCall *
SemaBuilder::CreateCall(ASTReference *Instance, ASTFunctionBase *Function) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateCall",
                      Logger().Attr("Function=", Function).End());
    ASTCall *Call = new ASTCall(Function);
    Call->Instance = Instance;
    return Call;
}

ASTVarRef *
SemaBuilder::CreateVarRef(ASTVar *Var) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateVarRef",
                      Logger().Attr("Var", Var).End());
    ASTVarRef *VarRef = new ASTVarRef(Var);
    return VarRef;
}

ASTVarRef *
SemaBuilder::CreateVarRef(ASTReference *Instance, ASTVar *Var) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateVarRef",
                      Logger().Attr("Instance", Instance).Attr("Var", Var).End());
    ASTVarRef *VarRef = new ASTVarRef(Var);
    VarRef->Instance = Instance;
    return VarRef;
}

ASTVarRef *
SemaBuilder::CreateVarRef(ASTIdentifier *Identifier) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateVarRef",
                      Logger().Attr("Identifier", Identifier).End());
    ASTVarRef *VarRef = new ASTVarRef(Identifier);
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
    assert(Value && "Create ASTValueExpr by ASTValue");
    ASTValueExpr *ValueExpr = new ASTValueExpr(Value);
    ValueExpr->Stmt = Stmt;
    AddExpr(Stmt, ValueExpr);
    return ValueExpr;
}

ASTCallExpr *
SemaBuilder::CreateExpr(ASTStmt *Stmt, ASTCall *Call) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateExpr",
                      Logger().Attr("Stmt", Stmt).Attr("Call", Call).End());

    ASTCallExpr *CallExpr = new ASTCallExpr(Call);
    CallExpr->Stmt = Stmt;
    AddExpr(Stmt, CallExpr);
    return CallExpr;
}

ASTCallExpr *
SemaBuilder::CreateNewExpr(ASTStmt *Stmt, ASTCall *Call) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateNewExpr",
                      Logger().Attr("Stmt", Stmt).Attr("Call", Call).End());
    Call->New = true;
    return CreateExpr(Stmt, Call);
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
SemaBuilder::getBlock(ASTFunctionBase *Function) {
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
SemaBuilder::AddClass(ASTClass *Class) {
    FLY_DEBUG_MESSAGE("ASTNode", "AddFunction", Logger()
                      .Attr("Class", Class).End());

    // Lookup into namespace
    ASTNode *Node = Class->Node;
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
SemaBuilder::AddGlobalVar(ASTGlobalVar *GlobalVar, ASTValue *Value) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddGlobalVar",
                      Logger().Attr("GlobalVar", GlobalVar).Attr("Value=", Value).End());

    // Set the Expr with ASTValueExpr
    return AddGlobalVar(GlobalVar, CreateExpr(nullptr, Value));
}

bool
SemaBuilder::AddGlobalVar(ASTGlobalVar *GlobalVar, ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddGlobalVar", Logger()
                      .Attr("GlobalVar", GlobalVar)
                      .Attr("Expr", Expr).End());
    ASTNode *Node = GlobalVar->Node;

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
SemaBuilder::AddFunction(ASTFunction *Function) {
    FLY_DEBUG_MESSAGE("ASTNode", "AddFunction",
                      Logger().Attr("Function", Function).End());
    ASTNode *Node = Function->Node;

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
SemaBuilder::AddClassVar(ASTClassVar *Var) {
    ASTClass *Class = Var->Class;
    if (Class->Vars.insert(std::pair<llvm::StringRef, ASTClassVar *>(Var->getName(), Var)).second) {

        // Set default value if not set
        if (!Var->getExpr()) {
            ASTValueExpr *Expr = S.Builder->CreateExpr(nullptr, SemaBuilder::CreateDefaultValue(Var->getType()));
            Expr->Type = Var->getType();
            Var->setExpr(Expr);
        }

        return Var;
    }

    S.Diag(Var->getLocation(), diag::err_sema_class_field_redeclare) << Var->getName();
    return false;
}

bool
SemaBuilder::AddEnumClassVar(ASTClassVar *Var) {
    ASTClass *Class = Var->Class;
    if (Class->Vars.insert(std::pair<llvm::StringRef, ASTClassVar *>(Var->getName(), Var)).second) {

        // Enum expr is not permitted
        if (Var->getExpr()) {
            S.Diag(Var->getLocation(), diag::err_sema_class_enum_expr) << Var->getName();
            return false;
        }

        // Check if is Enum
        if (Class->getClassKind() != ASTClassKind::ENUM) {
            S.Diag(Var->getLocation(), diag::err_sema_class_enum_var) << Var->getName();
            return false;
        }

        // enum Test {
        //     A = Test(1)
        //     B = Test(2)
        //     ...
        // }

        // Create a call to constructor with the sequential number
        ASTClassFunction *Constructor = Class->getConstructors().find(1)->second.front();
        ASTCall *Call = CreateCall(Constructor);
        uint32_t i = Class->getVars().size() + 1; // sequential
        ASTValueExpr *ValueExpr = CreateExpr(nullptr, SemaBuilder::CreateIntegerValue(SourceLocation(), i));
        AddCallArg(Call, ValueExpr);
        ASTCallExpr *Expr = S.Builder->CreateExpr(nullptr, Call);
        Expr->Type = Var->getType();
        Var->setExpr(Expr);

        return Var;
    }

    S.Diag(Var->getLocation(), diag::err_sema_class_field_redeclare) << Var->getName();
    return false;
}

bool
SemaBuilder::AddClassMethod(ASTClassFunction *Method) {
    if (!InsertFunction(Method->Class->Methods, Method)) {
        S.Diag(Method->getLocation(), diag::err_sema_class_method_redeclare) << Method->getName();
        return false;
    }
    return true;
}

bool
SemaBuilder::AddClassConstructor(ASTClassFunction *Method) {
    ASTClass *Class = Method->Class;
    if (Class->autoDefaultConstructor) {
        Class->Constructors.erase(0); // Remove default constructor if a new Constructor is defined
        Class->autoDefaultConstructor = false;
    }
    if (!InsertFunction(Class->Constructors, Method)) {
        S.Diag(Method->getLocation(), diag::err_sema_class_method_redeclare) << Method->getName();
        return false;
    }
    return true;
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

//llvm::StringRef
//getComment(llvm::StringRef C) {
//    if (C.empty()) {
//        return C;
//    }
//    const char *t = " \t\n\r\f\v";
//    C = C.substr(2, C.size() - 4);
//    C = C.erase(0, C.find_first_not_of(t)); // TODO Check
//    return C.erase(C.find_last_not_of(t) + 1);
//}

bool
SemaBuilder::AddComment(ASTTopDef *Top, llvm::StringRef Comment) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddComment", Logger()
                        .Attr("Top", Top)
                        .Attr("Comment", Comment).End());
    Top->Comment = Comment;
    return true;
}

bool
SemaBuilder::AddComment(ASTClassVar *ClassVar, llvm::StringRef Comment) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddComment", Logger()
                      .Attr("ClassVar", ClassVar)
                      .Attr("Comment", Comment).End());
    ClassVar->Comment = Comment;
    return true;
}

bool
SemaBuilder::AddComment(ASTClassFunction *ClassFunction, llvm::StringRef Comment) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddComment", Logger()
                      .Attr("ClassFunction", ClassFunction)
                      .Attr("Comment", Comment).End());
    ClassFunction->Comment = Comment;
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
SemaBuilder::AddCallArg(ASTCall *Call, ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddCallArg", Logger()
                      .Attr("FunctionCall", Call)
                      .Attr("Expr", Expr).End());
    if (Expr->getExprKind() != ASTExprKind::EXPR_EMPTY) {
        ASTArg *Arg = new ASTArg(Call, Expr);
        Arg->Index = Call->getArgs().empty() ? 0 : Call->getArgs().size();
        Call->Args.push_back(Arg);
    }
    return true;
}

bool
SemaBuilder::AddExpr(ASTStmt *Stmt, ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddExpr",
                      Logger().Attr("Stmt", Stmt).Attr("Expr", Expr).End());
    if (!Expr) {
        return S.Diag(Expr->getLocation(), diag::err_sema_generic) && false;
    }
    if (Stmt) {
        switch (Stmt->getKind()) {
            case ASTStmtKind::STMT_EXPR:
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
            Parent->UnInitVars.insert(std::pair<std::string, ASTLocalVar *>(LocalVar->getName(), LocalVar));
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
        if (VarAssign->getVarRef()->isLocalVar()) { // only for LocalVar
            auto It = Parent->UnInitVars.find(VarAssign->getVarRef()->getDef()->getName());
            if (It != Parent->UnInitVars.end())
                Parent->UnInitVars.erase(It);
        }
    } else if (Stmt->getKind() == ASTStmtKind::STMT_BLOCK) {
        return AddBlock((ASTBlock *) Stmt);
    }

    return true;
}

template <class T>
bool
SemaBuilder::InsertFunction(llvm::StringMap<std::map <uint64_t,llvm::SmallVector <T *, 4>>> &Functions, T *Function) {

    // Functions is llvm::StringMap<std::map <uint64_t, llvm::SmallVector <ASTFunction *, 4>>>
    const auto &StrMapIt = Functions.find(Function->getName());
    if (StrMapIt == Functions.end()) { // This Node not contains a Function with this Function->Name

        // add to llvm::SmallVector
        llvm::SmallVector<T *, 4> Vect;
        Vect.push_back(Function);

        // add to std::map
        std::map<uint64_t, llvm::SmallVector<T *, 4>> IntMap;
        IntMap.insert(std::make_pair(Function->Params->getSize(), Vect));

        // add to llvm::StringMap
        return Functions.insert(std::make_pair(Function->getName(), IntMap)).second;
    } else {
        return InsertFunction(StrMapIt->second, Function);
    }
}

template <class T>
bool
SemaBuilder::InsertFunction(std::map <uint64_t,llvm::SmallVector <T *, 4>> &Functions, T *Function) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddClassMethod",
                      Logger().Attr("Function", Function).End());

    // This Node contains a Function with this Function->Name
    const auto &IntMapIt = Functions.find(Function->Params->getSize());
    if (IntMapIt == Functions.end()) { // but not have the same number of Params

        // add to llvm::SmallVector
        llvm::SmallVector<T *, 4> Vect;
        Vect.push_back(Function);

        // add to std::map
        std::pair<uint64_t, SmallVector<T *, 4>> IntMapPair = std::make_pair(
                Function->Params->getSize(), Vect);

        return Functions.insert(std::make_pair(Function->Params->getSize(),Vect)).second;
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
            SmallVector<T *, 4> Vect = IntMapIt->second;
            Vect.push_back(Function);
            return true;
        } else { // Function is duplicated
            // already contains this Function
            S.Diag((Function->getLocation()), diag::err_duplicate_func) << Function->getName();
            return false;
        }
    }
}

bool
SemaBuilder::AddBlock(ASTBlock *Block) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddBlock",
                      Logger().Attr("Stmt", Block).End());

    switch (Block->getBlockKind()) {

        case ASTBlockKind::BLOCK:
            return true;

        case ASTBlockKind::BLOCK_IF:
            Block->UnInitVars = ((ASTIfBlock *) Block->getParent())->UnInitVars;
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