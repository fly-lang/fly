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
#include "AST/ASTDeleteStmt.h"
#include "AST/ASTParams.h"
#include "AST/ASTBlock.h"
#include "AST/ASTIfBlock.h"
#include "AST/ASTWhileBlock.h"
#include "AST/ASTForBlock.h"
#include "AST/ASTSwitchBlock.h"
#include "AST/ASTHandleStmt.h"
#include "AST/ASTVarStmt.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTValue.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassVar.h"
#include "AST/ASTEnum.h"
#include "AST/ASTEnumEntry.h"
#include "AST/ASTClassFunction.h"
#include "AST/ASTError.h"
#include "AST/ASTFailStmt.h"
#include "Sys/Sys.h"
#include "Basic/SourceLocation.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"

#include "llvm/ADT/StringMap.h"
#include <map>

using namespace fly;

/**
 * Private constructor used only from Sema constructor
 * @param S
 */
SemaBuilder::SemaBuilder(Sema &S) : S(S) {
    FLY_DEBUG("SemaBuilder", "SemaBuilder");
}

ASTContext *
SemaBuilder::CreateContext() {
    FLY_DEBUG("SemaBuilder", "CreateContext");
    ASTContext *Context = new ASTContext();
    Context->DefaultNameSpace = CreateDefaultNameSpace();
    Sys::Build(S); // add fail() functions in default namespace
    AddNameSpace(Context->DefaultNameSpace);
    return Context;
}

/**
 * Creates an ASTNode
 * If NameSpace doesn't exists it will be created
 * @param Name
 * @param NameSpace
 * @return the ASTNode
 */
ASTNode *
SemaBuilder::CreateNode(const std::string &Name) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateNode", "Name=" << Name);
    ASTNode *Node = new ASTNode(Name, S.Context, false);
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
SemaBuilder::CreateHeaderNode(const std::string &Name) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateHeaderNode", "Name=" << Name);
    return new ASTNode(Name, S.Context, true);
}

ASTNameSpace *SemaBuilder::CreateDefaultNameSpace() {
    FLY_DEBUG("SemaBuilder", "CreateDefaultNameSpace");
    return new ASTNameSpace(SourceLocation(), ASTNameSpace::DEFAULT, S.Context);
}

/**
 * Create a NameSpace for each parent
 * NS3.NS2.NS1 -> Identifier->Parent
 * NS2.NS1 -> Identifier->Parent->Parent
 * NS3 -> Identifier->Parent->Parent->Parent ... until to Root
 * @param Identifier
 * @return
 */
ASTNameSpace *SemaBuilder::CreateNameSpace(ASTIdentifier *Identifier) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateNameSpace", "Identifier=" << Identifier->str());
    ASTNameSpace *NameSpace;

    // Error: namespace empty
    if (Identifier == nullptr) {
        S.Diag(Identifier->getLocation(), diag::err_namespace_invalid);
    }

    NameSpace = new ASTNameSpace(Identifier->getLocation(), Identifier->getName(), S.Context);

    // Iterate over parents
    if (Identifier->getParent() != nullptr) {
        NameSpace->Parent = CreateNameSpace(Identifier->getParent());
    }

    return NameSpace;
}

ASTImport *
SemaBuilder::CreateImport(const SourceLocation &Loc, llvm::StringRef Name) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateImport",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Name", Name).End());
    return new ASTImport(Loc, Name);
}

ASTAlias *
SemaBuilder::CreateAlias(const SourceLocation &Loc, llvm::StringRef Name) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateImport",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Name", Name).End());
    return new ASTAlias(Loc, Name);
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
                                           const llvm::StringRef Name, ASTScopes *Scopes) {
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
                                         const llvm::StringRef Name, ASTScopes *Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateFunction",
                      Logger().Attr("Node", Node)
                              .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Type", Type)
                              .Attr("Name", Name)
                              .Attr("Scopes", Scopes).End());
    ASTFunction *F = new ASTFunction(Loc, Node, Type, Name, Scopes);
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
SemaBuilder::CreateClass(ASTNode *Node, ASTClassKind ClassKind, ASTScopes *ClassScopes,
                         const SourceLocation &Loc, const llvm::StringRef Name,
                         llvm::SmallVector<ASTClassType *, 4> &ClassTypes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClass",
                      Logger().Attr("Node", Node)
                              .Attr("ClassKind", (uint64_t) ClassKind)
                              .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Name", Name)
                              .Attr("Scopes", ClassScopes)
                              .End());
    ASTClass *Class = new ASTClass(Node, ClassKind, ClassScopes, Loc, Name);
    Class->SuperClasses = ClassTypes;
    Class->Type = CreateClassType(Class);

    // Create a default constructor
    if (ClassKind == ASTClassKind::CLASS || ClassKind == ASTClassKind::STRUCT) {
        ASTClassFunction *Constructor = CreateClassConstructor(Class, SourceLocation(),
                                                               new ASTScopes(SourceLocation()));
        AddClassConstructor(Constructor);
        Class->autoDefaultConstructor = true;
    }
    
    return Class;
}

