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
#include "AST/ASTModule.h"
#include "AST/ASTImport.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTBreakStmt.h"
#include "AST/ASTContinueStmt.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTFunctionBase.h"
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
    S.Context->DefaultNameSpace = CreateDefaultNameSpace();
    S.Context->NameSpaces.insert(std::make_pair(S.Context->DefaultNameSpace->getName(), S.Context->DefaultNameSpace));
    return S.Context;
}

std::string SemaBuilder::DEFAULT = "default";

ASTNameSpace *SemaBuilder::CreateDefaultNameSpace() {
    FLY_DEBUG("SemaBuilder", "CreateDefaultNameSpace");
    const SourceLocation &Loc = SourceLocation();
    return new ASTNameSpace(SourceLocation(), DEFAULT, S.Context);
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
ASTNameSpace *SemaBuilder::CreateNameSpace(ASTModule *Module, ASTIdentifier *Identifier) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateNameSpace", "Identifier=" << Identifier->str());

    S.getValidator().CheckCreateNameSpace(Identifier->getLocation(), Identifier->getName());
    Module->NameSpace = new ASTNameSpace(Identifier->getLocation(), Identifier->getName(), S.Context);

    // Iterate over parents
    if (Identifier->getParent() != nullptr) {
        Module->NameSpace->Parent = CreateNameSpace(Module, Identifier->getParent());
    }

    return Module->NameSpace;
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

ASTAlias *SemaBuilder::CreateAlias(ASTImport *Import, const SourceLocation &Loc, llvm::StringRef Name) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateImport",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Name", Name).End());
    S.getValidator().CheckCreateAlias(Loc, Name);
    ASTAlias *Alias = new ASTAlias(Loc, Name);
    return Alias;
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
                                           ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateGlobalVar",
                      Logger()
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                      .Attr("Type", Type)
                      .Attr("Name", Name)
                      .AttrList("Scopes", Scopes).End());
    S.getValidator().CheckCreateGlobalVar(Loc, Type, Name, Scopes);
    ASTGlobalVar *GlobalVar = new ASTGlobalVar(Module, Loc, Type, Name, Scopes);
    GlobalVar->Expr = Expr;
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
                                         const llvm::StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateFunction",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Type", Type)
                              .Attr("Name", Name)
                              .AttrList("Scopes", Scopes).End());
    S.getValidator().CheckCreateFunction(Loc, Type, Name, Scopes);
    ASTFunction *F = new ASTFunction(Module, Loc, Type, Name, Scopes);
    F->ErrorHandler = CreateErrorHandlerParam();
    return F;
}

/**
 * Creates an ASTClass
 * @param Module
 * @param Loc
 * @param Name
 * @param Scopes
 * @return
 */
ASTClass *SemaBuilder::CreateClass(ASTModule *Module, const SourceLocation &Loc, ASTClassKind ClassKind, const llvm::StringRef Name,
                                   llvm::SmallVector<ASTScope *, 8> &Scopes, llvm::SmallVector<ASTClassType *, 4> &ClassTypes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClass",
                      Logger().Attr("ClassKind", (uint64_t) ClassKind)
                              .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Name", Name)
                              .AttrList("Scopes", Scopes)
                              .End());
    S.getValidator().CheckCreateClass(Loc, Name, ClassKind, Scopes, ClassTypes);
    ASTClass *Class = new ASTClass(Module, ClassKind, Scopes, Loc, Name);
    Class->SuperClasses = ClassTypes;
    Class->Type = CreateClassType(Class);

    Class->Module = Module;

    // Lookup into namespace
    Module->Identities.push_back(Class);

    // Create Default Constructor
    SmallVector<ASTScope *, 8> ConstructorScopes = S.Builder->CreateScopes();
    Class->DefaultConstructor = S.Builder->CreateClassConstructor(SourceLocation(), *Class, ConstructorScopes);
    return Class;
}

/**
 * Creates a Scope for Classes
 * @param Visibility
 * @param Constant
 * @param Static
 * @return
 */
