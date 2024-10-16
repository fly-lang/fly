//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/Sema.cpp - The Sema Builder
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaBuilder.h"

#include <utility>
#include "Sema/SemaBuilderScopes.h"
#include "Sema/SemaBuilderStmt.h"
#include "Sema/SemaBuilderIfStmt.h"
#include "Sema/SemaBuilderSwitchStmt.h"
#include "Sema/SemaBuilderLoopStmt.h"
#include "Sema/Sema.h"
#include "Sema/SemaResolver.h"
#include "Sema/SemaValidator.h"
#include "CodeGen/CodeGen.h"
#include "AST/ASTContext.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTModule.h"
#include "AST/ASTImport.h"
#include "AST/ASTArg.h"
#include "AST/ASTComment.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTBreakStmt.h"
#include "AST/ASTContinueStmt.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTDeleteStmt.h"
#include "AST/ASTParam.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTIfStmt.h"
#include "AST/ASTLoopStmt.h"
#include "AST/ASTSwitchStmt.h"
#include "AST/ASTHandleStmt.h"
#include "AST/ASTVarStmt.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTValue.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassAttribute.h"
#include "AST/ASTEnum.h"
#include "AST/ASTEnumType.h"
#include "AST/ASTEnumEntry.h"
#include "AST/ASTExpr.h"
#include "AST/ASTGroupExpr.h"
#include "AST/ASTOperatorExpr.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTClassMethod.h"
#include "AST/ASTFailStmt.h"
#include "Basic/SourceLocation.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"

using namespace fly;

/**
 * Private constructor used only from Sema constructor
 * @param S
 */
SemaBuilder::SemaBuilder(Sema &S) : S(S) {
    FLY_DEBUG("SemaBuilder", "SemaBuilder");
}

ASTContext *SemaBuilder::CreateContext() {
    FLY_DEBUG("SemaBuilder", "CreateContext");
    S.Context = new ASTContext();
    return S.Context;
}

/**
 * Creates an ASTModule
 * If NameSpace doesn't exists it will be created
 * @param Name
 * @param NameSpace
 * @return the ASTModule
 */
ASTModule *SemaBuilder::CreateModule(const std::string &Name) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "GenerateModule", "Name=" << Name);
    S.getValidator().CheckCreateModule(Name);
    uint64_t Id = S.Context->getNextModuleId();
    ASTModule *Module = new ASTModule(Id, Name, S.Context, false);
    S.Context->Modules.push_back(Module);
    return Module;
}

/**
 * Creates an ASTHeaderModule: only prototype declarations without definitions
 * For .fly.h file generation
 * @param Name
 * @param NameSpace
 * @return thee ASTHeaderModule
 */
ASTModule *SemaBuilder::CreateHeaderModule(const std::string &Name) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateHeaderModule", "Name=" << Name);
    uint64_t Id = S.Context->getNextModuleId();
    return new ASTModule(Id, Name, S.Context, true);
}

/**
 * Create a NameSpace for each parent
 * NS3.NS2.NS1 -> Identifier->Parent
 * NS2.NS1 -> Identifier->Parent->Parent
 * NS3 -> Identifier->Parent->Parent->Parent ... until to Root
 * @param Identifier
 * @return
 */
ASTNameSpace *SemaBuilder::CreateNameSpace(ASTIdentifier *Identifier, ASTModule *Module) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateNameSpace", "Identifier=" << Identifier->str());

    S.getValidator().CheckCreateNameSpace(Identifier->getLocation(), Identifier->getName());
    ASTNameSpace *NameSpace = new ASTNameSpace(Identifier->getLocation(), Identifier->getName());

    if (Module)
        Module->NameSpaces.push_back(NameSpace);

    // Iterate over parents
    if (Identifier->getParent() != nullptr) {
        NameSpace->Parent = CreateNameSpace(Identifier->getParent());
    }

    return NameSpace;
}

ASTImport *SemaBuilder::CreateImport(ASTModule *Module, const SourceLocation &Loc, llvm::StringRef Name, ASTAlias *Alias) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateImport",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Name", Name).End());
    S.getValidator().CheckCreateImport(Loc, Name);
    ASTImport *Import = new ASTImport(Loc, Name);
    // Add Import to Module
    Module->Imports.push_back(Import);

    if (Alias) {
        Import->setAlias(Alias);
        Module->AliasImports.push_back(Import);
    }

    return Import;
}

ASTAlias *SemaBuilder::CreateAlias(const SourceLocation &Loc, llvm::StringRef Name) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateImport",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Name", Name).End());
    S.getValidator().CheckCreateAlias(Loc, Name);
    ASTAlias *Alias = new ASTAlias(Loc, Name);
    return Alias;
}

