//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTBuilder.cpp - The AST Builder
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTBuilder.h"

#include "AST/ASTArg.h"
#include "AST/ASTBinary.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTBreakStmt.h"
#include "AST/ASTCall.h"
#include "AST/ASTClass.h"
#include "AST/ASTComment.h"
#include "AST/ASTContinueStmt.h"
#include "AST/ASTDeleteStmt.h"
#include "AST/ASTEnum.h"
#include "AST/ASTExpr.h"
#include "AST/ASTFunction.h"
#include "AST/ASTHandleStmt.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTImport.h"
#include "AST/ASTModule.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTTernary.h"
#include "AST/ASTType.h"
#include "AST/ASTUnary.h"
#include "AST/ASTValue.h"
#include "AST/ASTVar.h"
#include "Basic/Debug.h"
#include "Basic/Diagnostic.h"
#include "Basic/SourceLocation.h"

#include <AST/ASTAttribute.h>
#include <AST/ASTDeclStmt.h>
#include <AST/ASTEnumValue.h>
#include <AST/ASTExprStmt.h>
#include <AST/ASTFailStmt.h>
#include <AST/ASTLocalVar.h>
#include <AST/ASTMember.h>
#include <AST/ASTMethod.h>
#include <AST/ASTName.h>
#include <AST/ASTParam.h>
#include <AST/ASTReturnStmt.h>
#include <Frontend/InputFile.h>
#include <utility>

using namespace fly;

/**
 * Private constructor used only from Sema constructor
 * @param S
 */
ASTBuilder::ASTBuilder(DiagnosticsEngine &Diags) : Diags(Diags) {

}

/**
 * Creates an ASTModule
 * If NameSpace doesn't exists it will be created
 * @param Name
 * @return the ASTModule
 */
ASTModule *ASTBuilder::CreateModule(InputFile *F) {
	FLY_DEBUG_START_MSG("ASTBuilder", "GenerateModule", "Name=" << F->getName());

	ASTModule *Module = new ASTModule(F);

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
ASTModule *ASTBuilder::CreateHeader(InputFile *F) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateHeader", "Name=" << F->getName());

	ASTModule *Module = new ASTModule(F);

	FLY_DEBUG_END("ASTBuilder", "CreateHeaderModule");
	return Module;
}

ASTComment *ASTBuilder::CreateComment(ASTModule *Module, const SourceLocation &Loc, llvm::StringRef Content) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateComment", "Loc" << Loc.getRawEncoding() << " Content=" << Content);
	ASTComment *Comment = new ASTComment(Loc, Content);

	// Add Comment to Module
	Module->addNode(Comment);

	FLY_DEBUG_END("ASTBuilder", "CreateHeaderModule");
	return Comment;
}

ASTName *ASTBuilder::CreateName(llvm::StringRef Name, const SourceLocation &Loc) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateName", "Loc" << Loc.getRawEncoding() << " Name=" << Name);
	ASTName *AST = new ASTName(Loc, Name);

	FLY_DEBUG_END("ASTBuilder", "CreateName");
	return AST;
}

ASTNameSpace *ASTBuilder::CreateNameSpace(
	ASTModule *Module, const SourceLocation &Loc, llvm::SmallVector<ASTName *, 4> Names) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateNameSpace", "Loc=" << Loc.getRawEncoding());
	Module->NameSpace = new ASTNameSpace(Loc, Names);
		FLY_DEBUG_END("ASTBuilder", "CreateNameSpace");
	return Module->NameSpace;
}

ASTImport *ASTBuilder::CreateImport(
	ASTModule *Module, const SourceLocation &Loc, llvm::SmallVector<ASTName *, 4> Names,
	llvm::SmallVector<ASTName *, 4> Alias) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateImport", "Loc=" << Loc.getRawEncoding());

	ASTImport *Import = new ASTImport(Loc, Names, Alias);

	// Add Import to Module
	Module->addNode(Import);

	FLY_DEBUG_END("ASTBuilder", "CreateImport");
	return Import;
}

