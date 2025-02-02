//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/Sema.cpp - The Sema Builder
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/ASTBuilder.h"

#include "Sema/SemaBuilderScopes.h"
#include "Sema/SemaBuilderStmt.h"
#include "Sema/SemaBuilderIfStmt.h"
#include "Sema/SemaBuilderSwitchStmt.h"
#include "Sema/SemaBuilderLoopStmt.h"
#include "Sema/Sema.h"
#include "Sema/SemaValidator.h"
#include "Sema/SymBuilder.h"
#include "CodeGen/CodeGen.h"
#include "AST/ASTAlias.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTModule.h"
#include "AST/ASTImport.h"
#include "AST/ASTArg.h"
#include "AST/ASTComment.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTBreakStmt.h"
#include "AST/ASTContinueStmt.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTDeleteStmt.h"
#include "AST/ASTVar.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTIfStmt.h"
#include "AST/ASTLoopStmt.h"
#include "AST/ASTSwitchStmt.h"
#include "AST/ASTHandleStmt.h"
#include "AST/ASTAssignmentStmt.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTValue.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
#include "AST/ASTExpr.h"
#include "AST/ASTOpExpr.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTFailStmt.h"
#include "Basic/SourceLocation.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"

#include <utility>
#include <AST/ASTTypeRef.h>

using namespace fly;

const uint8_t ASTBuilder::DEFAULT_INTEGER_RADIX= 10;
/**
 * Private constructor used only from Sema constructor
 * @param S
 */
ASTBuilder::ASTBuilder(Sema &S) : S(S) {
	FLY_DEBUG("SemaBuilder", "SemaBuilder");
}

const llvm::StringRef ASTBuilder::DEFAULT_INTEGER_VALUE = StringRef("0");
const llvm::StringRef ASTBuilder::DEFAULT_FLOATING_VALUE = StringRef("0.0");

/**
 * Creates an ASTModule
 * If NameSpace doesn't exists it will be created
 * @param Name
 * @return the ASTModule
 */
ASTModule *ASTBuilder::CreateModule(const std::string &Name) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "GenerateModule", "Name=" << Name);
    S.getValidator().CheckCreateModule(Name);
    uint64_t Id = ModuleIdCounter++; // FIXME compare Module by using FileID
    ASTModule *Module = new ASTModule(Id, Name, false);
	S.Modules.push_back(Module);
    return Module;
}

/**
 * Creates an ASTHeaderModule: only prototype declarations without definitions
 * For .fly.h file generation
 * @param Name
 * @param NameSpace
 * @return thee ASTHeaderModule
 */
ASTModule *ASTBuilder::CreateHeaderModule(const std::string &Name) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateHeaderModule", "Name=" << Name);
    uint64_t Id = ModuleIdCounter++; // FIXME compare Module by using FileID
    return new ASTModule(Id, Name, true);
}

ASTComment *ASTBuilder::CreateComment(ASTModule *Module, const SourceLocation &Loc, llvm::StringRef Content) {
	FLY_DEBUG_MESSAGE("SemaBuilder", "CreateComment",
					  Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
							  .Attr("Content", Content).End());
	ASTComment *Comment = new ASTComment(Loc, Content);

	// Add Comment to Module
	Module->Definitions.push_back(Comment);

	return Comment;
}

/**
 * Create a NameSpace for each parent
 * NS3.NS2.NS1 -> Identifier->Parent
 * NS2.NS1 -> Identifier->Parent->Parent
 * NS3 -> Identifier->Parent->Parent->Parent ... until to Root
 * @param Identifier
 * @return
 */
ASTNameSpace *ASTBuilder::CreateNameSpace(ASTModule *Module, ASTIdentifier *Identifier) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateNameSpaceRef", "Identifier=" << Identifier->str());

    S.getValidator().CheckCreateNameSpace(Identifier->getLocation(), Identifier->getName());
    ASTNameSpace *NameSpaceRef = new ASTNameSpace(Identifier->getLocation(), Identifier->getName());

	// Add NameSpace to Module
    Module->NameSpace = NameSpaceRef;

    return NameSpaceRef;
}