ASTComment *SemaBuilder::CreateComment(const SourceLocation &Loc, llvm::StringRef Content) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateComment",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Content", Content).End());
    ASTComment *Comment = new ASTComment(Loc, Content);
    return Comment;
}

/**
 * Creates an ASTGlobalVar
 * @param Module
 * @param Loc
 * @param Type
 * @param Name
 * @param Scopes
 * @return
 */
ASTGlobalVar *SemaBuilder::CreateGlobalVar(ASTModule *Module, const SourceLocation &Loc, ASTType *Type,
                                           const llvm::StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes,
                                           ASTExpr *Expr, ASTComment *Comment) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateGlobalVar",
                      Logger()
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                      .Attr("Type", Type)
                      .Attr("Name", Name)
                      .AttrList("Scopes", Scopes).End());
    S.getValidator().CheckCreateGlobalVar(Loc, Type, Name, Scopes);
    ASTGlobalVar *GlobalVar = new ASTGlobalVar(Module, Loc, Type, Name, Scopes);
    GlobalVar->Comment = Comment;
    GlobalVar->Expr = Expr;
    Module->GlobalVars.push_back(GlobalVar);
    return GlobalVar;
}

/**
 * Creates an ASTFunction
 * @param Module
 * @param Loc
 * @param Type
 * @param Name
 * @param Scopes
 * @return
 */
ASTFunction *SemaBuilder::CreateFunction(ASTModule *Module, const SourceLocation &Loc, ASTType *Type,
                                         llvm::StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes,
                                         SmallVector<ASTParam *, 8> &Params, ASTBlockStmt *Body, ASTComment *Comment) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateFunction",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Type", Type)
                              .Attr("Name", Name)
                              .AttrList("Scopes", Scopes).End());
    S.getValidator().CheckCreateFunction(Loc, Type, Name, Scopes);
    ASTFunction *Function = new ASTFunction(Module, Loc, Type, Name, Scopes, Params);
    Function->Comment = Comment;

    // Create Error handler
    Function->ErrorHandler = CreateErrorHandlerParam();

    // Create Body
    if (Body)
        CreateBody(Function, Body);

    Module->Functions.push_back(Function);
    return Function;
}

/**
 * Creates an ASTClass
 * @param Module
 * @param Loc
 * @param Name
 * @param Scopes
 * @return
 */
ASTClass *SemaBuilder::CreateClass(ASTModule *Module, const SourceLocation &Loc, ASTClassKind ClassKind,
                                   const llvm::StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes,
                                   llvm::SmallVector<ASTClassType *, 4> &ClassTypes, ASTComment *Comment) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClass",
                      Logger().Attr("ClassKind", (uint64_t) ClassKind)
                              .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Name", Name)
                              .AttrList("Scopes", Scopes)
                              .End());
    S.getValidator().CheckCreateClass(Loc, Name, ClassKind, Scopes, ClassTypes);
    ASTClass *Class = new ASTClass(Module, ClassKind, Scopes, Loc, Name, ClassTypes);
    Class->Comment = Comment;
    Class->Type = CreateClassType(Class);

    // Lookup into namespace
    Module->Identities.push_back(Class);

    // Create Default Constructor
    llvm::SmallVector<ASTParam *, 8> Params;
    ASTBlockStmt *Body = CreateBlockStmt(SourceLocation());
    Class->DefaultConstructor = S.Builder->CreateClassConstructor(SourceLocation(), *Class, Scopes, Params, Body);

    return Class;
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
ASTClassAttribute *SemaBuilder::CreateClassAttribute(const SourceLocation &Loc, ASTClass &Class, ASTType *Type,
                                                     llvm::StringRef Name, SmallVector<ASTScope *, 8> &Scopes,
                                                     ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassAttribute",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                                .Attr("Type", Type)
                                .Attr("Name", Name)
                                .AttrList("Scopes", Scopes).End());
    S.getValidator().CheckCreateClassVar(Loc, Name, Type, Scopes);
    ASTClassAttribute *Attribute = new ASTClassAttribute(Loc, Class, Type, Name, Scopes);
    Attribute->Expr = Expr;
    Class.Attributes.push_back(Attribute);
    return Attribute;
}

