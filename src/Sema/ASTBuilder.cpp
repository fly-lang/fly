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
#include "AST/ASTBreakStmt.h"
#include "AST/ASTContinueStmt.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTDeleteStmt.h"
#include "AST/ASTVar.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTHandleStmt.h"
#include "AST/ASTVarStmt.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTValue.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
#include "AST/ASTExpr.h"
#include "AST/ASTOpExpr.h"
#include "AST/ASTNameSpaceRef.h"
#include "Basic/SourceLocation.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"

#include <utility>
#include <AST/ASTTypeRef.h>
#include <Sym/SymFunctionBase.h>

using namespace fly;

const uint8_t ASTBuilder::DEFAULT_INTEGER_RADIX= 10;
/**
 * Private constructor used only from Sema constructor
 * @param S
 */
ASTBuilder::ASTBuilder(Sema &S) : S(S) {

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
    FLY_DEBUG_MESSAGE("ASTBuilder", "GenerateModule", "Name=" << Name);

	if (Name.empty()) {
		S.Diag(diag::err_sema_module_name_empty);
	}

    uint64_t Id = ModuleIdCounter++; // FIXME compare Module by using FileID
    ASTModule *Module = new ASTModule(Id, Name, false);
	S.Modules.push_back(Module);

	FLY_DEBUG_END("ASTBuilder", "CreateModule");
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
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateHeaderModule", "Name=" << Name);

    uint64_t Id = ModuleIdCounter++; // FIXME compare Module by using FileID
    ASTModule *Module = new ASTModule(Id, Name, true);

	FLY_DEBUG_END("ASTBuilder", "CreateHeaderModule");
	return Module;
}

ASTComment *ASTBuilder::CreateComment(ASTModule *Module, const SourceLocation &Loc, llvm::StringRef Content) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateComment", "Loc" << Loc.getRawEncoding() << " Content=" << Content);
	ASTComment *Comment = new ASTComment(Loc, Content);

	// Add Comment to Module
	Module->Definitions.push_back(Comment);

	FLY_DEBUG_END("ASTBuilder", "CreateHeaderModule");
	return Comment;
}

ASTNameSpace * ASTBuilder::CreateNameSpace(const SourceLocation &Loc, llvm::StringRef Name, ASTModule *Module) {
	if (!Name.empty()) {
		S.Diag(Loc, diag::err_sema_namespace_empty);
	}

	llvm::SmallVector<llvm::StringRef, 4> Names;
	Names.push_back(Name);
	ASTNameSpace *NS = new ASTNameSpace(Loc, Names);

	// Add NameSpace to Module
	if (Module)
		Module->NameSpace = NS;

	return NS;
}

/**
 * Create a NameSpace for each parent
 * NS3.NS2.NS1 -> Identifier->Parent
 * NS2.NS1 -> Identifier->Parent->Parent
 * NS3 -> Identifier->Parent->Parent->Parent ... until to Root
 * @param Identifier
 * @return
 */
ASTNameSpace *ASTBuilder::CreateNameSpace(const SourceLocation &Loc, llvm::SmallVector<llvm::StringRef, 4> &Names, ASTModule *Module) {
    FLY_DEBUG_START("ASTBuilder", "CreateNameSpace");

	// Check Name not empty
	if (Names.empty()) {
		S.Diag(Loc, diag::err_sema_namespace_empty);
	}

	// Create NameSpace with parent
	ASTNameSpace *NS = new ASTNameSpace(Loc, Names);
	Module->NameSpace = NS;

	FLY_DEBUG_END("ASTBuilder", "CreateNameSpace");
    return NS;
}

ASTImport *ASTBuilder::CreateImport(ASTModule *Module, const SourceLocation &Loc, llvm::StringRef Name, ASTAlias *Alias) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateImport", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	llvm::SmallVector<llvm::StringRef, 4> Names;
	Names.push_back(Name);

	FLY_DEBUG_END("ASTBuilder", "CreateImport");
	return CreateImport(Module, Loc, Names, Alias);
}

ASTImport *ASTBuilder::CreateImport(ASTModule *Module, const SourceLocation &Loc, llvm::SmallVector<llvm::StringRef, 4> &Names, ASTAlias *Alias) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateImport", "Loc=" << Loc.getRawEncoding() << ", Name=" << Names[0]);

    ASTImport *Import = new ASTImport(Loc, Names);

    // Add Import to Module
	Module->Definitions.push_back(Import);

    if (Alias) {
        Import->setAlias(Alias);
    }

	FLY_DEBUG_END("ASTBuilder", "CreateImport");
    return Import;
}