/**
 * Creates an ASTFunction
 * @param Module
 * @param Loc
 * @param Type
 * @param Name
 * @param Modifiers
 * @return
 */
ASTFunction *ASTBuilder::CreateFunction(
	ASTModule *Module, const SourceLocation &Loc, ASTType *TypeRef,
	llvm::StringRef Name, llvm::SmallVector<ASTModifier *, 8> &Modifiers,
	SmallVector<ASTParam *, 8> &Params, ASTBlockStmt *Body) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateFunction", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTFunction *Function = new ASTFunction(Loc, TypeRef, Modifiers, Name, Params);

	// Create Body
	if (Body)
		CreateBody(Function, Body);

	// Add Function to Module
	Module->addNode(Function);

	FLY_DEBUG_END("ASTBuilder", "CreateFunction");
	return Function;
}

/**
 * Creates an ASTClass
 * @param Module
 * @param Loc
 * @param Name
 * @param Modifiers
 * @return
 */
ASTClass *ASTBuilder::CreateClass(
	ASTModule *Module, const SourceLocation &Loc, ASTClassKind ClassKind,
	const llvm::StringRef Name, llvm::SmallVector<ASTModifier *, 8> &Modifiers,
	llvm::SmallVector<ASTType *, 4> &Bases) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateClass", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTClass *Class = new ASTClass(ClassKind, Modifiers, Loc, Name, Bases);

	// Add Class to Module
	Module->addNode(Class);

	FLY_DEBUG_END("ASTBuilder", "CreateClass");
	return Class;
}

/**
 * Creates a ASTClassVar
 * @param Class
 * @param Loc
 * @param Type
 * @param Name
 * @param Modifiers
 * @return
 */
ASTAttribute *ASTBuilder::CreateClassAttribute(
	const SourceLocation &Loc, ASTClass *Class, ASTType *TypeRef,
	llvm::StringRef Name, SmallVector<ASTModifier *, 8> &Modifiers,
	ASTExpr *Expr) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateClassAttribute", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTAttribute *Attribute = new ASTAttribute(Loc, TypeRef, Name, Modifiers);
	Attribute->Expr = Expr;
	Class->Nodes.push_back(Attribute);

	FLY_DEBUG_END("ASTBuilder", "CreateClassAttribute");
	return Attribute;
}

ASTMethod *ASTBuilder::CreateDefaultConstructor(ASTClass *Class) {
	SourceLocation Loc = SourceLocation();
	ASTType *Void = CreateVoidType(Loc);

	// Create Modifiers
	llvm::SmallVector<ASTModifier *, 8> Modifiers;
	Modifiers.push_back(CreateModifier(Loc, ASTModifierKind::MOD_PUBLIC));

	// Empty Params
	llvm::SmallVector<ASTParam *, 8> Params;

	// Create the Default Constructor
	ASTMethod *Method = new ASTMethod(Loc, Void, Modifiers, Class->getName(), Params);

	// Add empty body
	ASTBlockStmt *Body = CreateBlockStmt(Loc);
	CreateBody(Method, Body);

	// Add to Class Methods
	Class->Nodes.push_back(Method);

	return Method;
}

ASTMethod *ASTBuilder::CreateClassMethod(
	const SourceLocation &Loc, ASTClass *Class, ASTType *ReturnType,
	llvm::StringRef Name, llvm::SmallVector<ASTModifier *, 8> &Modifiers,
	llvm::SmallVector<ASTParam *, 8> &Params, ASTBlockStmt *Body) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateClassMethod", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTMethod *Method = new ASTMethod(Loc, ReturnType, Modifiers, Name, Params);

	if (Body)
		CreateBody(Method, Body);

	// Add to Class Methods
	Class->Nodes.push_back(Method);

	FLY_DEBUG_END("ASTBuilder", "CreateClassMethod");
	return Method;
}