SmallVector<ASTScope *, 8> SemaBuilder::CreateScopes(ASTVisibilityKind Visibility, bool Constant, bool Static) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassScopes",
                      Logger().Attr("Visibility", (uint64_t) Visibility)
                              .Attr("Constant", Constant)
                              .Attr("Static", Static).End());
    SmallVector<ASTScope *, 8> Scopes;
    Scopes.push_back(CreateScopeVisibility(SourceLocation(), Visibility));
    Scopes.push_back(CreateScopeStatic(SourceLocation(), Static));
    Scopes.push_back(CreateScopeConstant(SourceLocation(), Constant));
    return Scopes;
}

/**
 * Creates a Scope for Visibility
 * @param Visibility
 * @param Constant
 * @param Static
 * @return
 */
ASTScope *SemaBuilder::CreateScopeVisibility(const SourceLocation &Loc, ASTVisibilityKind Visibility) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateVisibilityScope",
                      Logger().Attr("Visibility", (uint64_t) Visibility).End());
    ASTScope *Scope = new ASTScope(Loc, ASTScopeKind::SCOPE_VISIBILITY);
    Scope->Visibility = Visibility;
    return Scope;
}

/**
 * Creates a Scope for Constant
 * @param Visibility
 * @param Constant
 * @param Static
 * @return
 */
ASTScope *SemaBuilder::CreateScopeConstant(const SourceLocation &Loc, bool Constant) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateScopeConstant",
                      Logger().Attr("Constant", Constant).End());
    ASTScope *Scope = new ASTScope(Loc, ASTScopeKind::SCOPE_CONSTANT);
    Scope->Constant = Constant;
    return Scope;
}

/**
 * Creates a Scope for Static
 * @param Visibility
 * @param Constant
 * @param Static
 * @return
 */
ASTScope *SemaBuilder::CreateScopeStatic(const SourceLocation &Loc, bool Static) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateScopeStatic",
                      Logger().Attr("Static", Static).End());
    ASTScope *Scope = new ASTScope(Loc, ASTScopeKind::SCOPE_STATIC);
    Scope->Static = Static;
    return Scope;
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
                                                     llvm::StringRef Name, SmallVector<ASTScope *, 8> &Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassAttribute",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                                .Attr("Type", Type)
                                .Attr("Name", Name)
                                .AttrList("Scopes", Scopes).End());
    S.getValidator().CheckCreateClassVar(Loc, Name, Type, Scopes);
    ASTClassAttribute *Attribute = new ASTClassAttribute(Loc, Type, Name, Scopes);
    Attribute->Class = &Class;
    Class.Attributes.push_back(Attribute);
    return Attribute;
}

ASTClassMethod *SemaBuilder::CreateClassConstructor(const SourceLocation &Loc, ASTClass &Class,
                                                    SmallVector<ASTScope *, 8> &Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassConstructor",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .AttrList("Scopes", Scopes)
                              .End());
    S.getValidator().CheckCreateClassConstructor(Loc, Scopes);
    ASTClassMethod *M = new ASTClassMethod(Loc, ASTClassMethodKind::METHOD_CONSTRUCTOR, CreateVoidType(Loc), Class.getName(), Scopes);

    // FIXME replace with vector push
    if (!InsertFunction(Class.Constructors, M)) {
        S.Diag(M->getLocation(), diag::err_sema_class_method_redeclare) << M->getName();
    }

    M->ErrorHandler = CreateErrorHandlerParam();
    M->Class = &Class;
    CreateBody(M);
    return M;
}

ASTClassMethod *SemaBuilder::CreateClassMethod(const SourceLocation &Loc, ASTClass &Class, ASTType *Type,
                                               llvm::StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassMethod",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Type", Type)
                              .Attr("Name=", Name)
                              .AttrList("Scopes", Scopes)
                              .End());
    S.getValidator().CheckCreateClassMethod(Loc, Type, Name, Scopes);
    ASTClassMethod *M = new ASTClassMethod(Loc, ASTClassMethodKind::METHOD, Type, Name, Scopes);

    // FIXME replace with vector push
    if (!InsertFunction(Class.Methods, M)) {
        S.Diag(M->getLocation(), diag::err_sema_class_method_redeclare) << M->getName();
    }

    M->ErrorHandler = CreateErrorHandlerParam();
    M->Class = &Class;
    return M;
}