ASTAlias * ASTBuilder::CreateAlias(const SourceLocation &Loc, llvm::StringRef Name) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateAlias", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTAlias * Alias = new ASTAlias(Loc, Name);

	FLY_DEBUG_END("ASTBuilder", "CreateImport");
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
ASTVar *ASTBuilder::CreateGlobalVar(ASTModule *Module, const SourceLocation &Loc, ASTTypeRef *Type,
                                           const llvm::StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes,
                                           ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateGlobalVar", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

    ASTVar *GlobalVar = new ASTVar(Loc, Type, Name, Scopes);
    GlobalVar->Expr = Expr;

	// Global Var to Module
	Module->Definitions.push_back(GlobalVar);

	FLY_DEBUG_END("ASTBuilder", "CreateGlobalVar");
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
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateFunction", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

    ASTFunction *Function = new ASTFunction(Loc, TypeRef, Scopes, Name, Params);

    // Create Body
    if (Body)
        CreateBody(Function, Body);

	// Add Function to Module
	Module->Definitions.push_back(Function);

	FLY_DEBUG_END("ASTBuilder", "CreateFunction");
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
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateClass", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

    ASTClass *Class = new ASTClass(Module, ASTClassKind::CLASS, Scopes, Loc, Name, SuperClasses);

	// Add Class to Module
	Module->Definitions.push_back(Class);

	FLY_DEBUG_END("ASTBuilder", "CreateClass");
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
ASTVar *ASTBuilder::CreateClassAttribute(const SourceLocation &Loc, ASTClass *Class, ASTTypeRef *TypeRef,
                                                     llvm::StringRef Name, SmallVector<ASTScope *, 8> &Scopes,
                                                     ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateClassAttribute", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

    ASTVar *Attribute = new ASTVar(Loc, TypeRef, Name, Scopes);
    Attribute->Expr = Expr;
    Class->Definitions.push_back(Attribute);

	FLY_DEBUG_END("ASTBuilder", "CreateClassAttribute");
    return Attribute;
}

ASTFunction *ASTBuilder::CreateClassMethod(const SourceLocation &Loc, ASTClass *Class, ASTTypeRef *TypeRef,
                                               llvm::StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes,
                                               llvm::SmallVector<ASTVar *, 8> &Params, ASTBlockStmt *Body) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateClassMethod", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

    ASTFunction *Method = new ASTFunction(Loc, TypeRef, Scopes, Name, Params);

    if (Body)
        CreateBody(Method, Body);

    // Add to Class Methods
    Class->Definitions.push_back(Method);

	FLY_DEBUG_END("ASTBuilder", "CreateClassMethod");
    return Method;
}

ASTEnum *ASTBuilder::CreateEnum(ASTModule *Module, const SourceLocation &Loc, const llvm::StringRef Name,
                                 llvm::SmallVector<ASTScope *, 8> &Scopes,
                                 llvm::SmallVector<ASTTypeRef *, 4> EnumTypes) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateEnum", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

    ASTEnum *Enum = new ASTEnum(Module, Loc, Name, Scopes, EnumTypes);

    // Add Enum to Module
	Module->Definitions.push_back(Enum);

	FLY_DEBUG_END("ASTBuilder", "CreateEnum");
    return Enum;
}

ASTVar *ASTBuilder::CreateEnumEntry(const SourceLocation &Loc, ASTEnum *Enum, llvm::StringRef Name,
                                           llvm::SmallVector<ASTScope *, 8> &Scopes) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateEnumEntry", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTTypeRef * TypeRef = CreateIntTypeRef(Loc);
    ASTVar *EnumEntry = new ASTVar(Loc, TypeRef, Name, Scopes);
    Enum->Definitions.push_back(EnumEntry);

	FLY_DEBUG_END("ASTBuilder", "CreateEnumEntry");
    return EnumEntry;
}

/**
 * Creates a bool type
 * @param Loc
 * @return
 */
ASTTypeRef *ASTBuilder::CreateBoolTypeRef(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateBoolTypeRef", "Loc=" << Loc.getRawEncoding());

    ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("bool"));
	TypeRef->Type = S.getSymTable().getBoolType();

	FLY_DEBUG_END("ASTBuilder", "CreateBoolTypeRef");
	return TypeRef;
}

/**
 * Creates an byte type
 * @param Loc
 * @return
 */
ASTTypeRef *ASTBuilder::CreateByteTypeRef(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateByteTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("byte"));
	TypeRef->Type = S.getSymTable().getByteType();

	FLY_DEBUG_END("ASTBuilder", "CreateByteTypeRef");
	return TypeRef;
}

/**
 * Creates an unsigned short type
 * @param Loc
 * @return
 */