/**
 * Creates a Scope for Classes
 * @param Visibility
 * @param Constant
 * @param Static
 * @return
 */
ASTScopes *
SemaBuilder::CreateScopes(ASTVisibilityKind Visibility, bool Constant, bool Static) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassScopes",
                      Logger().Attr("Visibility", (uint64_t) Visibility)
                              .Attr("Constant", Constant)
                              .Attr("Static", Static).End());
    ASTScopes *Scopes = new ASTScopes(SourceLocation());
    Scopes->setVisibility(Visibility);
    Scopes->setConstant(Constant);
    Scopes->setStatic(Static);
    return Scopes;
}

/**
 * Creates a ASTClassVar
 * @param Class
 * @param Loc
 * @param Type
 * @param Name
 * @param Scopes
 * @return
 */
ASTClassVar *
SemaBuilder::CreateClassVar(ASTClass *Class, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                            ASTScopes *Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassVar",
                      Logger().Attr("Class", Class)
                                .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                                .Attr("Type", Type)
                                .Attr("Name", Name)
                                .Attr("Scopes", Scopes).End());
    return new ASTClassVar(Loc, Class, Scopes, Type, Name);
}

ASTClassFunction *
SemaBuilder::CreateClassConstructor(ASTClass *Class, const SourceLocation &Loc, ASTScopes *Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassConstructor",
                      Logger().Attr("Class", Class)
                              .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Scopes", Scopes)
                              .End());
    ASTClassFunction *F = CreateClassMethod(Class, Loc, CreateVoidType(Loc), Class->Name, Scopes);
    F->Constructor = true;
    CreateBody(F);
    return F;
}

ASTClassFunction *
SemaBuilder::CreateClassMethod(ASTClass *Class, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                               ASTScopes *Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassConstructor",
                      Logger().Attr("Class", Class)
                              .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Type", Type)
                              .Attr("Name=", Name)
                              .Attr("Scopes", Scopes)
                              .End());
    ASTClassFunction *F = new ASTClassFunction(Loc, Class, Scopes, Type, Name);
    return F;
}

ASTEnum *
SemaBuilder::CreateEnum(ASTNode *Node, ASTScopes *Scopes, const SourceLocation &Loc, const llvm::StringRef Name,
                        llvm::SmallVector<ASTEnumType *, 4> EnumTypes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateEnum",
                      Logger().Attr("Node", Node)
                              .Attr("Scopes", Scopes).Attr("Name", Name).End());
    ASTEnum *Enum = new ASTEnum(Node, Scopes, Loc, Name, EnumTypes);
    Enum->Type = CreateEnumType(Enum);
    return Enum;
}

ASTEnum *
SemaBuilder::CreateEnum(ASTNode *Node, ASTScopes *Scopes, const SourceLocation &Loc, const llvm::StringRef Name) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassVar",
                      Logger().Attr("Node", Node)
                              .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Name", Name)
                              .Attr("Scopes", Scopes).End());
    llvm::SmallVector<ASTEnumType *, 4> EnumTypes;
    return CreateEnum(Node, Scopes, Loc, Name, EnumTypes);
}

ASTEnumEntry *SemaBuilder::CreateEnumEntry(ASTEnum *Enum, const SourceLocation &Loc, llvm::StringRef Name) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateEnumEntry",
                      Logger().Attr("Enum", Enum)
                              .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Name", Name).End());
    return new ASTEnumEntry(Enum, Loc, Name);
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
                      Logger()
                        .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                        .Attr("Type", Type)
                        .Attr("Size", Size).End());
    return new ASTArrayType(Loc, Type, Size);
}