ASTClassMethod *SemaBuilder::CreateClassVirtualMethod(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                                      llvm::SmallVector<ASTScope *, 8> &Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateAbstractClassMethod",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Type", Type)
                              .Attr("Name=", Name)
                              .AttrList("Scopes", Scopes)
                              .End());
    S.getValidator().CheckCreateClassMethod(Loc, Type, Name, Scopes);
    ASTClassMethod *M = new ASTClassMethod(Loc, ASTClassMethodKind::METHOD_VIRTUAL, Type, Name, Scopes);
    return M;
}

ASTEnum *SemaBuilder::CreateEnum(ASTModule *Module, const SourceLocation &Loc, const llvm::StringRef Name,
                                 llvm::SmallVector<ASTScope *, 8> &Scopes,
                                 llvm::SmallVector<ASTEnumType *, 4> EnumTypes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateEnum",
                      Logger().AttrList("Scopes", Scopes).Attr("Name", Name).End());
    S.getValidator().CheckCreateEnum(Loc, Name, Scopes, EnumTypes);
    ASTEnum *Enum = new ASTEnum(Module, Loc, Name, Scopes, EnumTypes);
    Enum->Type = CreateEnumType(Enum);
    return Enum;
}

ASTEnumEntry *SemaBuilder::CreateEnumEntry(const SourceLocation &Loc, ASTEnum *Enum, llvm::StringRef Name) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateEnumEntry",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Name", Name).End());
    S.getValidator().CheckCreateEnumEntry(Loc, Name);
    SmallVector<ASTScope *, 8> Scopes = CreateScopes();
    ASTEnumEntry *EnumEntry = new ASTEnumEntry(Loc, Enum->Type, Name, Scopes);

    EnumEntry->Enum = Enum;
    EnumEntry->Index = Enum->Entries.size() + 1;
    Enum->Entries.push_back(EnumEntry);
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

ASTErrorType *SemaBuilder::CreateErrorType() {
    FLY_DEBUG("SemaBuilder", "CreateErrorType");
    return new ASTErrorType();
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
ASTArrayValue *SemaBuilder::CreateArrayValue(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateArrayValue",
                      "Loc=" << Loc.getRawEncoding());
    return new ASTArrayValue(Loc);
}

ASTStringValue *SemaBuilder::CreateStringValue(const SourceLocation &Loc, llvm::StringRef Str) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateStringValue",
                      "Loc=" << Loc.getRawEncoding());
    return new ASTStringValue(Loc, Str);
}

ASTStructValue *SemaBuilder::CreateStructValue(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateArrayValue",
                      "Loc=" << Loc.getRawEncoding());
    return new ASTStructValue(Loc);
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
ASTParam *SemaBuilder::CreateParam(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                                   llvm::SmallVector<ASTScope *, 8> *Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateParam",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                      .Attr("Type", Type)
                      .Attr("Name", Name)
                      .AttrList("Scopes", *Scopes)
                      .End());
    llvm::SmallVector<ASTScope *, 8> VarScopes;
    if (Scopes == nullptr)
        VarScopes = CreateScopes();
    S.getValidator().CheckCreateParam(Loc, Type, Name, VarScopes);
    ASTParam *Param = new ASTParam(Loc, Type, Name, VarScopes);
    return Param;
}

ASTParam *SemaBuilder::CreateErrorHandlerParam() {
    return CreateParam(SourceLocation(), CreateErrorType(), "error");
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
ASTLocalVar *SemaBuilder::CreateLocalVar(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, llvm::SmallVector<ASTScope *, 8> *Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLocalVar",
                      Logger().Attr("Name", Name)
                              .AttrList("Scopes", *Scopes).End());
    llvm::SmallVector<ASTScope *, 8> VarScopes;
    if (Scopes == nullptr)
        VarScopes = CreateScopes();
    S.getValidator().CheckCreateLocalVar(Loc, Type, Name, VarScopes);
    ASTLocalVar *LocalVar = new ASTLocalVar(Loc, Type, Name, VarScopes);
    return LocalVar;
}

ASTIdentifier *SemaBuilder::CreateIdentifier(const SourceLocation &Loc, llvm::StringRef Name) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateIdentifier", Logger()
            .Attr("Loc", (uint64_t) Loc.getRawEncoding())
            .Attr("Name", Name)
            .End());
    S.getValidator().CheckCreateIdentifier(Loc, Name);
    return new ASTIdentifier(Loc, Name);
}