ASTTypeRef *ASTBuilder::CreateUShortTypeRef(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateUShortTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("ushort"));
	TypeRef->Type = S.getSymTable().getUShortType();

	FLY_DEBUG_END("ASTBuilder", "CreateUShortTypeRef");
	return TypeRef;
}

/**
 * Create a short type
 * @param Loc
 * @return
 */
ASTTypeRef *ASTBuilder::CreateShortTypeRef(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateShortTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("short"));
	TypeRef->Type = S.getSymTable().getShortType();

	FLY_DEBUG_END("ASTBuilder", "CreateShortTypeRef");
	return TypeRef;
}

/**
 * Creates an unsigned int type
 * @param Loc
 * @return
 */
ASTTypeRef *ASTBuilder::CreateUIntTypeRef(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateUIntTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("uint"));
	TypeRef->Type = S.getSymTable().getUIntType();

	FLY_DEBUG_END("ASTBuilder", "CreateUIntTypeRef");
	return TypeRef;
}

/**
 * Creates an int type
 * @param Loc
 * @return
 */
ASTTypeRef *ASTBuilder::CreateIntTypeRef(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateIntTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("int"));
	TypeRef->Type = S.getSymTable().getIntType();

	FLY_DEBUG_END("ASTBuilder", "CreateIntTypeRef");
	return TypeRef;
}

/**
 * Creates an unsigned long type
 * @param Loc
 * @return
 */
ASTTypeRef *ASTBuilder::CreateULongTypeRef(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateULongTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("ulong"));
	TypeRef->Type = S.getSymTable().getULongType();

	FLY_DEBUG_END("ASTBuilder", "CreateULongTypeRef");
	return TypeRef;
}

/**
 * Creates a long type
 * @param Loc
 * @return
 */
ASTTypeRef *ASTBuilder::CreateLongTypeRef(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateLongTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("long"));
	TypeRef->Type = S.getSymTable().getLongType();

	FLY_DEBUG_END("ASTBuilder", "CreateLongTypeRef");
	return TypeRef;
}

/**
 * Creates a float type
 * @param Loc
 * @return
 */
ASTTypeRef *ASTBuilder::CreateFloatTypeRef(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateFloatTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("float"));
	TypeRef->Type = S.getSymTable().getFloatType();

	FLY_DEBUG_END("ASTBuilder", "CreateFloatTypeRef");
	return TypeRef;
}

/**
 * Creates a double type
 * @param Loc
 * @return
 */
ASTTypeRef *ASTBuilder::CreateDoubleTypeRef(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateDoubleTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("double"));
	TypeRef->Type = S.getSymTable().getDoubleType();

	FLY_DEBUG_END("ASTBuilder", "CreateDoubleTypeRef");
	return TypeRef;
}

/**
 * Creates a void type
 * @param Loc
 * @return
 */
ASTTypeRef *ASTBuilder::CreateVoidTypeRef(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateVoidTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("void"));
	TypeRef->Type = S.getSymTable().getVoidType();

	FLY_DEBUG_END("ASTBuilder", "CreateVoidTypeRef");
	return TypeRef;
}

ASTTypeRef *ASTBuilder::CreateCharTypeRef(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateCharTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("char"));
	TypeRef->Type = S.getSymTable().getCharType();

	FLY_DEBUG_END("ASTBuilder", "CreateCharTypeRef");
	return TypeRef;
}

ASTTypeRef *ASTBuilder::CreateStringTypeRef(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateStringTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("string"));
	TypeRef->Type = S.getSymTable().getStringType();

	FLY_DEBUG_END("ASTBuilder", "CreateStringTypeRef");
	return TypeRef;
}

ASTTypeRef *ASTBuilder::CreateErrorTypeRef(const SourceLocation &Loc) {
    FLY_DEBUG_START("ASTBuilder", "CreateErrorTypeRef");

	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, llvm::StringRef("error"));
	TypeRef->Type = S.getSymTable().getErrorType();

	FLY_DEBUG_END("ASTBuilder", "CreateErrorTypeRef");
	return TypeRef;
}

/**
 * Creates an array type
 * @param Loc
 * @param TypeRef
 * @param Size
 * @return
 */
ASTArrayTypeRef *ASTBuilder::CreateArrayTypeRef(const SourceLocation &Loc, ASTTypeRef *TypeRef, ASTExpr *Size) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateArrayTypeRef", "Loc=" << Loc.getRawEncoding());

	// TODO Size
	ASTArrayTypeRef * ArrayTypeRef = new ASTArrayTypeRef(Loc, TypeRef, llvm::StringRef("array"));
	ArrayTypeRef->Size = Size;

	FLY_DEBUG_END("ASTBuilder", "CreateArrayTypeRef");
	return ArrayTypeRef;

}

