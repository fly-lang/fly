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
#include "AST/ASTExpr.h"
#include "AST/ASTGroupExpr.h"
#include "AST/ASTOperatorExpr.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTEnumEntry.h"
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
    S.Context->NameSpaces.insert(std::make_pair(S.Context->DefaultNameSpace->FullName, S.Context->DefaultNameSpace));
    AddNameSpace(nullptr, S.Context->DefaultNameSpace);
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
    ASTModule *Module = new ASTModule(Name, S.Context, false);
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
    return new ASTModule(Name, S.Context, true);
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

    S.getValidator().CheckCreateNameSpace(Identifier->getLocation(), Identifier->getName());
    NameSpace = new ASTNameSpace(Identifier->getLocation(), Identifier->getName(), S.Context);

    // Iterate over parents
    if (Identifier->getParent() != nullptr) {
        NameSpace->Parent = CreateNameSpace(Identifier->getParent());
    }

    return NameSpace;
}

ASTImport *SemaBuilder::CreateImport(const SourceLocation &Loc, llvm::StringRef Name) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateImport",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Name", Name).End());
    S.getValidator().CheckCreateImport(Loc, Name);
    return new ASTImport(Loc, Name);
}

ASTAlias *SemaBuilder::CreateAlias(const SourceLocation &Loc, llvm::StringRef Name) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateImport",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Name", Name).End());
    S.getValidator().CheckCreateAlias(Loc, Name);
    return new ASTAlias(Loc, Name);
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
ASTGlobalVar *SemaBuilder::CreateGlobalVar(const SourceLocation &Loc, ASTType *Type,
                                           const llvm::StringRef Name, ASTScopes *Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateGlobalVar",
                      Logger()
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                      .Attr("Type", Type)
                      .Attr("Name", Name)
                      .Attr("Scopes", Scopes).End());
    S.getValidator().CheckCreateGlobalVar(Loc, Type, Name, Scopes);
    return new ASTGlobalVar(Loc, Type, Name, Scopes);
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
ASTFunction *SemaBuilder::CreateFunction(const SourceLocation &Loc, ASTType *Type,
                                         const llvm::StringRef Name, ASTScopes *Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateFunction",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Type", Type)
                              .Attr("Name", Name)
                              .Attr("Scopes", Scopes).End());
    S.getValidator().CheckCreateFunction(Loc, Type, Name, Scopes);
    ASTFunction *F = new ASTFunction(Loc, Type, Name, Scopes);
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
ASTClass *SemaBuilder::CreateClass(const SourceLocation &Loc, ASTClassKind ClassKind, const llvm::StringRef Name,
                         ASTScopes *ClassScopes, llvm::SmallVector<ASTClassType *, 4> &ClassTypes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClass",
                      Logger().Attr("ClassKind", (uint64_t) ClassKind)
                              .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Name", Name)
                              .Attr("Scopes", ClassScopes)
                              .End());
    S.getValidator().CheckCreateClass(Loc, Name, ClassKind, ClassScopes, ClassTypes);
    ASTClass *Class = new ASTClass(ClassKind, ClassScopes, Loc, Name);
    Class->SuperClasses = ClassTypes;
    Class->Type = CreateClassType(Class);

    // Create Default Constructor
    ASTScopes *Scopes = S.Builder->CreateScopes();
    Class->DefaultConstructor = S.Builder->CreateClassConstructor(SourceLocation(), *Class, Scopes);
    return Class;
}

/**
 * Creates a Scope for Classes
 * @param Visibility
 * @param Constant
 * @param Static
 * @return
 */
ASTScopes *SemaBuilder::CreateScopes(ASTVisibilityKind Visibility, bool Constant, bool Static) {
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
ASTClassAttribute *SemaBuilder::CreateClassAttribute(const SourceLocation &Loc, ASTClass &Class, ASTType *Type, llvm::StringRef Name, ASTScopes *Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassAttribute",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                                .Attr("Type", Type)
                                .Attr("Name", Name)
                                .Attr("Scopes", Scopes).End());
    S.getValidator().CheckCreateClassVar(Loc, Name, Type, Scopes);
    ASTClassAttribute *Attribute = new ASTClassAttribute(Loc, Type, Name, Scopes);
    Attribute->Class = &Class;
    Class.Attributes.insert(std::pair<llvm::StringRef, ASTClassAttribute *>(Attribute->getName(), Attribute));
    return Attribute;
}