ASTClassMethod *SemaBuilder::CreateClassConstructor(const SourceLocation &Loc, ASTClass &Class,
                                                    llvm::SmallVector<ASTScope *, 8> &Scopes,
                                                    llvm::SmallVector<ASTParam *, 8> &Params, ASTBlockStmt *Body) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassConstructor",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .AttrList("Scopes", Scopes)
                              .End());
    S.getValidator().CheckCreateClassConstructor(Loc, Scopes);
    ASTClassMethod *Constructor = new ASTClassMethod(Loc, ASTClassMethodKind::METHOD_CONSTRUCTOR, CreateVoidType(Loc),
                                                     Class.getName(), Scopes, Params);

    // Set Error Handler
    Constructor->ErrorHandler = CreateErrorHandlerParam();

    if (Body)
        CreateBody(Constructor, Body);

    // Set Constructor Class
    Constructor->Class = &Class;

    // Remove Default Constructor
    if (Class.DefaultConstructor == nullptr) {
        delete Class.DefaultConstructor;
        Class.DefaultConstructor = nullptr;
        Class.Constructors.clear();
    }

    // Add to Class Constructors
    Class.Constructors.push_back(Constructor);

    return Constructor;
}

ASTClassMethod *SemaBuilder::CreateClassMethod(const SourceLocation &Loc, ASTClass &Class, ASTType *Type,
                                               llvm::StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes,
                                               llvm::SmallVector<ASTParam *, 8> &Params, ASTBlockStmt *Body) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassMethod",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Type", Type)
                              .Attr("Name=", Name)
                              .AttrList("Scopes", Scopes)
                              .End());
    S.getValidator().CheckCreateClassMethod(Loc, Type, Name, Scopes);
    ASTClassMethod *Method = new ASTClassMethod(Loc, ASTClassMethodKind::METHOD, Type, Name, Scopes, Params);

    // Set Error Handler
    Method->ErrorHandler = CreateErrorHandlerParam();

    if (Body)
        CreateBody(Method, Body);

    // Set Constructor Class
    Method->Class = &Class;

    // Add to Class Methods
    Class.Methods.push_back(Method);

    return Method;
}

ASTClassMethod *SemaBuilder::CreateClassVirtualMethod(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                                                      llvm::SmallVector<ASTScope *, 8> &Scopes,
                                                      llvm::SmallVector<ASTParam *, 8> &Params) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateAbstractClassMethod",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Type", Type)
                              .Attr("Name=", Name)
                              .AttrList("Scopes", Scopes)
                              .End());
    S.getValidator().CheckCreateClassMethod(Loc, Type, Name, Scopes);
    ASTClassMethod *VirtualMethod = new ASTClassMethod(Loc, ASTClassMethodKind::METHOD_VIRTUAL, Type, Name, Scopes, Params);
    return VirtualMethod;
}

ASTEnum *SemaBuilder::CreateEnum(ASTModule *Module, const SourceLocation &Loc, const llvm::StringRef Name,
                                 llvm::SmallVector<ASTScope *, 8> &Scopes,
                                 llvm::SmallVector<ASTEnumType *, 4> EnumTypes, ASTComment *Comment) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateEnum",
                      Logger().AttrList("Scopes", Scopes).Attr("Name", Name).End());
    S.getValidator().CheckCreateEnum(Loc, Name, Scopes, EnumTypes);
    ASTEnum *Enum = new ASTEnum(Module, Loc, Name, Scopes, EnumTypes);
    Enum->Type = CreateEnumType(Enum);
    Enum->Comment = Comment;

    // Lookup into namespace
    Module->Identities.push_back(Enum);

    return Enum;
}

ASTEnumEntry *SemaBuilder::CreateEnumEntry(const SourceLocation &Loc, ASTEnum &Enum, llvm::StringRef Name,
                                           llvm::SmallVector<ASTScope *, 8> &Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateEnumEntry",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Name", Name).End());
    S.getValidator().CheckCreateEnumEntry(Loc, Name);
    ASTEnumEntry *EnumEntry = new ASTEnumEntry(Loc, Enum, Name, Scopes);
    EnumEntry->Index = Enum.Entries.size() + 1;
    Enum.Entries.push_back(EnumEntry);
    return EnumEntry;
}

/**
 * Creates a bool type
 * @param Loc
 * @return
 */
ASTBoolType *SemaBuilder::CreateBoolType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateBoolType", "Loc=" << Loc.getRawEncoding());
    return new ASTBoolType(Loc);
}

/**
 * Creates an byte type
 * @param Loc
 * @return
 */
ASTByteType *SemaBuilder::CreateByteType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateByteType", "Loc=" << Loc.getRawEncoding());
    return new ASTByteType(Loc);
}

/**
 * Creates an unsigned short type
 * @param Loc
 * @return
 */