ASTTypeRef * ASTBuilder::CreateTypeRef(const SourceLocation &Loc, llvm::StringRef Name, ASTNameSpaceRef *NameSpaceRef) {
	FLY_DEBUG_START("ASTBuilder", "CreateTypeRef");

	ASTTypeRef * TypeRef = new ASTTypeRef(Loc, Name, NameSpaceRef, false);

	FLY_DEBUG_END("ASTBuilder", "CreateTypeRef");
	return TypeRef;
}

ASTTypeRef * ASTBuilder::CreateTypeRef(ASTClass *Class) {
	FLY_DEBUG_START("ASTBuilder", "CreateTypeRef");

	ASTTypeRef * TypeRef = new ASTTypeRef(Class->getLocation(), Class->getName(), nullptr, false);

	FLY_DEBUG_END("ASTBuilder", "CreateTypeRef");
	return TypeRef;
}

ASTTypeRef * ASTBuilder::CreateTypeRef(ASTEnum *Enum) {
	FLY_DEBUG_START("ASTBuilder", "CreateTypeRef");

	ASTTypeRef * TypeRef = new ASTTypeRef(Enum->getLocation(), Enum->getName(), nullptr, false);

	FLY_DEBUG_END("ASTBuilder", "CreateTypeRef");
	return TypeRef;
}

/**
 * Creates an ASTScope
 * @param Loc
 * @param Name
 * @return
 */
ASTNameSpaceRef *ASTBuilder::CreateNameSpaceRef(const SourceLocation &Loc, llvm::SmallVector<llvm::StringRef, 4> &Names) {
	FLY_DEBUG_START("ASTBuilder", "CreateNameSpaceRef");

	ASTNameSpaceRef * NameSpaceRef = new ASTNameSpaceRef(Loc, Names);

	FLY_DEBUG_END("ASTBuilder", "CreateNameSpaceRef");
	return NameSpaceRef;
}

/**
 * Creates a default value by type
 * @param Type
 * @return
 */
ASTValue *ASTBuilder::CreateDefaultValue(SymType *Type) {
	FLY_DEBUG_START("ASTBuilder", "CreateDefaultValue");

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

	FLY_DEBUG_END("ASTBuilder", "CreateDefaultValue");
	return Value;
}

/**
 * Creates a null value
 * @param Loc
 * @return
 */
ASTNullValue *ASTBuilder::CreateNullValue(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateNullValue", "Loc=" << Loc.getRawEncoding());

    ASTNullValue * Value = new ASTNullValue(Loc);

	FLY_DEBUG_END("ASTBuilder", "CreateNullValue");
	return Value;
}

/**
 * Creates a zero value
 * @param Loc
 * @return
 */
ASTZeroValue *ASTBuilder::CreateZeroValue(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateZeroValue", "Loc=" << Loc.getRawEncoding());

    ASTZeroValue * Value = new ASTZeroValue(Loc);

	FLY_DEBUG_END("ASTBuilder", "CreateZeroValue");
	return Value;
}

/**
 * Creates a bool value
 * @param Loc
 * @param Val
 * @return
 */
ASTBoolValue *ASTBuilder::CreateBoolValue(const SourceLocation &Loc, bool Val) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateBoolValue", "Loc=" << Loc.getRawEncoding() << ", Val=" << Val);

    ASTBoolValue *Value = new ASTBoolValue(Loc, Val);

	FLY_DEBUG_END("ASTBuilder", "CreateBoolValue");
	return Value;
}

/**
 * Creates an integer value
 * @param Loc
 * @param Val
 * @param Negative
 * @return
 */
ASTIntegerValue *ASTBuilder::CreateIntegerValue(const SourceLocation &Loc, llvm::StringRef Val, uint8_t Radix) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateIntegerValue",
                      "Loc=" << Loc.getRawEncoding() << ", Val=" << Val << " Radix=" << Radix);

    ASTIntegerValue * Value = new ASTIntegerValue(Loc, Val, Radix);

	FLY_DEBUG_END("ASTBuilder", "CreateIntegerValue");
	return Value;
}

ASTIntegerValue *ASTBuilder::CreateIntegerValue(const SourceLocation &Loc, llvm::StringRef Val) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateIntegerValue", "Loc=" << Loc.getRawEncoding() << ", Val=" << Val);

    ASTIntegerValue * Value = new ASTIntegerValue(Loc, Val, DEFAULT_INTEGER_RADIX);

	FLY_DEBUG_END("ASTBuilder", "CreateIntegerValue");
	return Value;
}