ASTImport *ASTBuilder::CreateImport(ASTModule *Module, const SourceLocation &Loc, llvm::StringRef Name, ASTAlias *Alias) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateImport",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Name", Name).End());
    S.getValidator().CheckCreateImport(Loc, Name);
    ASTImport *Import = new ASTImport(Loc, Name);

    // Add Import to Module
	Module->Definitions.push_back(Import);

    if (Alias) {
        Import->setAlias(Alias);
    }

    return Import;
}

ASTAlias * ASTBuilder::CreateAlias(SourceLocation Loc, llvm::StringRef Name) {
	FLY_DEBUG_MESSAGE("SemaBuilder", "CreateAlias", "Name=" << Name);
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
ASTVar *ASTBuilder::CreateGlobalVar(ASTModule *Module, const SourceLocation &Loc, ASTTypeRef *Type,
                                           const llvm::StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes,
                                           ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateGlobalVar",
                      Logger()
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                      .Attr("Type", Type)
                      .Attr("Name", Name)
                      .AttrList("Scopes", Scopes).End());
    S.getValidator().CheckCreateGlobalVar(Loc, Type, Name, Scopes);
    ASTVar *GlobalVar = new ASTVar(Loc, Type, Name, Scopes);
    GlobalVar->Expr = Expr;

	// Global Var to Module
	Module->Definitions.push_back(GlobalVar);

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
ASTFunction *ASTBuilder::CreateFunction(ASTModule *Module, const SourceLocation &Loc, ASTTypeRef *TypeRef,
                                         llvm::StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes,
                                         SmallVector<ASTVar *, 8> &Params, ASTBlockStmt *Body) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateFunction",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("TypeRef", TypeRef)
                              .Attr("Name", Name)
                              .AttrList("Scopes", Scopes).End());
    S.getValidator().CheckCreateFunction(Loc, TypeRef, Name, Scopes);
    ASTFunction *Function = new ASTFunction(Loc, Name, TypeRef, Scopes, Params);

    // Create Error handler
    Function->ErrorHandler = CreateErrorHandlerParam();

    // Create Body
    if (Body)
        CreateBody(Function, Body);

	// Add Function to Module
	Module->Definitions.push_back(Function);

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
ASTClass *ASTBuilder::CreateClass(ASTModule *Module, const SourceLocation &Loc, ASTClassKind ClassKind,
                                   const llvm::StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes,
                                   llvm::SmallVector<ASTTypeRef *, 4> &SuperClasses) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClass",
                      Logger().Attr("ClassKind", (uint64_t) ClassKind)
                              .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Name", Name)
                              .AttrList("Scopes", Scopes)
                              .End());
    S.getValidator().CheckCreateClass(Loc, Name, ClassKind, Scopes, SuperClasses);
    ASTClass *Class = new ASTClass(Module, ASTClassKind::CLASS, Scopes, Loc, Name, SuperClasses);

	// Add Class to Module
	Module->Definitions.push_back(Class);

    // Create Default Constructor
    llvm::SmallVector<ASTVar *, 8> Params;
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
ASTVar *ASTBuilder::CreateClassAttribute(const SourceLocation &Loc, ASTClass &Class, ASTTypeRef *Type,
                                                     llvm::StringRef Name, SmallVector<ASTScope *, 8> &Scopes,
                                                     ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassAttribute",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                                .Attr("Type", Type)
                                .Attr("Name", Name)
                                .AttrList("Scopes", Scopes).End());
    S.getValidator().CheckCreateClassAttribute(Loc, Name, Type, Scopes);
    ASTVar *Attribute = new ASTVar(Loc, Class, Type, Name, Scopes);
    Attribute->Expr = Expr;
    Class.Definitions.push_back(Attribute);
    return Attribute;
}