ASTEnum *ASTBuilder::CreateEnum(
	ASTModule *Module, const SourceLocation &Loc, const llvm::StringRef Name,
	llvm::SmallVector<ASTModifier *, 8> &Modifiers,
	llvm::SmallVector<ASTType *, 4> EnumTypes) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateEnum", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTEnum *Enum = new ASTEnum(Loc, Name, Modifiers, EnumTypes);

	// Add Enum to Module
	Module->addNode(Enum);

	FLY_DEBUG_END("ASTBuilder", "CreateEnum");
	return Enum;
}

ASTEnumValue *ASTBuilder::CreateEnumValue(
	const SourceLocation &Loc, ASTEnum *Enum, llvm::StringRef Name,
	llvm::SmallVector<ASTModifier *, 8> &Modifiers) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateEnumEntry", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTEnumValue *EnumValue = new ASTEnumValue(Loc, Enum, Name);
	Enum->Nodes.push_back(EnumValue);

	FLY_DEBUG_END("ASTBuilder", "CreateEnumEntry");
	return EnumValue;
}

/**
 * Creates a Modifier
 * @param Loc
 * @param Kind
 * @return
 */
ASTModifier *ASTBuilder::CreateModifier(const SourceLocation &Loc, ASTModifierKind Kind) {
	FLY_DEBUG_START_MSG(
		"ASTBuilder", "CreateModifier", "Loc=" << Loc.getRawEncoding() << ", Kind=" << static_cast<size_t>(Kind));

	ASTModifier *Modifier = new ASTModifier(Loc, Kind);

	FLY_DEBUG_END("ASTBuilder", "CreateModifier");
	return Modifier;
}

/**
 * Creates a bool type
 * @param Loc
 * @return
 */
ASTType *ASTBuilder::CreateBoolType(const SourceLocation &Loc) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateBoolTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_BOOL);

	FLY_DEBUG_END("ASTBuilder", "CreateBoolTypeRef");
	return TypeRef;
}

/**
 * Creates an byte type
 * @param Loc
 * @return
 */
ASTType *ASTBuilder::CreateByteType(const SourceLocation &Loc) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateByteTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_BYTE);

	FLY_DEBUG_END("ASTBuilder", "CreateByteTypeRef");
	return TypeRef;
}

/**
 * Creates an unsigned short type
 * @param Loc
 * @return
 */
ASTType *ASTBuilder::CreateUShortType(const SourceLocation &Loc) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateUShortTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_USHORT);

	FLY_DEBUG_END("ASTBuilder", "CreateUShortTypeRef");
	return TypeRef;
}

/**
 * Create a short type
 * @param Loc
 * @return
 */
ASTType *ASTBuilder::CreateShortType(const SourceLocation &Loc) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateShortTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_SHORT);

	FLY_DEBUG_END("ASTBuilder", "CreateShortTypeRef");
	return TypeRef;
}

/**
 * Creates an unsigned int type
 * @param Loc
 * @return
 */
ASTType *ASTBuilder::CreateUIntType(const SourceLocation &Loc) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateUIntTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_UINT);

	FLY_DEBUG_END("ASTBuilder", "CreateUIntTypeRef");
	return TypeRef;
}

/**
 * Creates an int type
 * @param Loc
 * @return
 */
ASTType *ASTBuilder::CreateIntType(const SourceLocation &Loc) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateIntTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_INT);

	FLY_DEBUG_END("ASTBuilder", "CreateIntTypeRef");
	return TypeRef;
}

/**
 * Creates an unsigned long type
 * @param Loc
 * @return
 */
ASTType *ASTBuilder::CreateULongType(const SourceLocation &Loc) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateULongTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_ULONG);

	FLY_DEBUG_END("ASTBuilder", "CreateULongTypeRef");
	return TypeRef;
}

/**
 * Creates a long type
 * @param Loc
 * @return
 */
ASTType *ASTBuilder::CreateLongType(const SourceLocation &Loc) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateLongTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_LONG);

	FLY_DEBUG_END("ASTBuilder", "CreateLongTypeRef");
	return TypeRef;
}