/**
 * Creates a floating point value
 * @param Loc
 * @param Val
 * @return
 */
ASTFloatingValue *ASTBuilder::CreateFloatingValue(const SourceLocation &Loc, llvm::StringRef Val) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateFloatingValue", "Loc=" << Loc.getRawEncoding() << ", Val=" << Val);

    ASTFloatingValue * Value = new ASTFloatingValue(Loc, Val);

	FLY_DEBUG_END("ASTBuilder", "CreateFloatingValue");
	return Value;
}

/**
 * Create an array value
 * @param Loc
 * @return
 */
ASTArrayValue *ASTBuilder::CreateArrayValue(const SourceLocation &Loc, llvm::SmallVector<ASTValue *, 8> Values) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateArrayValue", "Loc=" << Loc.getRawEncoding());

    ASTArrayValue *Array = new ASTArrayValue(Loc);
    Array->Values = std::move(Values);

	FLY_DEBUG_END("ASTBuilder", "CreateArrayValue");
    return Array;
}

ASTCharValue *ASTBuilder::CreateCharValue(const SourceLocation &Loc, llvm::StringRef Val) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateCharValue", "Loc=" << Loc.getRawEncoding() << ", Val=" << Val);

    ASTCharValue * Value = new ASTCharValue(Loc, Val);

	FLY_DEBUG_END("ASTBuilder", "CreateCharValue");
	return Value;
}

ASTStringValue *ASTBuilder::CreateStringValue(const SourceLocation &Loc, llvm::StringRef Val) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateStringValue", "Loc=" << Loc.getRawEncoding() << ", Val=" << Val);

    ASTStringValue * Value = new ASTStringValue(Loc, Val);

	FLY_DEBUG_END("ASTBuilder", "CreateStringValue");
	return Value;
}

ASTStructValue *ASTBuilder::CreateStructValue(const SourceLocation &Loc, llvm::StringMap<ASTValue *> Values) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateArrayValue", "Loc=" << Loc.getRawEncoding());

    ASTStructValue *Struct = new ASTStructValue(Loc);
    Struct->Values = std::move(Values);

	FLY_DEBUG_END("ASTBuilder", "CreateStructValue");
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
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateParam", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

    ASTVar *Param = new ASTVar(Loc, TypeRef, Name, Scopes);
    Param->Expr = CreateExpr(DefaultValue);

	FLY_DEBUG_END("ASTBuilder", "CreateParam");
    return Param;
}

/**
 * Creates an ASTVar
 * @param Parent
 * @param Loc
 * @param Type
 * @param Name
 * @param Constant
 * @return
 */
ASTVar *ASTBuilder::CreateLocalVar(ASTBlockStmt *BlockStmt, const SourceLocation &Loc, ASTTypeRef *Type,
                                         llvm::StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateLocalVar", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

    ASTVar *Var = new ASTVar(Loc, Type, Name, Scopes);
    BlockStmt->LocalVars.insert(std::make_pair(Var->getName(), Var)); // Check duplicate in Block Stmt

	FLY_DEBUG_END("ASTBuilder", "CreateLocalVar");
    return Var;
}

/**
 * Create an ASTFunctionCall without definition
 * @param Location
 * @param Name
 * @param NameSpace
 * @return
 */
ASTCall *ASTBuilder::CreateCall(const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTExpr *, 8> &Args, ASTCallKind CallKind,
                                 ASTRef *Parent) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateCall", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

    ASTCall *Call = new ASTCall(Loc, Name);
    Call->CallKind = CallKind;
    if (Parent) { // Take Parent
        Parent->AddChild(Call);
    	Call->Parent = Parent;
    }
    uint64_t i = 0;
    for (auto &Expr : Args) {
        ASTArg *Arg = new ASTArg(Expr, i++);
        Call->Args.push_back(Arg);
    }

	FLY_DEBUG_END("ASTBuilder", "CreateCall");
    return Call;
}

/**
 * Creates an ASTFunctionCall with definition
 * @param Stmt
 * @param Function
 * @return
 */
ASTCall *ASTBuilder::CreateCall(llvm::StringRef Name, llvm::SmallVector<ASTExpr *, 8> &Args) {
	FLY_DEBUG_START("ASTBuilder", "CreateCall");

	const SourceLocation &Loc = SourceLocation();
	ASTCall *Call = CreateCall(Loc, Name, Args, ASTCallKind::CALL_FUNCTION);

	FLY_DEBUG_END("ASTBuilder", "CreateCall");
	return Call;
}