ASTClassMethod *SemaBuilder::CreateClassConstructor(const SourceLocation &Loc, ASTClass &Class, ASTScopes *Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassConstructor",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Scopes", Scopes)
                              .End());
    S.getValidator().CheckCreateClassConstructor(Loc, Scopes);
    ASTClassMethod *M = new ASTClassMethod(Loc, ASTClassMethodKind::METHOD_CONSTRUCTOR, CreateVoidType(Loc), Class.getName(), Scopes);
    M->ErrorHandler = CreateErrorHandlerParam();
    M->Class = &Class;
    CreateBody(M);
    return M;
}

ASTClassMethod *SemaBuilder::CreateClassMethod(const SourceLocation &Loc, ASTClass &Class, ASTType *Type, llvm::StringRef Name, ASTScopes *Scopes,
                               bool Static) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassMethod",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Type", Type)
                              .Attr("Name=", Name)
                              .Attr("Scopes", Scopes)
                              .End());
    S.getValidator().CheckCreateClassMethod(Loc, Type, Name, Scopes);
    ASTClassMethodKind Kind = Static ? ASTClassMethodKind::METHOD_STATIC : ASTClassMethodKind::METHOD_CUSTOM;
    ASTClassMethod *M = new ASTClassMethod(Loc, Kind, Type, Name, Scopes);
    M->ErrorHandler = CreateErrorHandlerParam();
    M->Class = &Class;
    return M;
}

ASTClassMethod *SemaBuilder::CreateClassVirtualMethod(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                                      ASTScopes *Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateAbstractClassMethod",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Type", Type)
                              .Attr("Name=", Name)
                              .Attr("Scopes", Scopes)
                              .End());
    S.getValidator().CheckCreateClassMethod(Loc, Type, Name, Scopes);
    ASTClassMethod *M = new ASTClassMethod(Loc, ASTClassMethodKind::METHOD_VIRTUAL, Type, Name, Scopes);
    return M;
}

ASTEnum *SemaBuilder::CreateEnum(const SourceLocation &Loc, const llvm::StringRef Name, ASTScopes *Scopes,
                        llvm::SmallVector<ASTEnumType *, 4> EnumTypes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateEnum",
                      Logger().Attr("Scopes", Scopes).Attr("Name", Name).End());
    S.getValidator().CheckCreateEnum(Loc, Name, Scopes, EnumTypes);
    ASTEnum *Enum = new ASTEnum(Loc, Name, Scopes, EnumTypes);
    Enum->Type = CreateEnumType(Enum);
    return Enum;
}

ASTEnumEntry *SemaBuilder::CreateEnumEntry(const SourceLocation &Loc, ASTEnumType *Type, llvm::StringRef Name) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateEnumEntry",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Name", Name).End());
    S.getValidator().CheckCreateEnumEntry(Loc, Name);
    return new ASTEnumEntry(Loc, Type, Name);
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
ASTParam *SemaBuilder::CreateParam(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, ASTScopes *Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateParam",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                      .Attr("Type", Type)
                      .Attr("Name", Name)
                      .Attr("Scopes", Scopes)
                      .End());
    S.getValidator().CheckCreateParam(Loc, Type, Name, Scopes);
    ASTParam *Param = new ASTParam(Loc, Type, Name, Scopes);
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
ASTLocalVar *SemaBuilder::CreateLocalVar(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, ASTScopes *Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLocalVar",
                      Logger().Attr("Name", Name).End());
    S.getValidator().CheckCreateLocalVar(Loc, Type, Name, Scopes);
    ASTLocalVar *LocalVar = new ASTLocalVar(Loc, Type, Name, Scopes);
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

ASTFailStmt *SemaBuilder::CreateFailStmt(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateFailStmt", Logger()
            .Attr("Loc", (uint64_t)Loc.getRawEncoding()).End());
    ASTFailStmt *FailStmt = new ASTFailStmt(Loc);
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
/**
 * Add an ASTModule to the ASTContext and in its ASTNameSpace
 * @param Module
 * @return true if no error occurs, otherwise false
 */
bool
SemaBuilder::AddModule(ASTModule *Module, ASTNameSpace *NameSpace) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddModule", Logger().Attr("Module", Module).End());

    // If no NameSpace is assign set Default NameSpace
    Module->NameSpace = (NameSpace == nullptr) ? S.Context->DefaultNameSpace : NameSpace;

    // Add to Modules
    auto Pair = std::make_pair(Module->getName(), Module);
    // TODO check duplicated in namespace and context
    return Module->NameSpace->Modules.insert(Pair).second && S.Context->Modules.insert(Pair).second;
}