ASTUShortType *SemaBuilder::CreateUShortType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateUShortType", "Loc=" << Loc.getRawEncoding());
    return new ASTUShortType(Loc);
}

/**
 * Create a short type
 * @param Loc
 * @return
 */
ASTShortType *SemaBuilder::CreateShortType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateShortType", "Loc=" << Loc.getRawEncoding());
    return new ASTShortType(Loc);;
}

/**
 * Creates an unsigned int type
 * @param Loc
 * @return
 */
ASTUIntType *SemaBuilder::CreateUIntType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateUIntType", "Loc=" << Loc.getRawEncoding());
    return new ASTUIntType(Loc);
}

/**
 * Creates an int type
 * @param Loc
 * @return
 */
ASTIntType *SemaBuilder::CreateIntType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateIntType", "Loc=" << Loc.getRawEncoding());
    return new ASTIntType(Loc);
}

/**
 * Creates an unsigned long type
 * @param Loc
 * @return
 */
ASTULongType *SemaBuilder::CreateULongType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateULongType", "Loc=" << Loc.getRawEncoding());
    return new ASTULongType(Loc);
}

/**
 * Creates a long type
 * @param Loc
 * @return
 */
ASTLongType *SemaBuilder::CreateLongType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLongType", "Loc=" << Loc.getRawEncoding());
    return new ASTLongType(Loc);
}

/**
 * Creates a float type
 * @param Loc
 * @return
 */
ASTFloatType *SemaBuilder::CreateFloatType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateFloatType", "Loc=" << Loc.getRawEncoding());
    return new ASTFloatType(Loc);
}

/**
 * Creates a double type
 * @param Loc
 * @return
 */
ASTDoubleType *SemaBuilder::CreateDoubleType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateDoubleType", "Loc=" << Loc.getRawEncoding());
    return new ASTDoubleType(Loc);
}

/**
 * Creates a void type
 * @param Loc
 * @return
 */
ASTVoidType *SemaBuilder::CreateVoidType(const SourceLocation &Loc) {
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
ASTArrayType *SemaBuilder::CreateArrayType(const SourceLocation &Loc, ASTType *Type, ASTExpr *Size) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateArrayType",
                      Logger()
                        .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                        .Attr("Type", Type)
                        .Attr("Size", Size).End());
    return new ASTArrayType(Loc, Type, Size);
}

ASTStringType *SemaBuilder::CreateStringType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateStringType", "Loc=" << Loc.getRawEncoding());
    return new ASTStringType(Loc);
}

/**
 * Creates a class type without definition
 * @param Identifier
 * @return
 */
ASTClassType *SemaBuilder::CreateClassType(ASTIdentifier *Identifier) {
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
ASTClassType *SemaBuilder::CreateClassType(ASTClass *Class) {
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
ASTEnumType *SemaBuilder::CreateEnumType(ASTIdentifier *Identifier) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateEnumType",
                      Logger().Attr("Identifier", Identifier).End());
    ASTEnumType *EnumType = new ASTEnumType(Identifier);
    return EnumType;
}

ASTIdentityType *SemaBuilder::CreateIdentityType(ASTIdentifier *Identifier) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateIdentityType",
                      Logger().Attr("Identifier", Identifier).End());
    return new ASTIdentityType(Identifier);
}

ASTErrorType *SemaBuilder::CreateErrorType(const SourceLocation &Loc) {
    FLY_DEBUG("SemaBuilder", "CreateErrorType");
    return new ASTErrorType(Loc);
}

/**
 * Creates an enum type with definition
 * @param Enum
 * @return
 */