ASTCall *ASTBuilder::CreateCall(ASTRef *Instance, llvm::StringRef Name, llvm::SmallVector<ASTExpr *, 8> &Args) {
    FLY_DEBUG_START("ASTBuilder", "CreateCall");

    ASTCall *Call = CreateCall(Instance->getLocation(), Name, Args, ASTCallKind::CALL_FUNCTION);
    Call->Parent = Instance;

	FLY_DEBUG_END("ASTBuilder", "CreateCall");
    return Call;
}

ASTVarRef *ASTBuilder::CreateVarRef(ASTRef *Ref) {
    FLY_DEBUG_START("ASTBuilder", "CreateVarRef");

    ASTVarRef *VarRef = new ASTVarRef(Ref->getLocation(), Ref->getName());
    if (Ref->getParent()) { // Take Parent
        Ref->Parent->AddChild(VarRef);
    } else { // Do a copy
        VarRef->Parent = Ref->getParent();
    }
	VarRef->Child = Ref->Child;
    delete Ref;

	FLY_DEBUG_END("ASTBuilder", "CreateVarRef");
    return VarRef;
}

ASTVarRef *ASTBuilder::CreateVarRef(ASTVar *Var, ASTRef *Parent) {
	FLY_DEBUG_START("ASTBuilder", "CreateVarRef");

	ASTVarRef *VarRef = new ASTVarRef(Var->getLocation(), Var->getName());
	if (Parent) {
		VarRef->Parent = Parent;
	}
	VarRef->Resolved = true;
	VarRef->Var = &Var->Sym;

	FLY_DEBUG_END("ASTBuilder", "CreateVarRef");
	return VarRef;
}

ASTRef *ASTBuilder::CreateUndefinedRef(const SourceLocation &Loc, llvm::StringRef Name, ASTRef *Parent) {
	FLY_DEBUG_START("ASTBuilder", "CreateUndefinedRef");

	ASTRef *VarRef = new ASTRef(Loc, Name, ASTRefKind::REF_UNDEFINED);
	VarRef->Parent = Parent;

	FLY_DEBUG_END("ASTBuilder", "CreateUndefinedRef");
	return VarRef;
}

ASTValueExpr *ASTBuilder::CreateExpr(ASTValue *Value) {
    FLY_DEBUG_START("ASTBuilder", "CreateExpr");
    assert(Value && "Create ASTValueExpr by ASTValue");

    ASTValueExpr *Expr = new ASTValueExpr(Value);
    const SourceLocation &Loc = Value->getLocation();

    switch (Value->getTypeKind()) {

        case ASTValueKind::VAL_BOOL:
            Expr->TypeRef = CreateBoolTypeRef(Loc);
            break;

        case ASTValueKind::VAL_INT:
            Expr->TypeRef = CreateIntTypeRef(Loc);
            break;

        case ASTValueKind::VAL_FLOAT:
            Expr->TypeRef = CreateFloatTypeRef(Loc);
            break;

        case ASTValueKind::VAL_STRING:
            Expr->TypeRef = CreateStringTypeRef(Loc);
            break;

    	case ASTValueKind::VAL_CHAR:
    		break;
        case ASTValueKind::VAL_ARRAY:
            // TODO
            break;
        case ASTValueKind::VAL_NULL:
            // TODO
            break;
        case ASTValueKind::VAL_ZERO:
            break;
        case ASTValueKind::VAL_STRUCT:
            break;
    }

	FLY_DEBUG_END("ASTBuilder", "CreateExpr");
    return Expr;
}

ASTCallExpr *ASTBuilder::CreateExpr(ASTCall *Call) {
    FLY_DEBUG_START("ASTBuilder", "CreateExpr");

    ASTCallExpr *CallExpr = new ASTCallExpr(Call);

	FLY_DEBUG_END("ASTBuilder", "CreateExpr");
    return CallExpr;
}

ASTVarRefExpr *ASTBuilder::CreateExpr(ASTVarRef *VarRef) {
    FLY_DEBUG_START("ASTBuilder", "CreateExpr");

    ASTVarRefExpr *VarRefExpr = new ASTVarRefExpr(VarRef);

	FLY_DEBUG_END("ASTBuilder", "CreateExpr");
    return VarRefExpr;
}

ASTUnaryOpExpr *ASTBuilder::CreateUnaryOpExpr(const SourceLocation &Loc, ASTUnaryOpExprKind OpKind, ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateUnaryOpExpr", "Loc=" << Loc.getRawEncoding() << ", OpKind=" << static_cast<uint8_t>(OpKind));

    ASTUnaryOpExpr *UnaryOpExpr = new ASTUnaryOpExpr(Loc, OpKind, Expr);

	FLY_DEBUG_END("ASTBuilder", "CreateUnaryOpExpr");
    return UnaryOpExpr;
}