ASTFunction *ASTBuilder::CreateClassConstructor(const SourceLocation &Loc, ASTClass &Class,
                                                    llvm::SmallVector<ASTScope *, 8> &Scopes,
                                                    llvm::SmallVector<ASTVar *, 8> &Params, ASTBlockStmt *Body) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassConstructor",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .AttrList("Scopes", Scopes)
                              .End());
    S.getValidator().CheckCreateClassConstructor(Loc, Scopes);
    ASTFunction *Constructor = new ASTFunction(Loc, ASTFunctionKind::METHOD_CONSTRUCTOR, CreateVoidType(Loc),
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

ASTFunction *ASTBuilder::CreateClassMethod(const SourceLocation &Loc, ASTClass &Class, ASTTypeRef *Type,
                                               llvm::StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes,
                                               llvm::SmallVector<ASTVar *, 8> &Params, ASTBlockStmt *Body) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateClassMethod",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Type", Type)
                              .Attr("Name=", Name)
                              .AttrList("Scopes", Scopes)
                              .End());
    S.getValidator().CheckCreateClassMethod(Loc, Type, Name, Scopes);
    ASTFunction *Method = new ASTFunction(Loc, Type, Name, Scopes, Params);

    // Set Error Handler
    Method->ErrorHandler = CreateErrorHandlerParam();

    if (Body)
        CreateBody(Method, Body);

    // Set Constructor Class
    Method->Class = &Class;

    // Add to Class Methods
    Class.Definitions.push_back(Method);

    return Method;
}

ASTFunction *ASTBuilder::CreateClassVirtualMethod(const SourceLocation &Loc, ASTTypeRef *Type, llvm::StringRef Name,
                                                      llvm::SmallVector<ASTScope *, 8> &Scopes,
                                                      llvm::SmallVector<ASTVar *, 8> &Params) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateAbstractClassMethod",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Type", Type)
                              .Attr("Name=", Name)
                              .AttrList("Scopes", Scopes)
                              .End());
    S.getValidator().CheckCreateClassMethod(Loc, Type, Name, Scopes);
    ASTFunction *VirtualMethod = new ASTFunction(Loc, ASTFunctionKind::METHOD_VIRTUAL, Type, Name, Scopes, Params);
    return VirtualMethod;
}

ASTEnum *ASTBuilder::CreateEnum(ASTModule *Module, const SourceLocation &Loc, const llvm::StringRef Name,
                                 llvm::SmallVector<ASTScope *, 8> &Scopes,
                                 llvm::SmallVector<ASTEnumType *, 4> EnumTypes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateEnum",
                      Logger().AttrList("Scopes", Scopes).Attr("Name", Name).End());
    S.getValidator().CheckCreateEnum(Loc, Name, Scopes, EnumTypes);
    ASTEnum *Enum = new ASTEnum(Module, Loc, Name, Scopes, EnumTypes);
    Enum->Type = CreateEnumType(Enum);

    // Add Enum to Module
	Module->Definitions.push_back(Enum);

    return Enum;
}

ASTEnumEntry *ASTBuilder::CreateEnumEntry(const SourceLocation &Loc, ASTEnum &Enum, llvm::StringRef Name,
                                           llvm::SmallVector<ASTScope *, 8> &Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateEnumEntry",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                              .Attr("Name", Name).End());
    S.getValidator().CheckCreateEnumEntry(Loc, Name);
    ASTEnumEntry *EnumEntry = new ASTEnumEntry(Loc, Enum, Name, Scopes);
    EnumEntry->Index = Enum.Entries.size() + 1;
    Enum.Definitions.push_back(EnumEntry);
    return EnumEntry;
}

/**
 * Creates a bool type
 * @param Loc
 * @return
 */
ASTTypeRef *ASTBuilder::CreateBoolType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateBoolType", "Loc=" << Loc.getRawEncoding());
    ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("bool"));
	TypeRef->Def = S.getSymBuilder().CreateType(SymTypeKind::TYPE_BOOL);
	return TypeRef;
}

/**
 * Creates an byte type
 * @param Loc
 * @return
 */
ASTTypeRef *ASTBuilder::CreateByteType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateByteType", "Loc=" << Loc.getRawEncoding());
	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("byte"));
	TypeRef->Def = S.getSymBuilder().CreateIntType(SymIntTypeKind::TYPE_BYTE);
	return TypeRef;
}

/**
 * Creates an unsigned short type
 * @param Loc
 * @return
 */
ASTTypeRef *ASTBuilder::CreateUShortType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateUShortType", "Loc=" << Loc.getRawEncoding());
	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("ushort"));
	TypeRef->Def = S.getSymBuilder().CreateIntType(SymIntTypeKind::TYPE_USHORT);
	return TypeRef;
}

/**
 * Create a short type
 * @param Loc
 * @return
 */