/**
 * Creates a float type
 * @param Loc
 * @return
 */
ASTType *ASTBuilder::CreateFloatType(const SourceLocation &Loc) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateFloatTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_FLOAT);

	FLY_DEBUG_END("ASTBuilder", "CreateFloatTypeRef");
	return TypeRef;
}

/**
 * Creates a double type
 * @param Loc
 * @return
 */
ASTType *ASTBuilder::CreateDoubleType(const SourceLocation &Loc) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateDoubleTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_DOUBLE);

	FLY_DEBUG_END("ASTBuilder", "CreateDoubleTypeRef");
	return TypeRef;
}

/**
 * Creates a void type
 * @param Loc
 * @return
 */
ASTType *ASTBuilder::CreateVoidType(const SourceLocation &Loc) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateVoidTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_VOID);

	FLY_DEBUG_END("ASTBuilder", "CreateVoidTypeRef");
	return TypeRef;
}

ASTType *ASTBuilder::CreateStringType(const SourceLocation &Loc) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateStringTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_STRING);

	FLY_DEBUG_END("ASTBuilder", "CreateStringTypeRef");
	return TypeRef;
}

ASTType *ASTBuilder::CreateErrorType(const SourceLocation &Loc) {
	FLY_DEBUG_START("ASTBuilder", "CreateErrorTypeRef");

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_ERROR);

	FLY_DEBUG_END("ASTBuilder", "CreateErrorTypeRef");
	return TypeRef;
}

/**
 * Creates an array type
 * @param Loc
 * @param Type
 * @param Size
 * @return
 */
ASTArrayType *ASTBuilder::CreateArrayType(const SourceLocation &Loc, ASTType *ElementType, ASTExpr *SizeExpr) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateArrayTypeRef", "Loc=" << Loc.getRawEncoding());

	// TODO Size
	ASTArrayType *ArrayTypeRef = new ASTArrayType(Loc, ElementType, SizeExpr);

	FLY_DEBUG_END("ASTBuilder", "CreateArrayTypeRef");
	return ArrayTypeRef;
}

/**
 * Creates a Type
 * @param Loc
 * @param Name
 * @return
 */
ASTType *ASTBuilder::CreateType(const SourceLocation &Loc, llvm::SmallVector<ASTName *, 4> Names) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateType", "Loc=" << Loc.getRawEncoding());

	ASTType *T = new ASTNamedType(Loc, Names);

	FLY_DEBUG_END("ASTBuilder", "CreateType");
	return T;
}

/**
 * Create a Default Value
 * @return the default value
 */
ASTDefaultValue *ASTBuilder::CreateDefaultValue() {
	FLY_DEBUG_END("ASTBuilder", "CreateDefaultValue");

	ASTDefaultValue *Value = new ASTDefaultValue();

	FLY_DEBUG_END("ASTBuilder", "CreateDefaultValue");
	return Value;
}

/**
 * Creates a null value
 * @param Loc
 * @return
 */
ASTNullValue *ASTBuilder::CreateNullValue(const SourceLocation &Loc) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateNullValue", "Loc=" << Loc.getRawEncoding());

	ASTNullValue *Value = new ASTNullValue(Loc);

	FLY_DEBUG_END("ASTBuilder", "CreateNullValue");
	return Value;
}

/**
 * Creates a bool value
 * @param Loc
 * @param Val
 * @return
 */
ASTBoolValue *ASTBuilder::CreateBoolValue(const SourceLocation &Loc, bool Val) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateBoolValue", "Loc=" << Loc.getRawEncoding() << ", Val=" << Val);

	ASTBoolValue *Value = new ASTBoolValue(Loc, Val);

	FLY_DEBUG_END("ASTBuilder", "CreateBoolValue");
	return Value;
}

/**
 * Creates an integer value
 * @param Loc
 * @param Val
 * @param Radix
 * @return
 */