ASTBinaryOpExpr *ASTBuilder::CreateBinaryOpExpr(const SourceLocation &OpLocation, ASTBinaryOpExprKind OpKind,
                                                 ASTExpr *LeftExpr, ASTExpr *RightExpr) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateBinaryOpExpr", "OpLocation=" << OpLocation.getRawEncoding() << ", OpKind=" << static_cast<uint8_t>(OpKind));

    ASTBinaryOpExpr *BinaryOpExpr = new ASTBinaryOpExpr(OpKind, OpLocation, LeftExpr, RightExpr);

	FLY_DEBUG_END("ASTBuilder", "CreateBinaryOpExpr");
    return BinaryOpExpr;
}

ASTTernaryOpExpr *ASTBuilder::CreateTernaryOpExpr(ASTExpr *ConditionExpr,
                                                   const SourceLocation &TrueOpLocation, ASTExpr *TrueExpr,
                                                   const SourceLocation &FalseOpLocation, ASTExpr *FalseExpr) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateBinaryOpExpr",
		"TrueOpLocation=" << TrueOpLocation.getRawEncoding() << "FalseOpLocation=" << FalseOpLocation.getRawEncoding());

    ASTTernaryOpExpr *TernaryExpr = new ASTTernaryOpExpr(ConditionExpr,
                                                         TrueOpLocation, TrueExpr,
                                                         FalseOpLocation, FalseExpr);

	FLY_DEBUG_END("ASTBuilder", "CreateTernaryOpExpr");
    return TernaryExpr;
}

/**
 * Creates a SemaBuilderStmt with an ASTVarAssign from ASTVarRef
 * @param Parent
 * @param VarRef
 * @return
 */
SemaBuilderStmt *ASTBuilder::CreateAssignmentStmt(ASTBlockStmt *Parent, ASTVarRef *VarRef, ASTAssignOperatorKind Kind) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateAssignmentStmt", "Kind=" << static_cast<uint8_t>(Kind));

    SemaBuilderStmt * B = SemaBuilderStmt::CreateAssignment(this, Parent, VarRef, Kind);

	FLY_DEBUG_END("ASTBuilder", "CreateAssignmentStmt");
	return B;
}

/**
 * Creates a SemaBuilderStmt with an ASTVarAssign from ASTVar
 * @param Parent
 * @param VarRef
 * @return
 */
SemaBuilderStmt *ASTBuilder::CreateAssignmentStmt(ASTBlockStmt *Parent, ASTVar *Var, ASTAssignOperatorKind Kind) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateAssignmentStmt", "Kind=" << static_cast<uint8_t>(Kind));

    ASTVarRef *VarRef = CreateVarRef(Var);
    SemaBuilderStmt * B = SemaBuilderStmt::CreateAssignment(this, Parent, VarRef, Kind);

	FLY_DEBUG_END("ASTBuilder", "CreateAssignmentStmt");
	return B;
}

/**
 * Creates a SemaBuilderStmt with ASTReturn
 * @param Parent
 * @param Loc
 * @return
 */
SemaBuilderStmt *ASTBuilder::CreateReturnStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateLocalVar", "Loc=" << Loc.getRawEncoding());

    SemaBuilderStmt * B = SemaBuilderStmt::CreateReturn(this, Parent, Loc);

	FLY_DEBUG_END("ASTBuilder", "CreateLocalVar");
	return B;
}

SemaBuilderStmt *ASTBuilder::CreateExprStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateExprStmt", "Loc=" << Loc.getRawEncoding());

    SemaBuilderStmt * B = SemaBuilderStmt::CreateExpr(this, Parent, Loc);

	FLY_DEBUG_END("ASTBuilder", "CreateExprStmt");
	return B;
}

/**
 * Creates an SemaBuilderStmt with ASTFailStmt
 * @param Loc
 * @param ErrorHandler
 * @return
 */
SemaBuilderStmt *ASTBuilder::CreateFailStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateFailStmt", "Loc=" << Loc.getRawEncoding());

    SemaBuilderStmt * FailStmt = SemaBuilderStmt::CreateFail(this, Parent, Loc);

	FLY_DEBUG_END("ASTBuilder", "CreateFailStmt");
	return FailStmt;
}