ASTEnumType *SemaBuilder::CreateEnumType(ASTEnum *Enum) {
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
ASTNullValue *SemaBuilder::CreateNullValue(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateNullValue", "Loc=" << Loc.getRawEncoding());
    return new ASTNullValue(Loc);
}

/**
 * Creates a zero value
 * @param Loc
 * @return
 */
ASTZeroValue *SemaBuilder::CreateZeroValue(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateNullValue", "Loc=" << Loc.getRawEncoding());
    return new ASTZeroValue(Loc);
}

/**
 * Creates a bool value
 * @param Loc
 * @param Val
 * @return
 */
ASTBoolValue *SemaBuilder::CreateBoolValue(const SourceLocation &Loc, bool Val) {
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
ASTIntegerValue *SemaBuilder::CreateIntegerValue(const SourceLocation &Loc, uint64_t Val, bool Negative) {
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
ASTFloatingValue *SemaBuilder::CreateFloatingValue(const SourceLocation &Loc, std::string Val) {
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
ASTFloatingValue *SemaBuilder::CreateFloatingValue(const SourceLocation &Loc, double Val) {
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
ASTArrayValue *SemaBuilder::CreateArrayValue(const SourceLocation &Loc, llvm::SmallVector<ASTValue *, 8> Values) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateArrayValue",
                      "Loc=" << Loc.getRawEncoding());
    ASTArrayValue *Array = new ASTArrayValue(Loc);
    Array->Values = std::move(Values);
    return Array;
}

ASTStringValue *SemaBuilder::CreateStringValue(const SourceLocation &Loc, llvm::StringRef Str) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateStringValue",
                      "Loc=" << Loc.getRawEncoding());
    return new ASTStringValue(Loc, Str);
}

ASTStructValue *SemaBuilder::CreateStructValue(const SourceLocation &Loc, llvm::StringMap<ASTValue *> Values) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateArrayValue",
                      "Loc=" << Loc.getRawEncoding());
    ASTStructValue *Struct = new ASTStructValue(Loc);
    Struct->Values = std::move(Values);
    return Struct;
}

/**
 * Creates a default value by type
 * @param Type
 * @return
 */
ASTValue *SemaBuilder::CreateDefaultValue(ASTType *Type) {
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
        llvm::SmallVector<ASTValue *, 8> Values;
        Value = CreateArrayValue(Type->getLocation(), Values);
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
ASTParam *SemaBuilder::CreateParam(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                                   llvm::SmallVector<ASTScope *, 8> &Scopes, ASTValue *DefaultValue) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateParam",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                      .Attr("Type", Type)
                      .Attr("Name", Name)
                      .AttrList("Scopes", Scopes)
                      .End());
    S.getValidator().CheckCreateParam(Loc, Type, Name, Scopes);
    ASTParam *Param = new ASTParam(Loc, Type, Name, Scopes);
    Param->DefaultValue = DefaultValue;
    return Param;
}

ASTParam *SemaBuilder::CreateErrorHandlerParam() {
    SmallVector<ASTScope *, 8> Scopes = SemaBuilderScopes::Create()->getScopes();
    return CreateParam(SourceLocation(), CreateErrorType(SourceLocation()), "error", Scopes);
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
ASTLocalVar *SemaBuilder::CreateLocalVar(ASTBlockStmt *BlockStmt, const SourceLocation &Loc, ASTType *Type,
                                         llvm::StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLocalVar",
                      Logger().Attr("Name", Name)
                              .AttrList("Scopes", Scopes).End());
    S.getValidator().CheckCreateLocalVar(Loc, Type, Name, Scopes);
    ASTLocalVar *LocalVar = new ASTLocalVar(Loc, Type, Name, Scopes);
    BlockStmt->Function->LocalVars.push_back(LocalVar); // Function Local var to be allocated
    BlockStmt->LocalVars.insert(std::make_pair(LocalVar->getName(), LocalVar)); // Check duplicate in Block Stmt
    return LocalVar;
}

ASTIdentifier *SemaBuilder::CreateIdentifier(const SourceLocation &Loc, llvm::StringRef Name) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateIdentifier", Logger()
            .Attr("Loc", (uint64_t) Loc.getRawEncoding())
            .Attr("Name", Name)
            .End());
    S.getValidator().CheckCreateIdentifier(Loc, Name);
    ASTIdentifier *Identifier = new ASTIdentifier(Loc, Name);
    return Identifier;
}

/**
 * Create an ASTFunctionCall without definition
 * @param Location
 * @param Name
 * @param NameSpace
 * @return
 */
ASTCall *SemaBuilder::CreateCall(ASTIdentifier *Identifier, llvm::SmallVector<ASTExpr *, 8> &Args, ASTCallKind CallKind,
                                 ASTIdentifier *Parent) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateCall",
                      Logger().Attr("Identifier", Identifier).End());
    ASTCall *Call = new ASTCall(Identifier->getLocation(), Identifier->getName());
    Call->CallKind = CallKind;
    if (Parent) { // Take Parent
        Parent->AddChild(Call);
    } else { // Do a copy
        Call->Parent = Identifier->Parent;
        Call->Child = Identifier->Child;
        Call->FullName = Identifier->FullName;
    }
    uint64_t i = 0;
    for (auto &Expr : Args) {
        ASTArg *Arg = new ASTArg(Expr, i++);
        Call->Args.push_back(Arg);
    }
    return Call;
}

/**
 * Creates an ASTFunctionCall with definition
 * @param Stmt
 * @param Function
 * @return
 */