ASTNumberValue *ASTBuilder::CreateNumberValue(const SourceLocation &Loc, llvm::StringRef Val) {
	FLY_DEBUG_START_MSG(
		"ASTBuilder", "CreateIntegerValue",
		"Loc=" << Loc.getRawEncoding() << ", Val=" << Val);

	ASTNumberValue *Value = new ASTNumberValue(Loc, Val);

	FLY_DEBUG_END("ASTBuilder", "CreateIntegerValue");
	return Value;
}

ASTStringValue *ASTBuilder::CreateStringValue(const SourceLocation &Loc, llvm::StringRef Val) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateStringValue", "Loc=" << Loc.getRawEncoding() << ", Val=" << Val);

	ASTStringValue *Value = new ASTStringValue(Loc, Val);

	FLY_DEBUG_END("ASTBuilder", "CreateStringValue");
	return Value;
}

/**
 * Create an array value
 * @param Loc
 * @return
 */
ASTArrayValue *ASTBuilder::CreateArrayValue(const SourceLocation &Loc, llvm::SmallVector<ASTValue *, 8> Values) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateArrayValue", "Loc=" << Loc.getRawEncoding());

	ASTArrayValue *Array = new ASTArrayValue(Loc);
	Array->Values = std::move(Values);

	FLY_DEBUG_END("ASTBuilder", "CreateArrayValue");
	return Array;
}

ASTStructValue *ASTBuilder::CreateStructValue(const SourceLocation &Loc, llvm::StringMap<ASTValue *> Values) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateArrayValue", "Loc=" << Loc.getRawEncoding());

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
ASTParam *ASTBuilder::CreateParam(
	const SourceLocation &Loc, ASTType *TypeRef, llvm::StringRef Name,
	llvm::SmallVector<ASTModifier *, 8> &Modifiers, ASTValue *DefaultValue) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateParam", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTParam *Param = new ASTParam(Loc, TypeRef, Name, Modifiers);
	Param->setExpr(DefaultValue);

	FLY_DEBUG_END("ASTBuilder", "CreateParam");
	return Param;
}

/**
 * Creates an ASTVar
 * @param Loc
 * @param Type
 * @param Name
 * @param Constant
 * @return
 */
ASTLocalVar *ASTBuilder::CreateLocalVar(
	const SourceLocation &Loc, ASTType *Type,
	llvm::StringRef Name, llvm::SmallVector<ASTModifier *, 8> &Modifiers) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateLocalVar", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTLocalVar *Var = new ASTLocalVar(Loc, Type, Name, Modifiers);

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
ASTCall *ASTBuilder::CreateCall(
	const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTExpr *, 8> &Args,
	ASTCallKind CallKind, ASTExpr *Parent) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateCall", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTCall *Call = new ASTCall(Loc, Name, CallKind);
	if (Parent) {
		// Take Parent
		Call->setParent(Parent);
	}
	uint64_t i = 0;
	for (auto &Expr : Args) {
		ASTArg *Arg = new ASTArg(Expr, i++);
		Call->Args.push_back(Arg);
	}

	FLY_DEBUG_END("ASTBuilder", "CreateCall");
	return Call;
}

ASTIdentifier *ASTBuilder::CreateIdentifier(ASTVar *Var) {
	FLY_DEBUG_START("ASTBuilder", "CreateVarRef");

	ASTIdentifier *Identifier = new ASTIdentifier(Var->getLocation(), Var->getName());
	Identifier->Var = Var;

	FLY_DEBUG_END("ASTBuilder", "CreateVarRef");
	return Identifier;
}

ASTIdentifier *ASTBuilder::CreateIdentifier(const SourceLocation &Loc, llvm::StringRef Name, ASTExpr *Parent) {
	FLY_DEBUG_START("ASTBuilder", "CreateIdentifier");

	ASTIdentifier *Identifier = new ASTIdentifier(Loc, Name);
	Identifier->Parent = Parent;

	FLY_DEBUG_END("ASTBuilder", "CreateIdentifier");
	return Identifier;
}