ASTHandleStmt *ASTBuilder::CreateHandleStmt(ASTBlockStmt *Parent, const SourceLocation &Loc,
    ASTBlockStmt *BlockStmt, ASTVarRef *ErrorRef) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateHandleStmt", "Loc=" << Loc.getRawEncoding());

    ASTHandleStmt *HandleStmt = new ASTHandleStmt(Loc);
    HandleStmt->ErrorHandlerRef = ErrorRef;
    HandleStmt->Parent = Parent;
    HandleStmt->Function = Parent->Function;
    HandleStmt->Handle = BlockStmt;
    Parent->Content.push_back(HandleStmt);

    // set Handle Block
    BlockStmt->Parent = HandleStmt;
    BlockStmt->Function = HandleStmt->Function;

	FLY_DEBUG_END("ASTBuilder", "CreateHandleStmt");
	return HandleStmt;
}

ASTBreakStmt *ASTBuilder::CreateBreakStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateLocalVar", "Loc=" << Loc.getRawEncoding());

    ASTBreakStmt *Break = new ASTBreakStmt(Loc);
    // Inner Stmt
    Parent->Content.push_back(Break);
    Break->Parent = Parent;
    Break->Function = Parent->Function;

	FLY_DEBUG_END("ASTBuilder", "CreateBreakStmt");
    return Break;
}

ASTContinueStmt *ASTBuilder::CreateContinueStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateContinueStmt", "Loc=" << Loc.getRawEncoding());

    ASTContinueStmt *Continue = new ASTContinueStmt(Loc);
    // Inner Stmt
    Parent->Content.push_back(Continue);
    Continue->Parent = Parent;
    Continue->Function = Parent->Function;

	FLY_DEBUG_END("ASTBuilder", "CreateContinueStmt");
    return Continue;
}

ASTDeleteStmt *ASTBuilder::CreateDeleteStmt(ASTBlockStmt *Parent, const SourceLocation &Loc, ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateDeleteStmt", "Loc=" << Loc.getRawEncoding());

    ASTDeleteStmt *Delete = new ASTDeleteStmt(Loc, VarRef);
    // Inner Stmt
    Parent->Content.push_back(Delete);
    Delete->Parent = Parent;
    Delete->Function = Parent->Function;

	FLY_DEBUG_END("ASTBuilder", "CreateDeleteStmt");
    return Delete;
}

ASTBlockStmt*
ASTBuilder::CreateBody(ASTFunction *FunctionBase, ASTBlockStmt *Body) {
    FLY_DEBUG_START("ASTBuilder", "CreateBody");

    Body->Parent = nullptr; // body cannot have a parent stmt
    FunctionBase->Body = Body;
    FunctionBase->Body->Function = FunctionBase;

	FLY_DEBUG_END("ASTBuilder", "CreateBody");
    return FunctionBase->Body;
}

ASTBlockStmt *ASTBuilder::CreateBlockStmt(const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateBlockStmt", "Loc=" << Loc.getRawEncoding());

    ASTBlockStmt *Block = new ASTBlockStmt(Loc);

	FLY_DEBUG_END("ASTBuilder", "CreateBlockStmt");
    return Block;
}

ASTBlockStmt *ASTBuilder::CreateBlockStmt(ASTStmt *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateBlockStmt", "Loc=" << Loc.getRawEncoding());

    ASTBlockStmt *Block = new ASTBlockStmt(Loc);
    Block->Parent = Parent;

	FLY_DEBUG_END("ASTBuilder", "CreateBlockStmt");
    return Block;
}

SemaBuilderIfStmt *ASTBuilder::CreateIfBuilder(ASTBlockStmt *Parent) {
    FLY_DEBUG_START("ASTBuilder", "CreateIfBuilder");

    SemaBuilderIfStmt * B = SemaBuilderIfStmt::Create(S, Parent);

	FLY_DEBUG_END("ASTBuilder", "CreateIfBuilder");
	return B;
}

SemaBuilderSwitchStmt *ASTBuilder::CreateSwitchBuilder(ASTBlockStmt *Parent) {
    FLY_DEBUG_START("ASTBuilder", "CreateSwitchBuilder");

    SemaBuilderSwitchStmt * B = SemaBuilderSwitchStmt::Create(S, Parent);

	FLY_DEBUG_END("ASTBuilder", "CreateSwitchBuilder");
	return B;
}

SemaBuilderLoopStmt *ASTBuilder::CreateLoopBuilder(ASTBlockStmt *Parent, const SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("ASTBuilder", "CreateLoopBuilder", "Loc=" << Loc.getRawEncoding());

    SemaBuilderLoopStmt * B = SemaBuilderLoopStmt::Create(S, Parent, Loc);

	FLY_DEBUG_END("ASTBuilder", "CreateLoopBuilder");
	return B;
}