ASTStringType *
SemaBuilder::CreateStringType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateStringType", "Loc=" << Loc.getRawEncoding());
    return new ASTStringType(Loc);
}

/**
 * Creates a class type without definition
 * @param Identifier
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
 * Creates an enum type without definition
 * @param Identifier
 * @return
 */
ASTEnumType *
SemaBuilder::CreateEnumType(ASTIdentifier *Identifier) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateEnumType",
                      Logger().Attr("Identifier", Identifier).End());
    ASTEnumType *EnumType = new ASTEnumType(Identifier);
    return EnumType;
}

ASTIdentityType *
SemaBuilder::CreateIdentityType(ASTIdentifier *Identifier) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateIdentityType",
                      Logger().Attr("Identifier", Identifier).End());
    return new ASTIdentityType(Identifier);
}

ASTErrorType *
SemaBuilder::CreateErrorType() {
    FLY_DEBUG("SemaBuilder", "CreateErrorType");
    return new ASTErrorType();
}

/**
 * Creates an enum type with definition
 * @param Enum
 * @return
 */
ASTEnumType *
SemaBuilder::CreateEnumType(ASTEnum *Enum) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateEnumType", Logger().Attr("Enum", Enum).End());
    ASTEnumType *EnumType = new ASTEnumType(Enum);
    EnumType->Def = Enum;
    return EnumType;
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
 * Creates a zero value
 * @param Loc
 * @return
 */
ASTZeroValue *
SemaBuilder::CreateZeroValue(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateNullValue", "Loc=" << Loc.getRawEncoding());
    return new ASTZeroValue(Loc);
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

ASTStringValue *
SemaBuilder::CreateStringValue(const SourceLocation &Loc, StringRef Str) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateStringValue",
                      "Loc=" << Loc.getRawEncoding());
    return new ASTStringValue(Loc, Str);
}

ASTStructValue *
SemaBuilder::CreateStructValue(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateArrayValue",
                      "Loc=" << Loc.getRawEncoding());
    return new ASTStructValue(Loc);
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
    } else if (Type->isIdentity()) {
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
SemaBuilder::CreateParam(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, ASTScopes *Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateParam",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                      .Attr("Type", Type)
                      .Attr("Name", Name)
                      .Attr("Scopes", Scopes)
                      .End());
    ASTParam *Param = new ASTParam(Loc, Type, Name, Scopes);
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
SemaBuilder::CreateLocalVar(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, ASTScopes *Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLocalVar",
                      Logger().Attr("Name", Name).End());
    ASTLocalVar *LocalVar = new ASTLocalVar(Loc, Type, Name, Scopes);
    return LocalVar;
}

/**
 * Creates an ASTVarAssign from ASTVarRef
 * @param Parent
 * @param VarRef
 * @return
 */
ASTVarStmt *
SemaBuilder::CreateVarStmt(ASTBlock *Parent, ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateVarStmt",
                      Logger().Attr("Parent", Parent).End());
    ASTVarStmt *VarDefine = new ASTVarStmt(Parent, VarRef->getLocation(), VarRef);
    return VarDefine;
}

/**
 * Creates an ASTVarAssign from ASTVar
 * @param Parent
 * @param VarRef
 * @return
 */
ASTVarStmt *
SemaBuilder::CreateVarStmt(ASTBlock *Parent, ASTVar *Var) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateVarStmt",
                      Logger().Attr("Parent", Parent).End());
    ASTVarRef *VarRef = CreateVarRef(Var);
    ASTVarStmt *VarDefine = new ASTVarStmt(Parent, Var->getLocation(), VarRef);
    return VarDefine;
}

/**
 * Creates an ASTReturn
 * @param Parent
 * @param Loc
 * @return
 */
ASTReturnStmt *
SemaBuilder::CreateReturn(ASTBlock *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLocalVar", Logger()
                      .Attr("Parent", Parent)
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTReturnStmt *Return = new ASTReturnStmt(Parent, Loc);
    return Return;
}