ASTTypeRef *ASTBuilder::CreateShortType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateShortType", "Loc=" << Loc.getRawEncoding());
	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("short"));
	TypeRef->Def = S.getSymBuilder().CreateIntType(SymIntTypeKind::TYPE_SHORT);
	return TypeRef;
}

/**
 * Creates an unsigned int type
 * @param Loc
 * @return
 */
ASTTypeRef *ASTBuilder::CreateUIntType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateUIntType", "Loc=" << Loc.getRawEncoding());
	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("uint"));
	TypeRef->Def = S.getSymBuilder().CreateIntType(SymIntTypeKind::TYPE_UINT);
	return TypeRef;
}

/**
 * Creates an int type
 * @param Loc
 * @return
 */
ASTTypeRef *ASTBuilder::CreateIntType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateIntType", "Loc=" << Loc.getRawEncoding());
	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("int"));
	TypeRef->Def = S.getSymBuilder().CreateIntType(SymIntTypeKind::TYPE_INT);
	return TypeRef;
}

/**
 * Creates an unsigned long type
 * @param Loc
 * @return
 */
ASTTypeRef *ASTBuilder::CreateULongType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateULongType", "Loc=" << Loc.getRawEncoding());
	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("ulong"));
	TypeRef->Def = S.getSymBuilder().CreateIntType(SymIntTypeKind::TYPE_ULONG);
	return TypeRef;
}

/**
 * Creates a long type
 * @param Loc
 * @return
 */
ASTTypeRef *ASTBuilder::CreateLongType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLongType", "Loc=" << Loc.getRawEncoding());
	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("long"));
	TypeRef->Def = S.getSymBuilder().CreateIntType(SymIntTypeKind::TYPE_LONG);
	return TypeRef;
}

/**
 * Creates a float type
 * @param Loc
 * @return
 */
ASTTypeRef *ASTBuilder::CreateFloatType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateFloatType", "Loc=" << Loc.getRawEncoding());
	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("float"));
	TypeRef->Def = S.getSymBuilder().CreateFPType(SymFPTypeKind::TYPE_FLOAT);
	return TypeRef;
}

/**
 * Creates a double type
 * @param Loc
 * @return
 */
ASTTypeRef *ASTBuilder::CreateDoubleType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateDoubleType", "Loc=" << Loc.getRawEncoding());
	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("double"));
	TypeRef->Def = S.getSymBuilder().CreateFPType(SymFPTypeKind::TYPE_DOUBLE);
	return TypeRef;
}

/**
 * Creates a void type
 * @param Loc
 * @return
 */
ASTTypeRef *ASTBuilder::CreateVoidType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateVoidType", "Loc=" << Loc.getRawEncoding());
	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("void"));
	TypeRef->Def = S.getSymBuilder().CreateType(SymTypeKind::TYPE_VOID);
	return TypeRef;
}

/**
 * Creates an array type
 * @param Loc
 * @param TypeRef
 * @param Size
 * @return
 */
ASTTypeRef *ASTBuilder::CreateArrayType(const SourceLocation &Loc, ASTTypeRef *TypeRef, ASTExpr *Size) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateArrayType",
                      Logger()
                        .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                        .Attr("Type", TypeRef)
                        .Attr("Size", Size).End());
	return new ASTArrayTypeRef(Loc, llvm::StringRef("array"));
}

ASTTypeRef *ASTBuilder::CreateCharType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateCharType", "Loc=" << Loc.getRawEncoding());
	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("char"));
	TypeRef->Def = S.getSymBuilder().CreateType(SymTypeKind::TYPE_CHAR);
	return TypeRef;
}

ASTTypeRef *ASTBuilder::CreateStringType(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateStringType", "Loc=" << Loc.getRawEncoding());
	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("string"));
	TypeRef->Def = S.getSymBuilder().CreateType(SymTypeKind::TYPE_STRING);
	return TypeRef;
}

ASTTypeRef *ASTBuilder::CreateErrorType(const SourceLocation &Loc) {
    FLY_DEBUG("SemaBuilder", "CreateErrorType");
	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("error"));
	TypeRef->Def = S.getSymBuilder().CreateType(SymTypeKind::TYPE_ERROR);
	return TypeRef;
}

/**
 * Creates a default value by type
 * @param Type
 * @return
 */