ASTCall *SemaBuilder::CreateCall(ASTFunction *Function, llvm::SmallVector<ASTExpr *, 8> &Args) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateCall",
                      Logger().Attr("Function=", Function).End());
    ASTIdentifier *Identifier = CreateIdentifier(SourceLocation(), Function->Name);
    ASTCall *Call = CreateCall(Identifier, Args, ASTCallKind::CALL_FUNCTION);
    Call->Def = Function;
    Call->Resolved = true;
    return Call;
}

ASTCall *SemaBuilder::CreateCall(ASTClassMethod *Method, ASTCallKind CallKind) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateCall",
                      Logger().Attr("Method=", Method).End());
    ASTCall *Call = new ASTCall(SourceLocation(), Method->Name);
    Call->CallKind = CallKind;
    Call->Def = Method;
    return Call;
}

ASTCall *SemaBuilder::CreateCall(ASTIdentifier *Instance, ASTClassMethod *Method) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateCall",
                      Logger().Attr("Function=", Method).End());
    ASTCall *Call = new ASTCall(SourceLocation(), Method->getName());
    Call->Parent = Instance;
    Call->Def = Method;
    return Call;
}

ASTVarRef *SemaBuilder::CreateVarRef(ASTIdentifier *Identifier, ASTIdentifier *Parent) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateVarRef",
                      Logger().Attr("Identifier", Identifier).End());
    ASTVarRef *VarRef = new ASTVarRef(Identifier->getLocation(), Identifier->getName());
    if (Parent) { // Take Parent
        Parent->AddChild(VarRef);
    } else { // Do a copy
        VarRef->Parent = Identifier->Parent;
        VarRef->Child = Identifier->Child;
        VarRef->FullName = Identifier->FullName;
    }
    // delete Identifier; TODO
    return VarRef;
}

ASTVarRef *SemaBuilder::CreateVarRef(ASTVar *Var) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateVarRef",
                      Logger().Attr("Var", Var).End());
    ASTVarRef *VarRef = new ASTVarRef(Var->getLocation(), Var->getName());
    VarRef->Resolved = true;
    VarRef->Def = Var;
    return VarRef;
}

ASTEmptyExpr *SemaBuilder::CreateExpr() {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateExpr",
                      Logger().End());
    ASTEmptyExpr *Expr = new ASTEmptyExpr(SourceLocation());
    return Expr;
}

ASTValueExpr *SemaBuilder::CreateExpr(ASTValue *Value) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateExpr",
                      Logger().Attr("Value", Value).End());
    assert(Value && "Create ASTValueExpr by ASTValue");
    ASTValueExpr *Expr = new ASTValueExpr(Value);
    const SourceLocation &Loc = Value->getLocation();

    switch (Value->getTypeKind()) {

        case ASTTypeKind::TYPE_BOOL:
            Expr->Type = CreateBoolType(Loc);
            break;

        case ASTTypeKind::TYPE_INTEGER: {
            ASTIntegerValue *Integer = ((ASTIntegerValue *) Expr->Value);

            if (Integer->Negative) { // Integer is negative (Ex. -2)

                if (Integer->Value > MIN_LONG) { // Negative Integer overflow min value
                    S.Diag(Expr->getLocation(), diag::err_sema_int_min_overflow);
                    return Expr;
                }

                if (Integer->Value > MIN_INT) {
                    Expr->Type = CreateLongType(Loc);
                } else if (Integer->Value > MIN_SHORT) {
                    Expr->Type = CreateIntType(Loc);
                } else {
                    Expr->Type = CreateShortType(Loc);
                }
            } else { // Positive Integer

                if (Integer->Value > MAX_LONG) { // Positive Integer overflow max value
                    S.Diag(Expr->getLocation(), diag::err_sema_int_max_overflow);
                    return Expr;
                }

                if (Integer->Value > MAX_INT) {
                    Expr->Type = CreateLongType(Loc);
                } else if (Integer->Value > MAX_SHORT) {
                    Expr->Type = CreateIntType(Loc);
                } else if (Integer->Value > MAX_BYTE) {
                    Expr->Type = CreateShortType(Loc);
                } else {
                    Expr->Type = CreateByteType(Loc);
                }
            }
            break;
        }

        case ASTTypeKind::TYPE_FLOATING_POINT:
            // Creating as Float on first but transform in Double if is contained into a Binary Expr with a Double Type
            Expr->Type = CreateDoubleType(Loc);
            break;

        case ASTTypeKind::TYPE_STRING:
            Expr->Type = CreateStringType(Loc);
            break;

        case ASTTypeKind::TYPE_ARRAY:
            // TODO
            break;
        case ASTTypeKind::TYPE_IDENTITY:
            // TODO
            break;
        case ASTTypeKind::TYPE_VOID:
            break;
        case ASTTypeKind::TYPE_ERROR:
            break;
    }
    return Expr;
}