ASTMember *ASTBuilder::CreateMember(const SourceLocation &Loc, llvm::StringRef Name, ASTExpr *Parent) {
	FLY_DEBUG_START("ASTBuilder", "CreateMember");

	ASTMember *Member = new ASTMember(Loc, Name, Parent);

	FLY_DEBUG_END("ASTBuilder", "CreateMember");
	return Member;
}

ASTUnary *ASTBuilder::CreateUnary(const SourceLocation &Loc, ASTUnaryKind OpKind, ASTExpr *Expr) {
	FLY_DEBUG_START_MSG(
		"ASTBuilder", "CreateUnaryOpExpr",
		"Loc=" << Loc.getRawEncoding() << ", OpKind=" << static_cast<uint8_t>(OpKind));

	ASTUnary *UnaryOpExpr = new ASTUnary(Loc, OpKind, Expr);

	FLY_DEBUG_END("ASTBuilder", "CreateUnaryOpExpr");
	return UnaryOpExpr;
}

ASTBinary *ASTBuilder::CreateBinary(
	const SourceLocation &OpLocation, ASTBinaryKind OpKind,
	ASTExpr *LeftExpr, ASTExpr *RightExpr) {
	FLY_DEBUG_START_MSG(
		"ASTBuilder", "CreateBinaryOpExpr",
		"OpLocation=" << OpLocation.getRawEncoding() << ", OpKind=" << static_cast<uint8_t>(OpKind));

	ASTBinary *BinaryOpExpr = new ASTBinary(OpKind, OpLocation, LeftExpr, RightExpr);

	FLY_DEBUG_END("ASTBuilder", "CreateBinaryOpExpr");
	return BinaryOpExpr;
}

ASTTernary *ASTBuilder::CreateTernary(
	ASTExpr *ConditionExpr,
	const SourceLocation &TrueOpLocation, ASTExpr *TrueExpr,
	const SourceLocation &FalseOpLocation, ASTExpr *FalseExpr) {
	FLY_DEBUG_START_MSG(
		"ASTBuilder", "CreateBinaryOpExpr",
		"TrueOpLocation=" << TrueOpLocation.getRawEncoding() << "FalseOpLocation=" << FalseOpLocation.getRawEncoding());

	ASTTernary *TernaryExpr = new ASTTernary(
		ConditionExpr,
		TrueOpLocation, TrueExpr,
		FalseOpLocation, FalseExpr);

	FLY_DEBUG_END("ASTBuilder", "CreateTernaryOpExpr");
	return TernaryExpr;
}

/**
 * Creates a ASTDeclStmt
 * @param Parent
 * @param Loc
 * @param Var
 * @return
 */
ASTDeclStmt *ASTBuilder::CreateDeclStmt(ASTBlockStmt *Parent, const SourceLocation &Loc, ASTLocalVar *Var) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateDeclStmt", "Loc=" << Loc.getRawEncoding());
	ASTDeclStmt *Stmt = new ASTDeclStmt(Loc, Var);
	Parent->addContent(Stmt);
	FLY_DEBUG_END("ASTBuilder", "CreateDeclStmt");
	return Stmt;
}

/**
 * Creates a ASTReturnStmt
 * @param Parent
 * @param Loc
 * @return
 */
ASTReturnStmt *ASTBuilder::CreateReturnStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateReturnStmt", "Loc=" << Loc.getRawEncoding());

	ASTReturnStmt *Stmt = new ASTReturnStmt(Loc);
	Parent->addContent(Stmt);

	FLY_DEBUG_END("ASTBuilder", "CreateReturnStmt");
	return Stmt;
}

/**
 * Creates an ASTExprStmt
 * @param Loc
 * @return
 */
ASTExprStmt *ASTBuilder::CreateExprStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateExprStmt", Logger()
                      .Attr("Parent", Parent)
                      .Attr("Loc", (uint64_t) Loc.getRawEncoding()).End());
     ASTExprStmt *Stmt = new ASTExprStmt(Loc);
	Parent->addContent(Stmt);
     return Stmt;
 }