ASTBreakStmt *
SemaBuilder::CreateBreak(ASTBlock *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLocalVar", Logger()
                        .Attr("Parent", Parent)
                        .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTBreakStmt *Break = new ASTBreakStmt(Parent, Loc);
    return Break;
}

ASTContinueStmt *
SemaBuilder::CreateContinue(ASTBlock *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateContinue", Logger()
                      .Attr("Parent", Parent)
                      .Attr("Loc", (uint64_t)Loc.getRawEncoding()).End());
    ASTContinueStmt *Continue = new ASTContinueStmt(Parent, Loc);
    return Continue;
}

ASTFailStmt *
SemaBuilder::CreateFail(ASTBlock *Block, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateFail", Logger()
            .Attr("Block", Block)
            .Attr("Loc", (uint64_t)Loc.getRawEncoding()).End());
    ASTFailStmt *FailStmt = new ASTFailStmt(Block, Loc);
    return FailStmt;
}

ASTExprStmt *
SemaBuilder::CreateExprStmt(ASTBlock *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateExprStmt", Logger()
                      .Attr("Parent", Parent)
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTExprStmt *ExprStmt = new ASTExprStmt(Parent, Loc);
    return ExprStmt;
}

ASTIdentifier *SemaBuilder::CreateIdentifier(const SourceLocation &Loc, llvm::StringRef Name) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateIdentifier", Logger()
            .Attr("Loc", (uint64_t) Loc.getRawEncoding())
            .Attr("Name", Name)
            .End());
    return new ASTIdentifier(Loc, Name);
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
    ASTCall *Call = new ASTCall(Identifier->getLocation(), Identifier->getName());
    Call->Parent = Identifier->Parent;
    Call->Child = Identifier->Child;
    Call->FullName = Identifier->FullName;
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
SemaBuilder::CreateCall(ASTIdentifier *Instance, ASTFunctionBase *Function) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateCall",
                      Logger().Attr("Function=", Function).End());
    ASTCall *Call = new ASTCall(Function);
    Call->Parent = Instance;
    return Call;
}

ASTVarRef *
SemaBuilder::CreateVarRef(ASTIdentifier *Identifier) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateVarRef",
                      Logger().Attr("Identifier", Identifier).End());
    ASTVarRef *VarRef = new ASTVarRef(Identifier->getLocation(), Identifier->getName());
    VarRef->Parent = Identifier->Parent;
    VarRef->Child = Identifier->Child;
    VarRef->FullName = Identifier->FullName;
    // delete Identifier; TODO
    return VarRef;
}

ASTVarRef *
SemaBuilder::CreateVarRef(ASTVar *Var) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateVarRef",
                      Logger().Attr("Var", Var).End());
    ASTVarRef *VarRef = new ASTVarRef(Var);
    VarRef->Def = Var;
    return VarRef;
}

ASTVarRef *
SemaBuilder::CreateVarRef(ASTIdentifier *Instance, ASTVar *Var) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateVarRef",
                      Logger().Attr("Instance", Instance).Attr("Var", Var).End());
    ASTVarRef *VarRef = new ASTVarRef(Var);
    VarRef->Parent = Instance;
    return VarRef;
}

ASTEmptyExpr *
SemaBuilder::CreateExpr() {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateExpr",
                      Logger().End());
    ASTEmptyExpr *Expr = new ASTEmptyExpr(SourceLocation());
    return Expr;
}

ASTValueExpr *
SemaBuilder::CreateExpr(ASTValue *Value) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateExpr",
                      Logger().Attr("Value", Value).End());
    assert(Value && "Create ASTValueExpr by ASTValue");
    ASTValueExpr *ValueExpr = new ASTValueExpr(Value);
    return ValueExpr;
}

ASTCallExpr *
SemaBuilder::CreateExpr(ASTCall *Call) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateExpr",
                      Logger().Attr("Call", Call).End());
    ASTCallExpr *CallExpr = new ASTCallExpr(Call);
    return CallExpr;
}

ASTVarRefExpr *
SemaBuilder::CreateExpr(ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateExpr",
                      Logger().Attr("VarRef", VarRef).End());
    ASTVarRefExpr *VarRefExpr = new ASTVarRefExpr(VarRef);
    return VarRefExpr;
}