/**
 * Create an ASTFunctionCall without definition
 * @param Location
 * @param Name
 * @param NameSpace
 * @return
 */
ASTCall *SemaBuilder::CreateCall(ASTIdentifier *Identifier) {
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
ASTCall *SemaBuilder::CreateCall(ASTFunction *Function) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateCall",
                      Logger().Attr("Function=", Function).End());
    ASTCall *Call = new ASTCall(SourceLocation(), Function->Name);
    Call->Def = Function;
    return Call;
}

ASTCall *SemaBuilder::CreateCall(ASTClassMethod *Method) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateCall",
                      Logger().Attr("Method=", Method).End());
    ASTCall *Call = new ASTCall(SourceLocation(), Method->Name);
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

ASTArg *SemaBuilder::CreateArg(ASTExpr *Expr) {
    FLY_DEBUG("SemaBuilder", "CreateArg");
    ASTArg *Arg = new ASTArg(Expr);
    return Arg;
}

ASTVarRef *SemaBuilder::CreateVarRef(ASTIdentifier *Identifier) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateVarRef",
                      Logger().Attr("Identifier", Identifier).End());
    ASTVarRef *VarRef = new ASTVarRef(Identifier->getLocation(), Identifier->getName());
    VarRef->Parent = Identifier->Parent;
    VarRef->Child = Identifier->Child;
    VarRef->FullName = Identifier->FullName;
    // delete Identifier; TODO
    return VarRef;
}

ASTVarRef *SemaBuilder::CreateVarRef(ASTVar *Var) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateVarRef",
                      Logger().Attr("Var", Var).End());
    ASTVarRef *VarRef = new ASTVarRef(Var->getLocation(), Var->getName());
    VarRef->Def = Var;
    return VarRef;
}

ASTVarRef *SemaBuilder::CreateVarRef(ASTIdentifier *Instance, ASTVar *Var) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateVarRef",
                      Logger().Attr("Instance", Instance).Attr("Var", Var).End());
    ASTVarRef *VarRef = CreateVarRef(Var);
    VarRef->Parent = Instance;
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

ASTCallExpr *SemaBuilder::CreateNewExpr(ASTCall *Call) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateNewExpr",
                      Logger().Attr("Call", Call).End());
    Call->CallKind = ASTCallKind::CALL_CONSTRUCTOR;
    return CreateExpr(Call);
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
 * Creates an ASTVarAssign from ASTVarRef
 * @param Parent
 * @param VarRef
 * @return
 */
ASTVarStmt *SemaBuilder::CreateVarStmt(ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateVarStmt",
                      Logger().Attr("VarRef", VarRef).End());
    ASTVarStmt *VarDefine = new ASTVarStmt(VarRef->getLocation(), VarRef);
    return VarDefine;
}

/**
 * Creates an ASTVarAssign from ASTVar
 * @param Parent
 * @param VarRef
 * @return
 */
ASTVarStmt *SemaBuilder::CreateVarStmt(ASTVar *Var) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateVarStmt",
                      Logger().Attr("Var", Var).End());
    ASTVarRef *VarRef = CreateVarRef(Var);
    ASTVarStmt *VarStmt = new ASTVarStmt(Var->getLocation(), VarRef);
    return VarStmt;
}

/**
 * Creates an ASTReturn
 * @param Parent
 * @param Loc
 * @return
 */
ASTReturnStmt *SemaBuilder::CreateReturnStmt(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLocalVar", Logger()
            .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTReturnStmt *Return = new ASTReturnStmt(Loc);
    return Return;
}

ASTBreakStmt *SemaBuilder::CreateBreakStmt(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLocalVar", Logger()
            .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTBreakStmt *Break = new ASTBreakStmt(Loc);
    return Break;
}

ASTContinueStmt *SemaBuilder::CreateContinueStmt(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateContinueStmt", Logger()
            .Attr("Loc", (uint64_t)Loc.getRawEncoding()).End());
    ASTContinueStmt *Continue = new ASTContinueStmt(Loc);
    return Continue;
}