ASTValue *ASTBuilder::CreateDefaultValue(SymType *Type) {
	FLY_DEBUG_MESSAGE("SemaBuilder", "CreateDefaultValue",
					  Logger().Attr("Type", Type).End());
	ASTValue *Value;
	if (Type->isBool()) {
		Value = CreateBoolValue(SourceLocation(), false);
	} else if (Type->isInteger()) {
		Value = CreateIntegerValue(SourceLocation(), DEFAULT_INTEGER_VALUE, DEFAULT_INTEGER_RADIX);
	} else if (Type->isFloatingPoint()) {
		Value = CreateFloatingValue(SourceLocation(), DEFAULT_FLOATING_VALUE);
	}else if (Type->isArray()) {
		llvm::SmallVector<ASTValue *, 8> Values;
		Value = CreateArrayValue(SourceLocation(), Values);
	} else if (Type->isClass()) {
		Value = CreateNullValue(SourceLocation());
	} else {
		assert("Unknown type");
		Value = nullptr;
	}
	return Value;
}

/**
 * Creates a null value
 * @param Loc
 * @return
 */
ASTNullValue *ASTBuilder::CreateNullValue(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateNullValue", "Loc=" << Loc.getRawEncoding());
    return new ASTNullValue(Loc);
}

/**
 * Creates a zero value
 * @param Loc
 * @return
 */
ASTZeroValue *ASTBuilder::CreateZeroValue(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateNullValue", "Loc=" << Loc.getRawEncoding());
    return new ASTZeroValue(Loc);
}

/**
 * Creates a bool value
 * @param Loc
 * @param Val
 * @return
 */
ASTBoolValue *ASTBuilder::CreateBoolValue(const SourceLocation &Loc, bool Val) {
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
ASTIntegerValue *ASTBuilder::CreateIntegerValue(const SourceLocation &Loc, llvm::StringRef Value, uint8_t Radix) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateIntegerValue",
                      "Loc=" << Loc.getRawEncoding() <<
                      ", Val=" << Value << " Radix=" << Radix);
    return new ASTIntegerValue(Loc, Value, Radix);
}

ASTIntegerValue *ASTBuilder::CreateIntegerValue(const SourceLocation &Loc, llvm::StringRef Value) {
    return new ASTIntegerValue(Loc, Value, DEFAULT_INTEGER_RADIX);
}

/**
 * Creates a floating point value
 * @param Loc
 * @param Val
 * @return
 */
ASTFloatingValue *ASTBuilder::CreateFloatingValue(const SourceLocation &Loc, llvm::StringRef Val) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateFloatingValue",
                      "Loc=" << Loc.getRawEncoding() <<
                      ", Val=" << Val);
    return new ASTFloatingValue(Loc, Val);
}

/**
 * Create an array value
 * @param Loc
 * @return
 */
ASTArrayValue *ASTBuilder::CreateArrayValue(const SourceLocation &Loc, llvm::SmallVector<ASTValue *, 8> Values) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateArrayValue",
                      "Loc=" << Loc.getRawEncoding());
    ASTArrayValue *Array = new ASTArrayValue(Loc);
    Array->Values = std::move(Values);
    return Array;
}

ASTCharValue *ASTBuilder::CreateCharValue(const SourceLocation &Loc, llvm::StringRef Str) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateCharValue",
                      "Loc=" << Loc.getRawEncoding());
    return new ASTCharValue(Loc, Str);
}

ASTStringValue *ASTBuilder::CreateStringValue(const SourceLocation &Loc, llvm::StringRef Str) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateStringValue",
                      "Loc=" << Loc.getRawEncoding());
    return new ASTStringValue(Loc, Str);
}

ASTStructValue *ASTBuilder::CreateStructValue(const SourceLocation &Loc, llvm::StringMap<ASTValue *> Values) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateArrayValue",
                      "Loc=" << Loc.getRawEncoding());
    ASTStructValue *Struct = new ASTStructValue(Loc);
    Struct->Values = std::move(Values);
    return Struct;
}

/**
 * Creates an Param
 * @param Function
 * @param Loc
 * @param TypeRef
 * @param Name
 * @param Constant
 * @return
 */