ASTCallExpr *
SemaBuilder::CreateNewExpr(ASTCall *Call) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateNewExpr",
                      Logger().Attr("Call", Call).End());
    Call->CallKind = ASTCallKind::CALL_NEW;
    return CreateExpr(Call);
}

ASTDeleteStmt *
SemaBuilder::CreateDelete(ASTBlock *Parent, const SourceLocation &Loc, ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateDelete",
                      Logger().Attr("Stmt", Parent)
                              .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("VarRef", VarRef).End());
    return new ASTDeleteStmt(Parent, Loc, VarRef);
}

ASTUnaryGroupExpr *
SemaBuilder::CreateUnaryExpr(const SourceLocation &Loc, ASTUnaryOperatorKind Kind,
                             ASTUnaryOptionKind OptionKind, ASTVarRefExpr *First) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateUnaryExpr",
                      Logger()
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                      .Attr("Kind", (uint64_t) Kind)
                      .Attr("OptionKind", (uint64_t) OptionKind)
                      .Attr("First", First).End());

    ASTUnaryGroupExpr *UnaryExpr = new ASTUnaryGroupExpr(Loc, Kind, OptionKind, First);

    // Set Parent Expression
    First->Parent = UnaryExpr;
    return UnaryExpr;
}

ASTBinaryGroupExpr *
SemaBuilder::CreateBinaryExpr(const SourceLocation &OpLoc, ASTBinaryOperatorKind Kind,
                              ASTExpr *First, ASTExpr *Second) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateBinaryExpr", Logger()
                        .Attr("OpLoc", (uint64_t) OpLoc.getRawEncoding())
                        .Attr("Kind", (uint64_t) Kind)
                        .Attr("First", First)
                        .Attr("Second", Second).End());

    ASTBinaryGroupExpr *BinaryExpr = new ASTBinaryGroupExpr(OpLoc, Kind, First, Second);

    // Set Parent Expression
    First->Parent = BinaryExpr;
    Second->Parent = BinaryExpr;
    return BinaryExpr;
}

ASTTernaryGroupExpr *
SemaBuilder::CreateTernaryExpr(ASTExpr *First, const SourceLocation &IfLoc,
                               ASTExpr *Second, const SourceLocation &ElseLoc, ASTExpr *Third) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateTernaryExpr", Logger()
                      .Attr("First", First)
                      .Attr("IfLoc", (uint64_t) IfLoc.getRawEncoding())
                      .Attr("Second", Second)
                      .Attr("Loc", (uint64_t) ElseLoc.getRawEncoding())
                      .Attr("Third", Third).End());

    ASTTernaryGroupExpr *TernaryExpr = new ASTTernaryGroupExpr(First, IfLoc, Second, ElseLoc, Third);

    // Set Parent Expression
    First->Parent = TernaryExpr;
    Second->Parent = TernaryExpr;
    Third->Parent = TernaryExpr;
    return TernaryExpr;
}

ASTBlock*
SemaBuilder::CreateBody(ASTFunctionBase *FunctionBase) {
    FLY_DEBUG("SemaBuilder", "CreateBody");
    FunctionBase->Body = CreateBlock(nullptr, SourceLocation());
    FunctionBase->Body->Top = FunctionBase;
    return FunctionBase->Body;
}

ASTBlock *
SemaBuilder::CreateBlock(ASTBlock *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateBlock", Logger()
                      .Attr("Parent", Parent)
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTBlock *Block = new ASTBlock(Parent, Loc);
    return Block;
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

ASTHandleStmt *
SemaBuilder::CreateHandleStmt(ASTBlock *Parent, const SourceLocation &Loc, ASTVarRef *ErrorRef) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateHandleStmt", Logger()
            .Attr("Parent", Parent)
            .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTHandleStmt *Block = new ASTHandleStmt(Parent, Loc);
    Block->setErrorRef(ErrorRef);
    return Block;
}

/********************** Following Methods Adds AST objects to other AST object ****************************************/

/**
 * Add an ASTNameSpace to the ASTContext if not exists yet
 * @param Name
 * @param ExternLib
 * @return the result of add
 */