ASTCallExpr *SemaBuilder::CreateExpr(ASTCall *Call) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateExpr",
                      Logger().Attr("Call", Call).End());
    ASTCallExpr *CallExpr = new ASTCallExpr(Call);
    return CallExpr;
}

ASTVarRefExpr *SemaBuilder::CreateExpr(ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateExpr",
                      Logger().Attr("VarRef", VarRef).End());
    ASTVarRefExpr *VarRefExpr = new ASTVarRefExpr(VarRef);
    return VarRefExpr;
}

ASTUnaryOperatorExpr *SemaBuilder::CreateOperatorExpr(const SourceLocation &Loc, ASTUnaryOperatorKind UnaryKind) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateOperatorExpr", Logger()
            .Attr("Loc", (uint64_t) Loc.getRawEncoding())
            .Attr("Kind", (uint64_t) UnaryKind)
            .End());
    return new ASTUnaryOperatorExpr(Loc, UnaryKind);
}

ASTBinaryOperatorExpr *SemaBuilder::CreateOperatorExpr(const SourceLocation &Loc, ASTBinaryOperatorKind BinaryKind) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateOperatorExpr", Logger()
            .Attr("Loc", (uint64_t) Loc.getRawEncoding())
            .Attr("Kind", (uint64_t) BinaryKind)
            .End());
    return new ASTBinaryOperatorExpr(Loc, BinaryKind);
}

ASTTernaryOperatorExpr *SemaBuilder::CreateOperatorExpr(const SourceLocation &Loc, ASTTernaryOperatorKind TernaryKind) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateOperatorExpr", Logger()
            .Attr("Loc", (uint64_t) Loc.getRawEncoding())
            .Attr("Kind", (uint64_t) TernaryKind)
            .End());
    return new ASTTernaryOperatorExpr(Loc, TernaryKind);
}

ASTUnaryGroupExpr *SemaBuilder::CreateUnaryExpr(ASTUnaryOperatorExpr *Operator, ASTVarRefExpr *First) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateUnaryExpr",
                      Logger()
                      .Attr("Operator", Operator)
                      .Attr("First", First)
                      .End());
    ASTUnaryGroupExpr *UnaryExpr = new ASTUnaryGroupExpr(Operator->getLocation(), Operator, First);
    return UnaryExpr;
}

ASTBinaryGroupExpr *SemaBuilder::CreateBinaryExpr(ASTBinaryOperatorExpr *Operator,
                                                  ASTExpr *First, ASTExpr *Second) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateBinaryExpr", Logger()
                        .Attr("Operator", Operator)
                        .Attr("First", First)
                        .Attr("Second", Second).End());

    ASTBinaryGroupExpr *BinaryExpr = new ASTBinaryGroupExpr(First->getLocation(), Operator, First, Second);
    return BinaryExpr;
}

ASTTernaryGroupExpr *SemaBuilder::CreateTernaryExpr(ASTExpr *First,
                                                    ASTTernaryOperatorExpr *FirstOperator, ASTExpr *Second,
                                                    ASTTernaryOperatorExpr *SecondOperator, ASTExpr *Third) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateTernaryExpr", Logger()
                      .Attr("First", First)
                      .Attr("FirstOperator", FirstOperator)
                      .Attr("Second", Second)
                      .Attr("SecondOperator", SecondOperator)
                      .Attr("Third", Third).End());

    ASTTernaryGroupExpr *TernaryExpr = new ASTTernaryGroupExpr(First->getLocation(),
                                                               First, FirstOperator, Second, SecondOperator, Third);

    return TernaryExpr;
}

/**
 * Creates a SemaBuilderStmt with an ASTVarAssign from ASTVarRef
 * @param Parent
 * @param VarRef
 * @return
 */
SemaBuilderStmt *SemaBuilder::CreateVarStmt(ASTBlockStmt *Parent, ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateVarStmt",
                      Logger().Attr("VarRef", VarRef).End());
    return SemaBuilderStmt::CreateVar(this, Parent, VarRef);
}

/**
 * Creates a SemaBuilderStmt with an ASTVarAssign from ASTVar
 * @param Parent
 * @param VarRef
 * @return
 */