ASTVar *ASTBuilder::CreateParam(const SourceLocation &Loc, ASTTypeRef *TypeRef, llvm::StringRef Name,
                                   llvm::SmallVector<ASTScope *, 8> &Scopes, ASTValue *DefaultValue) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateParam",
                      Logger().Attr("Loc", (uint64_t) Loc.getRawEncoding())
                      .Attr("TypeRef", TypeRef)
                      .Attr("Name", Name)
                      .AttrList("Scopes", Scopes)
                      .End());
    S.getValidator().CheckCreateParam(Loc, TypeRef, Name, Scopes);
    ASTVar *Param = new ASTVar(Loc, TypeRef, Name, Scopes);
    Param->Expr = CreateExpr(DefaultValue);
    return Param;
}

ASTVar *ASTBuilder::CreateErrorHandlerParam() {
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
ASTVar *ASTBuilder::CreateLocalVar(ASTBlockStmt *BlockStmt, const SourceLocation &Loc, ASTTypeRef *Type,
                                         llvm::StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLocalVar",
                      Logger().Attr("Name", Name)
                              .AttrList("Scopes", Scopes).End());
    S.getValidator().CheckCreateLocalVar(Loc, Type, Name, Scopes);
    ASTVar *Var = new ASTVar(Loc, Type, Name, Scopes);
    BlockStmt->getFunction()->LocalVars.push_back(Var); // Function Local var to be allocated
    BlockStmt->LocalVars.insert(std::make_pair(Var->getName(), Var)); // Check duplicate in Block Stmt
    return Var;
}

ASTIdentifier *ASTBuilder::CreateIdentifier(const SourceLocation &Loc, llvm::StringRef Name) {
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
ASTCall *ASTBuilder::CreateCall(ASTIdentifier *Identifier, llvm::SmallVector<ASTExpr *, 8> &Args, ASTCallKind CallKind,
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
ASTCall *ASTBuilder::CreateCall(ASTFunction *Function, llvm::SmallVector<ASTExpr *, 8> &Args) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateCall",
                      Logger().Attr("Function=", Function).End());
    ASTIdentifier *Identifier = CreateIdentifier(SourceLocation(), Function->Name);
    ASTCall *Call = CreateCall(Identifier, Args, ASTCallKind::CALL_FUNCTION);
    Call->Def = Function;
    Call->Resolved = true;
    return Call;
}

ASTCall *ASTBuilder::CreateCall(ASTFunction *Method, ASTCallKind CallKind) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateCall",
                      Logger().Attr("Method=", Method).End());
    ASTCall *Call = new ASTCall(SourceLocation(), Method->Name);
    Call->CallKind = CallKind;
    Call->Def = Method;
    return Call;
}

ASTCall *ASTBuilder::CreateCall(ASTIdentifier *Instance, ASTFunction *Method) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateCall",
                      Logger().Attr("Function=", Method).End());
    ASTCall *Call = new ASTCall(SourceLocation(), Method->getName());
    Call->Parent = Instance;
    Call->Def = Method;
    return Call;
}

ASTVarRef *ASTBuilder::CreateVarRef(ASTIdentifier *Identifier, ASTIdentifier *Parent) {
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

ASTVarRef *ASTBuilder::CreateVarRef(ASTVar *Var) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateVarRef",
                      Logger().Attr("Var", Var).End());
    ASTVarRef *VarRef = new ASTVarRef(Var->getLocation(), Var->getName());
    VarRef->Resolved = true;
    VarRef->Def = Var;
    return VarRef;
}

ASTValueExpr *ASTBuilder::CreateExpr(ASTValue *Value) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateExpr",
                      Logger().Attr("Value", Value).End());
    assert(Value && "Create ASTValueExpr by ASTValue");
    ASTValueExpr *Expr = new ASTValueExpr(Value);
    const SourceLocation &Loc = Value->getLocation();

    switch (Value->getTypeKind()) {

        case ASTValueKind::TYPE_BOOL:
            Expr->Type = CreateBoolType(Loc);
            break;

        case ASTValueKind::TYPE_INTEGER:
            Expr->Type = CreateIntType(Loc);
            break;

        case ASTValueKind::TYPE_FLOATING_POINT:
            Expr->Type = CreateFloatType(Loc);
            break;

        case ASTValueKind::TYPE_STRING:
            Expr->Type = CreateStringType(Loc);
            break;

        case ASTValueKind::TYPE_ARRAY:
            // TODO
            break;
        case ASTValueKind::TYPE_CLASS:
            // TODO
            break;
        case ASTValueKind::TYPE_VOID:
            break;
        case ASTValueKind::TYPE_ERROR:
            break;
    }
    return Expr;
}