bool
SemaBuilder::AddNameSpace(ASTNameSpace *NewNameSpace, ASTNode *Node, bool ExternLib) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddNameSpace",
                      "NameSpace=" << NewNameSpace << ", ExternLib=" << ExternLib);
    // Check if Name exist or add it
    ASTNameSpace *NameSpace = S.Context->NameSpaces.lookup(NewNameSpace->getName());
    if (NameSpace == nullptr) {
        S.Context->NameSpaces.insert(std::make_pair(NewNameSpace->getFullName(), NewNameSpace));
        NameSpace = NewNameSpace;
        if (NewNameSpace->getParent()) {
            return AddNameSpace((ASTNameSpace *) NewNameSpace->getParent());
        }
    }

    if (Node)
        Node->NameSpace = NameSpace;

    return true;
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
    return Node->NameSpace->Nodes.insert(Pair).second && S.Context->Nodes.insert(Pair).second;
}

bool
SemaBuilder::AddImport(ASTNode *Node, ASTImport * Import) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddImport", Logger()
                        .Attr("Node", Node)
                        .Attr("Import", Import).End());

    if (S.Validator->CheckImport(Node, Import)) {

        // Check if this ASTNode already contains the imports
        if (Node->Imports.find(Import->getName()) != Node->Imports.end()) {
            S.Diag(Import->getLocation(), diag::err_conflict_import) << Import->getName();
            return false;
        }

        // Check if this ASTNode already contains the name or alias
        llvm::StringRef Id = (Import->getAlias() == nullptr) ? Import->getName() : Import->getAlias()->getName();
        if (Node->AliasImports.find(Id) != Node->Imports.end()) {
            S.Diag(Import->getLocation(), diag::err_conflict_import) << Id;
            return false;
        }

        // Add Import to Node
        return Node->Imports.insert(std::make_pair(Import->getName(), Import)).second &&
            Node->AliasImports.insert(std::make_pair(Id, Import)).second;
    }

    return false;
}

bool
SemaBuilder::AddIdentity(ASTIdentity *Identity) {
    FLY_DEBUG_MESSAGE("ASTNode", "AddIdentity", Logger()
                      .Attr("Identity", Identity).End());

    // Lookup into namespace
    ASTNode *Node = Identity->Node;
    Node->Identity = Identity;

    bool Success = true;
    if (Identity->Scopes->Visibility == ASTVisibilityKind::V_PUBLIC || Identity->Scopes->Visibility == ASTVisibilityKind::V_DEFAULT) {
        ASTIdentity *LookupClass = Node->NameSpace->getIdentities().lookup(Identity->getName());
        if (LookupClass) { // This NameSpace already contains this Function
            S.Diag(LookupClass->Location, diag::err_duplicate_class) << LookupClass->getName();
            return false;
        }
        Success = Node->NameSpace->Identities.insert(std::make_pair(Identity->getName(), Identity)).second;
    }

    return Success;
}

bool
SemaBuilder::AddGlobalVar(ASTGlobalVar *GlobalVar, ASTValue *Value) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddGlobalVar", Logger()
                      .Attr("GlobalVar", GlobalVar)
                      .Attr("Value", Value).End());
    ASTNode *Node = GlobalVar->Node;

    ASTVarStmt *VarStmt = SemaBuilder::CreateVarStmt(nullptr, GlobalVar);
    ASTValueExpr *ValueExpr = SemaBuilder::CreateExpr(Value);
    VarStmt->setExpr(ValueExpr);
    GlobalVar->setInit(VarStmt);

    // Lookup into namespace for public var
    if(GlobalVar->getScopes()->Visibility == ASTVisibilityKind::V_PUBLIC ||
        GlobalVar->getScopes()->Visibility == ASTVisibilityKind::V_DEFAULT) {
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
    if(GlobalVar->getScopes()->Visibility == ASTVisibilityKind::V_PRIVATE) {
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
        return InsertFunction(Node->NameSpace->Functions, Function) &&
            InsertFunction(Node->Functions, Function);
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
        return Var;
    }

    S.Diag(Var->getLocation(), diag::err_sema_class_field_redeclare) << Var->getName();
    return false;
}

bool
SemaBuilder::AddClassMethod(ASTClassFunction *Method) {
    // Check duplicates
    if (!InsertFunction(Method->Class->Methods, Method)) {
        S.Diag(Method->getLocation(), diag::err_sema_class_method_redeclare) << Method->getName();
        return false;
    }

    return true;
}