/**
 * Creates an ASTFailStmt
 * @param Loc
 * @param ErrorHandler
 * @return
 */
ASTFailStmt *ASTBuilder::CreateFailStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateFailStmt", "Loc=" << Loc.getRawEncoding());

	ASTFailStmt *Stmt = new ASTFailStmt(Loc);
	Parent->addContent(Stmt);

	FLY_DEBUG_END("ASTBuilder", "CreateFailStmt");
	return Stmt;
}

ASTHandleStmt *ASTBuilder::CreateHandleStmt(
	ASTBlockStmt *Parent, const SourceLocation &Loc,
	ASTBlockStmt *BlockStmt, ASTExpr *ErrorHandler) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateHandleStmt", "Loc=" << Loc.getRawEncoding());

	ASTHandleStmt *HandleStmt = new ASTHandleStmt(Loc);
	Parent->addContent(HandleStmt);
	HandleStmt->ErrorHandler = ErrorHandler;
	HandleStmt->Handle = BlockStmt;

	// set Handle Block
	BlockStmt->Parent = HandleStmt;
	BlockStmt->Function = HandleStmt->Function;

	FLY_DEBUG_END("ASTBuilder", "CreateHandleStmt");
	return HandleStmt;
}

ASTBreakStmt *ASTBuilder::CreateBreakStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateLocalVar", "Loc=" << Loc.getRawEncoding());

	ASTBreakStmt *Break = new ASTBreakStmt(Loc);
	// Inner Stmt
	Parent->Content.push_back(Break);
	Break->Parent = Parent;
	Break->Function = Parent->Function;

	FLY_DEBUG_END("ASTBuilder", "CreateBreakStmt");
	return Break;
}

ASTContinueStmt *ASTBuilder::CreateContinueStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateContinueStmt", "Loc=" << Loc.getRawEncoding());

	ASTContinueStmt *Continue = new ASTContinueStmt(Loc);
	// Inner Stmt
	Parent->Content.push_back(Continue);
	Continue->Parent = Parent;
	Continue->Function = Parent->Function;

	FLY_DEBUG_END("ASTBuilder", "CreateContinueStmt");
	return Continue;
}

ASTDeleteStmt *ASTBuilder::CreateDeleteStmt(ASTBlockStmt *Parent, const SourceLocation &Loc, ASTIdentifier *VarRef) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateDeleteStmt", "Loc=" << Loc.getRawEncoding());

	ASTDeleteStmt *Delete = new ASTDeleteStmt(Loc, VarRef);
	// Inner Stmt
	Parent->Content.push_back(Delete);
	Delete->Parent = Parent;
	Delete->Function = Parent->Function;

	FLY_DEBUG_END("ASTBuilder", "CreateDeleteStmt");
	return Delete;
}

ASTBlockStmt *
ASTBuilder::CreateBody(ASTFunction *Function, ASTBlockStmt *Body) {
	FLY_DEBUG_START("ASTBuilder", "CreateBody");

	Body->Parent = nullptr; // body cannot have a parent stmt
	Function->Body = Body;
	Function->Body->Function = Function;

	FLY_DEBUG_END("ASTBuilder", "CreateBody");
	return Function->Body;
}

ASTBlockStmt *ASTBuilder::CreateBlockStmt(const SourceLocation &Loc) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateBlockStmt", "Loc=" << Loc.getRawEncoding());

	ASTBlockStmt *Block = new ASTBlockStmt(Loc);

	FLY_DEBUG_END("ASTBuilder", "CreateBlockStmt");
	return Block;
}

ASTBlockStmt *ASTBuilder::CreateBlockStmt(ASTStmt *Parent, const SourceLocation &Loc) {
	FLY_DEBUG_START_MSG("ASTBuilder", "CreateBlockStmt", "Loc=" << Loc.getRawEncoding());

	ASTBlockStmt *Block = new ASTBlockStmt(Loc);
	Block->Parent = Parent;

	FLY_DEBUG_END("ASTBuilder", "CreateBlockStmt");
	return Block;
}