ASTCallExpr *ASTBuilder::CreateExpr(ASTCall *Call) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateExpr",
                      Logger().Attr("Call", Call).End());
    ASTCallExpr *CallExpr = new ASTCallExpr(Call);
    return CallExpr;
}

ASTVarRefExpr *ASTBuilder::CreateExpr(ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateExpr",
                      Logger().Attr("VarRef", VarRef).End());
    ASTVarRefExpr *VarRefExpr = new ASTVarRefExpr(VarRef);
    return VarRefExpr;
}

ASTUnaryOpExpr *ASTBuilder::CreateUnaryOpExpr(const SourceLocation &Loc, ASTUnaryOpExprKind OpKind, ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateUnaryOpExpr",
                      Logger()
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding())
                      .Attr("OpKind", (uint64_t) OpKind)
                      .Attr("Expr", Expr)
                      .End());
    ASTUnaryOpExpr *UnaryOpExpr = new ASTUnaryOpExpr(Loc, OpKind, Expr);
    return UnaryOpExpr;
}

ASTBinaryOpExpr *ASTBuilder::CreateBinaryOpExpr(const SourceLocation &OpLocation, ASTBinaryOpExprKind OpKind,
                                                 ASTExpr *LeftExpr, ASTExpr *RightExpr) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateBinaryOpExpr", Logger()
                        .Attr("OpKind", (uint64_t) OpKind)
                        .Attr("OpLocation", (uint64_t) OpLocation.getRawEncoding())
                        .Attr("LeftExpr", LeftExpr)
                        .Attr("RightExpr", RightExpr).End());

    ASTBinaryOpExpr *BinaryOpExpr = new ASTBinaryOpExpr(OpKind, OpLocation, LeftExpr, RightExpr);
    return BinaryOpExpr;
}

ASTTernaryOpExpr *ASTBuilder::CreateTernaryOpExpr(ASTExpr *ConditionExpr,
                                                   const SourceLocation &TrueOpLocation, ASTExpr *TrueExpr,
                                                   const SourceLocation &FalseOpLocation, ASTExpr *FalseExpr) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateTernaryOpExpr", Logger()
                      .Attr("ConditionExpr", ConditionExpr)
                      .Attr("TrueOpLocation", (uint64_t) TrueOpLocation.getRawEncoding())
                      .Attr("TrueExpr", TrueExpr)
                      .Attr("FalseOpLocation", (uint64_t) FalseOpLocation.getRawEncoding())
                      .Attr("FalseExpr", FalseExpr).End());

    ASTTernaryOpExpr *TernaryExpr = new ASTTernaryOpExpr(ConditionExpr,
                                                         TrueOpLocation, TrueExpr,
                                                         FalseOpLocation, FalseExpr);

    return TernaryExpr;
}

/**
 * Creates a SemaBuilderStmt with an ASTVarAssign from ASTVarRef
 * @param Parent
 * @param VarRef
 * @return
 */
SemaBuilderStmt *ASTBuilder::CreateAssignmentStmt(ASTBlockStmt *Parent, ASTVarRef *VarRef, ASTAssignOperatorKind Kind) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateAssignmentStmt",
                      Logger().Attr("VarRef", VarRef).End());
    return SemaBuilderStmt::CreateAssignment(this, Parent, VarRef, Kind);
}

/**
 * Creates a SemaBuilderStmt with an ASTVarAssign from ASTVar
 * @param Parent
 * @param VarRef
 * @return
 */
SemaBuilderStmt *ASTBuilder::CreateAssignmentStmt(ASTBlockStmt *Parent, ASTVar *Var, ASTAssignOperatorKind Kind) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateAssignmentStmt",
                      Logger().Attr("Var", Var).End());
    ASTVarRef *VarRef = CreateVarRef(Var);
    return SemaBuilderStmt::CreateAssignment(this, Parent, VarRef, Kind);
}

/**
 * Creates a SemaBuilderStmt with ASTReturn
 * @param Parent
 * @param Loc
 * @return
 */