bool
SemaBuilder::AddClassConstructor(ASTClassFunction *Constructor) {
    ASTClass *Class = Constructor->Class;

    // Check default constructor
    if (Class->autoDefaultConstructor) {
        Class->Constructors.erase(0); // Remove default constructor if a new Constructor is defined
        Class->autoDefaultConstructor = false;
    }

    // Check duplicates
    if (!InsertFunction(Class->Constructors, Constructor)) {
        S.Diag(Constructor->getLocation(), diag::err_sema_class_method_redeclare) << Constructor->getName();
        return false;
    }

    return true;
}

bool SemaBuilder::AddEnumEntry(ASTEnumEntry *EnumVar) {
    EnumVar->Index = EnumVar->getEnum()->Vars.size() + 1;
    return EnumVar->getEnum()->Vars.insert(std::make_pair(EnumVar->getName(), EnumVar)).second;
}

bool
SemaBuilder::AddParam(ASTFunctionBase *FunctionBase, ASTParam *Param) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "addParam", Logger().Attr("Param", Param).End());

    // Check var duplicates
    if (S.Validator->CheckDuplicateParams(FunctionBase->Params, Param)) {
        FunctionBase->addParam(Param);
        return true;
    }

    return false;
}

void SemaBuilder::AddFunctionVarParams(ASTFunction *Function, ASTParam *Param) {
    Function->setEllipsis(Param);
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
SemaBuilder::AddComment(ASTBase *Base, llvm::StringRef Comment) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddComment", Logger()
                        .Attr("Comment", Comment).End());
    Base->Comment = Comment;
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

bool SemaBuilder::AddStructValue(ASTStructValue *StructValue, llvm::StringRef Key, ASTValue *Value) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddStructValue", Logger()
            .Attr("StructValue", StructValue)
            .Attr("Key", Key).Attr("Value", Value).End());
    return StructValue->Values.insert(std::make_pair(Key, Value)).second;
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
            case ASTStmtKind::STMT_VAR: {
                ASTVarStmt *VarStmt = (ASTVarStmt *) Stmt;
                VarStmt->Expr = Expr;
                VarStmt->getVarRef()->getDef()->setInitialization(VarStmt);
            }
                break;
            case ASTStmtKind::STMT_EXPR:
                ((ASTExprStmt *) Stmt)->Expr = Expr;
                break;
            case ASTStmtKind::STMT_RETURN:
                ((ASTReturnStmt *) Stmt)->Expr = Expr;
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
    bool Success =  false;

    switch (Stmt->getKind()) {

        case ASTStmtKind::STMT_VAR: {
            ASTVarStmt *VarDefine = (ASTVarStmt *) Stmt;

            ASTVar *Var = VarDefine->getVarRef()->getDef();

            // Collects all LocalVars in the hierarchy Block
            if (Var->getVarKind() == ASTVarKind::VAR_LOCAL) {
                ASTLocalVar *LocalVar = (ASTLocalVar *) Var;

                // Statements have an assignment so is initialized
                if (VarDefine->getExpr() != nullptr)
                    LocalVar->setInitialization(VarDefine);

                // Add to the Parent Block
                const auto &Pair = std::pair<std::string, ASTLocalVar *>(LocalVar->getName(), LocalVar);
                if (Parent->LocalVars.insert(Pair).second) {

                    // Useful for Alloca into CodeGen
                    Parent->Top->Body->LocalVars.insert(Pair);
                    Success = true;
                }
            }
        }
            break;

        case ASTStmtKind::STMT_BLOCK:
            Success = AddBlock((ASTBlock *) Stmt);
            break;

        case ASTStmtKind::STMT_EXPR:
        case ASTStmtKind::STMT_BREAK:
        case ASTStmtKind::STMT_CONTINUE:
        case ASTStmtKind::STMT_DELETE:
        case ASTStmtKind::STMT_RETURN:
            Success = true;
            break;
    }

    return Success;
}

bool
SemaBuilder::AddBlock(ASTBlock *Block) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddBlock",
                      Logger().Attr("Stmt", Block).End());

    switch (Block->getBlockKind()) {

        case ASTBlockKind::BLOCK:
            return true;

        case ASTBlockKind::BLOCK_IF:
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