SemaBuilderStmt *SemaBuilder::CreateVarStmt(ASTBlockStmt *Parent, ASTVar *Var) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateVarStmt",
                      Logger().Attr("Var", Var).End());
    return SemaBuilderStmt::CreateVar(this, Parent, Var);
}

/**
 * Creates a SemaBuilderStmt with ASTReturn
 * @param Parent
 * @param Loc
 * @return
 */
SemaBuilderStmt *SemaBuilder::CreateReturnStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLocalVar", Logger()
            .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    return SemaBuilderStmt::CreateReturn(this, Parent, Loc);
}

/**
 * Creates an SemaBuilderStmt with ASTFailStmt
 * @param Loc
 * @param ErrorHandler
 * @return
 */
SemaBuilderStmt *SemaBuilder::CreateFailStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateFailStmt", Logger()
            .Attr("Loc", (uint64_t)Loc.getRawEncoding()).End());
    return SemaBuilderStmt::CreateFail(this, Parent, Loc);
}

SemaBuilderStmt *SemaBuilder::CreateExprStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateExprStmt", Logger()
            .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    return SemaBuilderStmt::CreateExpr(this, Parent, Loc);
}

ASTBreakStmt *SemaBuilder::CreateBreakStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLocalVar", Logger()
            .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTBreakStmt *Break = new ASTBreakStmt(Loc);
    // Inner Stmt
    Parent->Content.push_back(Break);
    Break->Parent = Parent;
    Break->Function = Parent->Function;
    return Break;
}

ASTContinueStmt *SemaBuilder::CreateContinueStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateContinueStmt", Logger()
            .Attr("Loc", (uint64_t)Loc.getRawEncoding()).End());
    ASTContinueStmt *Continue = new ASTContinueStmt(Loc);
    // Inner Stmt
    Parent->Content.push_back(Continue);
    Continue->Parent = Parent;
    Continue->Function = Parent->Function;
    return Continue;
}

ASTDeleteStmt *SemaBuilder::CreateDeleteStmt(ASTBlockStmt *Parent, const SourceLocation &Loc, ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateDeleteStmt",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("VarRef", VarRef).End());
    ASTDeleteStmt *Delete = new ASTDeleteStmt(Loc, VarRef);
    // Inner Stmt
    Parent->Content.push_back(Delete);
    Delete->Parent = Parent;
    Delete->Function = Parent->Function;
    return Delete;
}

ASTBlockStmt*
SemaBuilder::CreateBody(ASTFunctionBase *FunctionBase, ASTBlockStmt *Body) {
    FLY_DEBUG("SemaBuilder", "CreateBody");
    Body->Parent = nullptr; // body cannot have a parent stmt
    FunctionBase->Body = Body;
    FunctionBase->Body->Function = FunctionBase;
    return FunctionBase->Body;
}

ASTBlockStmt *SemaBuilder::CreateBlockStmt(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateBlockStmt", Logger()
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTBlockStmt *Block = new ASTBlockStmt(Loc);
    return Block;
}

ASTBlockStmt *SemaBuilder::CreateBlockStmt(ASTStmt *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateBlockStmt", Logger()
            .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTBlockStmt *Block = new ASTBlockStmt(Loc);
    Block->Parent = Parent;
    return Block;
}

SemaBuilderIfStmt *SemaBuilder::CreateIfBuilder(ASTBlockStmt *Parent) {
    FLY_DEBUG("SemaBuilder", "CreateIfBuilder");
    return SemaBuilderIfStmt::Create(S, Parent);
}

SemaBuilderSwitchStmt *SemaBuilder::CreateSwitchBuilder(ASTBlockStmt *Parent) {
    FLY_DEBUG("SemaBuilder", "CreateSwitchBuilder");
    return SemaBuilderSwitchStmt::Create(S, Parent);
}

SemaBuilderLoopStmt *SemaBuilder::CreateLoopBuilder(ASTBlockStmt *Parent, const SourceLocation &Loc) {
    FLY_DEBUG("SemaBuilder", "CreateLoopBuilder");
    return SemaBuilderLoopStmt::Create(S, Parent, Loc);
}

ASTHandleStmt *SemaBuilder::CreateHandleStmt(ASTBlockStmt *Parent, const SourceLocation &Loc, ASTVarRef *ErrorRef) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateHandleStmt", Logger()
            .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTHandleStmt *HandleStmt = new ASTHandleStmt(Loc);
    HandleStmt->setErrorHandlerRef(ErrorRef);
    // Inner Stmt
    HandleStmt->Parent = Parent;
    HandleStmt->Function = Parent->Function;
    return HandleStmt;
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