ASTFailStmt *SemaBuilder::CreateFailStmt(const SourceLocation &Loc, ASTVar *ErrorHandler) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateFailStmt", Logger()
            .Attr("Loc", (uint64_t)Loc.getRawEncoding()).End());
    ASTFailStmt *FailStmt = new ASTFailStmt(Loc);
    FailStmt->setErrorHandler(ErrorHandler);
    return FailStmt;
}

ASTExprStmt *SemaBuilder::CreateExprStmt(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateExprStmt", Logger()
            .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTExprStmt *ExprStmt = new ASTExprStmt(Loc);
    return ExprStmt;
}

ASTDeleteStmt *SemaBuilder::CreateDeleteStmt(const SourceLocation &Loc, ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateDeleteStmt",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("VarRef", VarRef).End());
    return new ASTDeleteStmt(Loc, VarRef);
}

ASTBlockStmt*
SemaBuilder::CreateBody(ASTFunctionBase *FunctionBase) {
    FLY_DEBUG("SemaBuilder", "CreateBody");
    FunctionBase->Body = CreateBlockStmt(SourceLocation());
    FunctionBase->Body->Function = FunctionBase;
    FunctionBase->Body->ErrorHandler = FunctionBase->ErrorHandler;
    return FunctionBase->Body;
}

ASTBlockStmt *SemaBuilder::CreateBlockStmt(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateBlockStmt", Logger()
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTBlockStmt *Block = new ASTBlockStmt(Loc);
    return Block;
}

ASTIfStmt *SemaBuilder::CreateIfStmt(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateIfStmt", Logger()
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTIfStmt *IfStmt = new ASTIfStmt(Loc);
    return IfStmt;
}

ASTSwitchStmt *SemaBuilder::CreateSwitchStmt(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateSwitchStmt", Logger()
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTSwitchStmt *SwitchBlock = new ASTSwitchStmt(Loc);
    return SwitchBlock;
}

ASTLoopStmt *SemaBuilder::CreateLoopStmt(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLoopStmt", Logger()
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTLoopStmt *LoopStmt = new ASTLoopStmt(Loc);
    LoopStmt->Init = CreateBlockStmt(Loc);
    return LoopStmt;
}

ASTHandleStmt *SemaBuilder::CreateHandleStmt(const SourceLocation &Loc, ASTVarRef *ErrorRef) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateHandleStmt", Logger()
            .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTHandleStmt *Block = new ASTHandleStmt(Loc);
    Block->setErrorHandlerRef(ErrorRef);
    return Block;
}

/********************** Following Methods Adds AST objects to other AST object ****************************************/

bool
SemaBuilder::AddParam(ASTFunctionBase *FunctionBase, ASTParam *Param) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "addParam", Logger().Attr("Param", Param).End());

    // Check var duplicates
    if (S.Validator->CheckDuplicateParams(FunctionBase->Params, Param)) {
        FunctionBase->Params.push_back(Param);
        return true;
    }

    return false;
}

void SemaBuilder::AddFunctionVarParams(ASTFunctionBase *Function, ASTParam *Param) {
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
        ASTArg *Arg = new ASTArg(Expr);
        Arg->Index = Call->getArgs().empty() ? 0 : Call->getArgs().size();
        Call->Args.push_back(Arg);
    }
    return true;
}

bool SemaBuilder::AddLocalVar(ASTBlockStmt *BlockStmt, ASTLocalVar *LocalVar) {
    BlockStmt->Function->LocalVars.push_back(LocalVar);
    return BlockStmt->LocalVars.insert(std::make_pair(LocalVar->getName(), LocalVar)).second;
}

/**
 * Add ExprStmt to Content
 * @param ExprStmt
 * @return true if no error occurs, otherwise false
 */
bool
SemaBuilder::AddStmt(ASTStmt *Parent, ASTStmt *Stmt) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddStmt", Logger().Attr("Stmt", Stmt).End());
    Stmt->Parent = Parent;
    Stmt->Function = Parent->Function;
    if (Parent->getKind() == ASTStmtKind::STMT_BLOCK) {
        ASTBlockStmt *BlockStmt = (ASTBlockStmt *) Parent;
        BlockStmt->Content.push_back(Stmt);
    } else if (Parent->getKind() == ASTStmtKind::STMT_IF) {
        ASTIfStmt *IfStmt = (ASTIfStmt *) Parent;
        IfStmt->Stmt = Stmt;
    } else if (Parent->getKind() == ASTStmtKind::STMT_LOOP) {
        ASTLoopStmt *LoopStmt = (ASTLoopStmt *) Parent;
        LoopStmt->Loop = Stmt;
    }
    return true;
}