SemaBuilderStmt *ASTBuilder::CreateReturnStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLocalVar", Logger()
            .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    return SemaBuilderStmt::CreateReturn(this, Parent, Loc);
}

SemaBuilderStmt *ASTBuilder::CreateExprStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateExprStmt", Logger()
            .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    return SemaBuilderStmt::CreateExpr(this, Parent, Loc);
}

/**
 * Creates an SemaBuilderStmt with ASTFailStmt
 * @param Loc
 * @param ErrorHandler
 * @return
 */
SemaBuilderStmt *ASTBuilder::CreateFailStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateFailStmt", Logger()
            .Attr("Loc", (uint64_t)Loc.getRawEncoding()).End());
    return SemaBuilderStmt::CreateFail(this, Parent, Loc);
}

ASTHandleStmt *ASTBuilder::CreateHandleStmt(ASTBlockStmt *Parent, const SourceLocation &Loc,
    ASTBlockStmt *BlockStmt, ASTVarRef *ErrorRef) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateHandleStmt", Logger()
            .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());

        ASTHandleStmt *HandleStmt = new ASTHandleStmt(Loc);
        HandleStmt->ErrorHandlerRef = ErrorRef;
        HandleStmt->Parent = Parent;
        HandleStmt->Function = Parent->Function;
        HandleStmt->Handle = BlockStmt;
        Parent->Content.push_back(HandleStmt);

        // set Handle Block
        BlockStmt->Parent = HandleStmt;
        BlockStmt->Function = HandleStmt->Function;

        return HandleStmt;
}

ASTBreakStmt *ASTBuilder::CreateBreakStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateLocalVar", Logger()
            .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTBreakStmt *Break = new ASTBreakStmt(Loc);
    // Inner Stmt
    Parent->Content.push_back(Break);
    Break->Parent = Parent;
    Break->Function = Parent->Function;
    return Break;
}

ASTContinueStmt *ASTBuilder::CreateContinueStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateContinueStmt", Logger()
            .Attr("Loc", (uint64_t)Loc.getRawEncoding()).End());
    ASTContinueStmt *Continue = new ASTContinueStmt(Loc);
    // Inner Stmt
    Parent->Content.push_back(Continue);
    Continue->Parent = Parent;
    Continue->Function = Parent->Function;
    return Continue;
}

ASTDeleteStmt *ASTBuilder::CreateDeleteStmt(ASTBlockStmt *Parent, const SourceLocation &Loc, ASTVarRef *VarRef) {
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
ASTBuilder::CreateBody(ASTFunction *FunctionBase, ASTBlockStmt *Body) {
    FLY_DEBUG("SemaBuilder", "CreateBody");
    Body->Parent = nullptr; // body cannot have a parent stmt
    FunctionBase->Body = Body;
    FunctionBase->Body->Function = FunctionBase;
    return FunctionBase->Body;
}

ASTBlockStmt *ASTBuilder::CreateBlockStmt(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateBlockStmt", Logger()
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTBlockStmt *Block = new ASTBlockStmt(Loc);
    return Block;
}

ASTBlockStmt *ASTBuilder::CreateBlockStmt(ASTStmt *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("SemaBuilder", "CreateBlockStmt", Logger()
            .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
    ASTBlockStmt *Block = new ASTBlockStmt(Loc);
    Block->Parent = Parent;
    return Block;
}

SemaBuilderIfStmt *ASTBuilder::CreateIfBuilder(ASTBlockStmt *Parent) {
    FLY_DEBUG("SemaBuilder", "CreateIfBuilder");
    return SemaBuilderIfStmt::Create(S, Parent);
}

SemaBuilderSwitchStmt *ASTBuilder::CreateSwitchBuilder(ASTBlockStmt *Parent) {
    FLY_DEBUG("SemaBuilder", "CreateSwitchBuilder");
    return SemaBuilderSwitchStmt::Create(S, Parent);
}

SemaBuilderLoopStmt *ASTBuilder::CreateLoopBuilder(ASTBlockStmt *Parent, const SourceLocation &Loc) {
    FLY_DEBUG("SemaBuilder", "CreateLoopBuilder");
    return SemaBuilderLoopStmt::Create(S, Parent, Loc);
}