/**
 * Add an ASTNameSpace to the ASTContext if not exists yet
 * @param Name
 * @param ExternLib
 * @return the result of add
 */
bool
SemaBuilder::AddNameSpace(ASTModule *Module, ASTNameSpace *NewNameSpace, bool ExternLib) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddNameSpace",
                      "NameSpace=" << NewNameSpace << ", ExternLib=" << ExternLib);
    // Check if Name exist or add it
    ASTNameSpace *NameSpace = S.Context->NameSpaces.lookup(NewNameSpace->getName());
    if (NameSpace == nullptr) {
        S.Context->NameSpaces.insert(std::make_pair(NewNameSpace->getFullName(), NewNameSpace));
        NameSpace = NewNameSpace;
        if (NewNameSpace->getParent()) {
            return AddNameSpace(nullptr, (ASTNameSpace *) NewNameSpace->getParent());
        }
    }

    if (Module)
        Module->NameSpace = NameSpace;

    return true;
}

bool
SemaBuilder::AddImport(ASTModule *Module, ASTImport * Import) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddImport", Logger()
                        .Attr("Module", Module)
                        .Attr("Import", Import).End());

    if (S.Validator->CheckImport(Module, Import)) {

        // Check if this ASTModule already contains the imports
        if (Module->Imports.find(Import->getName()) != Module->Imports.end()) {
            S.Diag(Import->getLocation(), diag::err_conflict_import) << Import->getName();
            return false;
        }

        // Check if this ASTModule already contains the name or alias
        llvm::StringRef Id = (Import->getAlias() == nullptr) ? Import->getName() : Import->getAlias()->getName();
        if (Module->AliasImports.find(Id) != Module->Imports.end()) {
            S.Diag(Import->getLocation(), diag::err_conflict_import) << Id;
            return false;
        }

        // Add Import to Module
        return Module->Imports.insert(std::make_pair(Import->getName(), Import)).second &&
            Module->AliasImports.insert(std::make_pair(Id, Import)).second;
    }

    return false;
}

bool
SemaBuilder::AddExternalGlobalVar(ASTModule *Module, ASTGlobalVar *GlobalVar) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddExternalGlobalVar", Logger()
            .Attr("Module", Module)
            .Attr("GlobalVar", GlobalVar).End());
    // TODO Check duplicate
    GlobalVar->Module = Module;
    return Module->ExternalGlobalVars.insert(std::make_pair(GlobalVar->getName(), GlobalVar)).second;
}

bool
SemaBuilder::AddExternalFunction(ASTModule *Module, ASTFunction *Function) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddExternalFunction", Logger()
            .Attr("Module", Module)
            .Attr("Function", Function).End());
    Function->Module = Module;
    // TODO Check duplicate
    return InsertFunction(Module->ExternalFunctions, Function);
}

bool SemaBuilder::AddExternalIdentities(ASTModule *Module, ASTIdentity *Identity) {
    return Module->ExternalIdentities.insert(std::make_pair(Identity->getName(), Identity)).second;
}

bool
SemaBuilder::AddGlobalVar(ASTModule *Module, ASTGlobalVar *GlobalVar, ASTValue *Value) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddGlobalVar", Logger()
                      .Attr("GlobalVar", GlobalVar)
                      .Attr("Value", Value).End());

    GlobalVar->Module = Module;
    GlobalVar->Expr = SemaBuilder::CreateExpr(Value);

    // Lookup into namespace for public var
    if(GlobalVar->getScopes()->Visibility == ASTVisibilityKind::V_PUBLIC ||
        GlobalVar->getScopes()->Visibility == ASTVisibilityKind::V_DEFAULT) {
        ASTGlobalVar *LookupVar = Module->NameSpace->getGlobalVars().lookup(GlobalVar->getName());

        if (LookupVar) { // This NameSpace already contains this GlobalVar
            S.Diag(LookupVar->getLocation(), diag::err_duplicate_gvar) << LookupVar->getName();
            return false;
        }

        // Add into NameSpace for global resolution
        // Add into Module for local resolution
        auto Pair = std::make_pair(GlobalVar->getName(), GlobalVar);
        // TODO check duplicate in namespace and Module
        return Module->GlobalVars.insert(Pair).second && Module->NameSpace->GlobalVars.insert(Pair).second;
    }

    // Lookup into Module for private var
    if(GlobalVar->getScopes()->Visibility == ASTVisibilityKind::V_PRIVATE) {
        ASTGlobalVar *LookupVar = Module->GlobalVars.lookup(GlobalVar->getName());

        if (LookupVar) { // This Module already contains this Function
            S.Diag(LookupVar->getLocation(), diag::err_duplicate_gvar) << LookupVar->getName();
            return false;
        }

        // Add into Module for local resolution
        auto Pair = std::make_pair(GlobalVar->getName(), GlobalVar);
        // TODO check duplicate in Module
        return Module->GlobalVars.insert(Pair).second;
    }

    assert(0 && "Unknown GlobalVar Visibility");
}