bool SemaBuilder::AddElsif(ASTIfStmt *IfStmt, ASTExpr *Condition, ASTStmt *Stmt) {
    ASTElsif *Elsif = new ASTElsif(Stmt->getLocation());
    Elsif->Condition = Condition;
    Elsif->Stmt = Stmt;
    Elsif->Stmt->Parent = IfStmt;
    IfStmt->Elsif.push_back(Elsif);
    return !IfStmt->Elsif.empty();
}

bool SemaBuilder::AddElse(ASTIfStmt *IfStmt, ASTStmt *Else) {
    IfStmt->Else = Else;
    IfStmt->Else->Parent = IfStmt;
    return true;
}

bool SemaBuilder::AddSwitchCase(ASTSwitchStmt *SwitchStmt, ASTValueExpr *ValueExpr, ASTStmt *Stmt) {
    Stmt->Function = SwitchStmt->Function;
    Stmt->Parent = SwitchStmt;
    ASTSwitchCase *Case = new ASTSwitchCase(Stmt->getLocation());
    Case->Value = ValueExpr;
    Case->Stmt = Stmt;
    SwitchStmt->Cases.push_back(Case);
    return !SwitchStmt->Cases.empty();
}

bool SemaBuilder::AddSwitchDefault(ASTSwitchStmt *SwitchStmt, ASTStmt *Stmt) {
    Stmt->Parent = SwitchStmt;
    Stmt->Function = SwitchStmt->Function;
    SwitchStmt->Default = Stmt;
    return true;
}

bool SemaBuilder::AddLoopInit(ASTLoopStmt *LoopStmt, ASTStmt *Stmt) {
    Stmt->Parent = LoopStmt;
    Stmt->Function = LoopStmt->Function;
    LoopStmt->Init = Stmt;
    return true;
}

bool SemaBuilder::AddLoopPost(ASTLoopStmt *LoopStmt, ASTStmt *Stmt) {
    Stmt->Parent = LoopStmt->Init;
    Stmt->Function = LoopStmt->Function;
    LoopStmt->Post = Stmt;
    return true;
}

bool SemaBuilder::AddExpr(ASTVarStmt *Stmt, ASTExpr *Expr) {
    Stmt->Expr = Expr;
    return true;
}

bool SemaBuilder::AddExpr(ASTExprStmt *Stmt, ASTExpr *Expr) {
    Stmt->Expr = Expr;
    return true;
}

bool SemaBuilder::AddExpr(ASTReturnStmt *Stmt, ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddExpr",
                      Logger().Attr("Stmt", Stmt).Attr("Expr", Expr).End());
    Stmt->Expr = Expr;
    return true;
}

bool SemaBuilder::AddExpr(ASTFailStmt *Stmt, ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddExpr",
                      Logger().Attr("Stmt", Stmt).Attr("Expr", Expr).End());
    Stmt->Expr = Expr;
    return true;
}

bool SemaBuilder::AddExpr(ASTIfStmt *Stmt, ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddExpr",
                      Logger().Attr("Stmt", Stmt).Attr("Expr", Expr).End());
    Stmt->Condition = Expr;
    return true;
}

bool SemaBuilder::AddExpr(ASTSwitchStmt *SwitchStmt, ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddExpr",
                      Logger().Attr("Stmt", SwitchStmt).Attr("Expr", Expr).End());
    if (S.Validator->CheckVarRefExpr(Expr)) {
        ASTVarRefExpr *VarRefExpr = (ASTVarRefExpr *) Expr;
        SwitchStmt->VarRef = VarRefExpr->VarRef;
    }
    return true;
}

bool SemaBuilder::AddExpr(ASTLoopStmt *LoopStmt, ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddExpr",
                      Logger().Attr("Stmt", LoopStmt).Attr("Expr", Expr).End());
    LoopStmt->Condition = Expr;
    return true;
}