bool
SemaBuilder::AddFunction(ASTModule *Module, ASTFunction *Function) {
    FLY_DEBUG_MESSAGE("ASTModule", "AddFunction",
                      Logger().Attr("Function", Function).End());

    Function->Module = Module;

    // Lookup into namespace for public var
    if(Function->Scopes->Visibility == ASTVisibilityKind::V_PUBLIC ||
        Function->Scopes->Visibility == ASTVisibilityKind::V_DEFAULT) {

        // Add into NameSpace for global resolution
        // Add into Module for local resolution
        return InsertFunction(Module->NameSpace->Functions, Function) &&
            InsertFunction(Module->Functions, Function);
    }

    // Lookup into Module for private var
    if (Function->Scopes->Visibility == ASTVisibilityKind::V_PRIVATE) {

        // Add into Module for local resolution
        return InsertFunction(Module->Functions, Function);
    }

    assert(0 && "Unknown Function Visibility");
}

bool
SemaBuilder::AddIdentity(ASTModule *Module, ASTIdentity *Identity) {
    FLY_DEBUG_MESSAGE("ASTModule", "AddIdentity", Logger()
            .Attr("Identity", Identity).End());

    Identity->Module = Module;

    // Lookup into namespace
    Module->Identities.insert(std::make_pair(Identity->getName(), Identity));

    bool Success = true;
    if (Identity->Scopes->Visibility == ASTVisibilityKind::V_PUBLIC || Identity->Scopes->Visibility == ASTVisibilityKind::V_DEFAULT) {
        ASTIdentity *LookupClass = Module->NameSpace->getIdentities().lookup(Identity->getName());
        if (LookupClass) { // This NameSpace already contains this Function
            S.Diag(LookupClass->Location, diag::err_duplicate_class) << LookupClass->getName();
            return false;
        }
        Success = Module->NameSpace->Identities.insert(std::make_pair(Identity->getName(), Identity)).second;
    }

    return Success;
}

bool
SemaBuilder::AddClassAttribute(ASTClass *Class, ASTClassAttribute *Var) {
    if (Class->Attributes.insert(std::pair<llvm::StringRef, ASTClassAttribute *>(Var->getName(), Var)).second) {
        Var->Class = Class;
        return Var;
    }

    S.Diag(Var->getLocation(), diag::err_sema_class_field_redeclare) << Var->getName();
    return false;
}

bool
SemaBuilder::AddClassMethod(ASTClass *Class, ASTClassMethod *Method) {
    Method->Class = Class;
    if (Method->isConstructor()) {

        // Check duplicates
        if (!InsertFunction(Class->Constructors, Method)) {
            S.Diag(Method->getLocation(), diag::err_sema_class_method_redeclare) << Method->getName();
            return false;
        }
    } else {
        // Check duplicates
        if (!InsertFunction(Method->Class->Methods, Method)) {
            S.Diag(Method->getLocation(), diag::err_sema_class_method_redeclare) << Method->getName();
            return false;
        }
    }

    return true;
}

bool SemaBuilder::AddEnumEntry(ASTEnum *Enum, ASTEnumEntry *Entry) {
    Entry->Enum = Enum;
    Entry->Index = Entry->getEnum()->Entries.size() + 1;
    return Entry->getEnum()->Entries.insert(std::make_pair(Entry->getName(), Entry)).second;
}

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
SemaBuilder::AddComment(ASTBase *Base, llvm::StringRef Comment) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "AddComment", Logger()
                        .Attr("Comment", Comment).End());
    Base->Comment = Comment;
    return true;
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
