//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTBuilder.cpp - The AST Builder
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTBuilder.h"
#include "CodeGen/CodeGen.h"
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
#include "AST/ASTIdentifier.h"
#include "AST/ASTValue.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
#include "AST/ASTExpr.h"
#include "AST/ASTOpExpr.h"
#include "AST/ASTType.h"
#include "Basic/SourceLocation.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"

#include <utility>
#include <AST/ASTAttribute.h>
#include <AST/ASTEnumEntry.h>
#include <AST/ASTExprStmt.h>
#include <AST/ASTFailStmt.h>
#include <AST/ASTLocalVar.h>
#include <AST/ASTMember.h>
#include <AST/ASTMethod.h>
#include <AST/ASTName.h>
#include <AST/ASTParam.h>
#include <AST/ASTReturnStmt.h>
#include <Frontend/InputFile.h>

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
	FLY_DEBUG_MESSAGE("ASTBuilder", "GenerateModule", "Name=" << F->getName());

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
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateHeader", "Name=" << F->getName());

	ASTModule *Module = new ASTModule(F);

	FLY_DEBUG_END("ASTBuilder", "CreateHeaderModule");
	return Module;
}

ASTComment *ASTBuilder::CreateComment(ASTModule *Module, const SourceLocation &Loc, llvm::StringRef Content) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateComment", "Loc" << Loc.getRawEncoding() << " Content=" << Content);
	ASTComment *Comment = new ASTComment(Loc, Content);

	// Add Comment to Module
	Module->Nodes.push_back(Comment);

	FLY_DEBUG_END("ASTBuilder", "CreateHeaderModule");
	return Comment;
}

ASTName *ASTBuilder::CreateName(llvm::StringRef Name, const SourceLocation &Loc) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateName", "Loc" << Loc.getRawEncoding() << " Name=" << Name);
	ASTName *AST = new ASTName(Loc, Name);

	FLY_DEBUG_END("ASTBuilder", "CreateName");
	return AST;
}

ASTNameSpace *ASTBuilder::CreateNameSpace(
	ASTModule *Module, const SourceLocation &Loc, llvm::SmallVector<ASTName *, 4> Names) {
	if (Names.empty()) {
		Diags.Report(Loc, diag::err_sema_namespace_empty);
	}

	Module->NameSpace = new ASTNameSpace(Loc, Names);

	return Module->NameSpace;
}

ASTImport *ASTBuilder::CreateImport(
	ASTModule *Module, const SourceLocation &Loc, llvm::SmallVector<ASTName *, 4> Names,
	llvm::SmallVector<ASTName *, 4> Alias) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateImport", "Loc=" << Loc.getRawEncoding());

	ASTImport *Import = new ASTImport(Loc, Names, Alias);

	// Add Import to Module
	Module->Nodes.push_back(Import);

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
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateFunction", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTFunction *Function = new ASTFunction(Loc, TypeRef, Modifiers, Name, Params);

	// Create Body
	if (Body)
		CreateBody(Function, Body);

	// Add Function to Module
	Module->Nodes.push_back(Function);

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
	llvm::SmallVector<ASTType *, 4> &SuperClasses) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateClass", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTClass *Class = new ASTClass(ClassKind, Modifiers, Loc, Name, SuperClasses);

	// Add Class to Module
	Module->Nodes.push_back(Class);

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
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateClassAttribute", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

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
	Modifiers.push_back(CreateModifier(Loc, ASTModifierKind::MOD_DEFAULT));

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
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateClassMethod", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

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
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateEnum", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTEnum *Enum = new ASTEnum(Loc, Name, Modifiers, EnumTypes);

	// Add Enum to Module
	Module->Nodes.push_back(Enum);

	FLY_DEBUG_END("ASTBuilder", "CreateEnum");
	return Enum;
}

ASTEnumEntry *ASTBuilder::CreateEnumEntry(
	const SourceLocation &Loc, ASTEnum *Enum, llvm::StringRef Name,
	llvm::SmallVector<ASTModifier *, 8> &Modifiers) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateEnumEntry", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTType *TypeRef = CreateIntType(Loc);
	ASTEnumEntry *EnumEntry = new ASTEnumEntry(Loc, TypeRef, Name, Modifiers);
	Enum->Nodes.push_back(EnumEntry);

	FLY_DEBUG_END("ASTBuilder", "CreateEnumEntry");
	return EnumEntry;
}

/**
 * Creates a Modifier
 * @param Loc
 * @param Kind
 * @return
 */
ASTModifier *ASTBuilder::CreateModifier(const SourceLocation &Loc, ASTModifierKind Kind) {
	FLY_DEBUG_MESSAGE(
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
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateBoolTypeRef", "Loc=" << Loc.getRawEncoding());

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
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateByteTypeRef", "Loc=" << Loc.getRawEncoding());

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
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateUShortTypeRef", "Loc=" << Loc.getRawEncoding());

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
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateShortTypeRef", "Loc=" << Loc.getRawEncoding());

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
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateUIntTypeRef", "Loc=" << Loc.getRawEncoding());

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
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateIntTypeRef", "Loc=" << Loc.getRawEncoding());

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
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateULongTypeRef", "Loc=" << Loc.getRawEncoding());

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
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateLongTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_ULONG);

	FLY_DEBUG_END("ASTBuilder", "CreateLongTypeRef");
	return TypeRef;
}

/**
 * Creates a float type
 * @param Loc
 * @return
 */
ASTType *ASTBuilder::CreateFloatType(const SourceLocation &Loc) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateFloatTypeRef", "Loc=" << Loc.getRawEncoding());

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
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateDoubleTypeRef", "Loc=" << Loc.getRawEncoding());

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
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateVoidTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_VOID);

	FLY_DEBUG_END("ASTBuilder", "CreateVoidTypeRef");
	return TypeRef;
}

ASTType *ASTBuilder::CreateStringType(const SourceLocation &Loc) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateStringTypeRef", "Loc=" << Loc.getRawEncoding());

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
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateArrayTypeRef", "Loc=" << Loc.getRawEncoding());

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
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateType", "Loc=" << Loc.getRawEncoding());

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
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateNullValue", "Loc=" << Loc.getRawEncoding());

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
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateBoolValue", "Loc=" << Loc.getRawEncoding() << ", Val=" << Val);

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
	FLY_DEBUG_MESSAGE(
		"ASTBuilder", "CreateIntegerValue",
		"Loc=" << Loc.getRawEncoding() << ", Val=" << Val);

	ASTNumberValue *Value = new ASTNumberValue(Loc, Val);

	FLY_DEBUG_END("ASTBuilder", "CreateIntegerValue");
	return Value;
}

ASTStringValue *ASTBuilder::CreateStringValue(const SourceLocation &Loc, llvm::StringRef Val) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateStringValue", "Loc=" << Loc.getRawEncoding() << ", Val=" << Val);

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
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateArrayValue", "Loc=" << Loc.getRawEncoding());

	ASTArrayValue *Array = new ASTArrayValue(Loc);
	Array->Values = std::move(Values);

	FLY_DEBUG_END("ASTBuilder", "CreateArrayValue");
	return Array;
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
ASTParam *ASTBuilder::CreateParam(
	const SourceLocation &Loc, ASTType *TypeRef, llvm::StringRef Name,
	llvm::SmallVector<ASTModifier *, 8> &Modifiers, ASTValue *DefaultValue) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateParam", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTParam *Param = new ASTParam(Loc, TypeRef, Name, Modifiers);
	Param->setExpr(DefaultValue);

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
ASTLocalVar *ASTBuilder::CreateLocalVar(
	ASTBlockStmt *BlockStmt, const SourceLocation &Loc, ASTType *Type,
	llvm::StringRef Name, llvm::SmallVector<ASTModifier *, 8> &Modifiers) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateLocalVar", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTLocalVar *Var = new ASTLocalVar(Loc, Type, Name, Modifiers);
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
ASTCall *ASTBuilder::CreateCall(
	const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTExpr *, 8> &Args,
	ASTCallKind CallKind, ASTExpr *Parent) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateCall", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTCall *Call = new ASTCall(Loc, Name, CallKind);
	if (Parent) {
		// Take Parent
		Parent->setChild(Call);
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

/**
 * Creates an ASTFunctionCall with definition
 * @param Stmt
 * @param Function
 * @return
 */
ASTCall *ASTBuilder::CreateCall(llvm::StringRef Name, llvm::SmallVector<ASTExpr *, 8> &Args) {
	FLY_DEBUG_START("ASTBuilder", "CreateCall");

	const SourceLocation &Loc = SourceLocation();
	ASTCall *Call = CreateCall(Loc, Name, Args, ASTCallKind::CALL_DIRECT);

	FLY_DEBUG_END("ASTBuilder", "CreateCall");
	return Call;
}

ASTCall *ASTBuilder::CreateCall(ASTIdentifier *Instance, llvm::StringRef Name, llvm::SmallVector<ASTExpr *, 8> &Args) {
	FLY_DEBUG_START("ASTBuilder", "CreateCall");

	ASTCall *Call = CreateCall(Instance->getLocation(), Name, Args, ASTCallKind::CALL_DIRECT);
	Call->setParent(Instance);

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

ASTMember *ASTBuilder::CreateMember(ASTVar *Var, ASTExpr *Parent) {
	FLY_DEBUG_START("ASTBuilder", "CreateVarRef");

	ASTMember *Member = new ASTMember(Var->getLocation(), Var->getName(), Parent);
	Member->Var = Var;

	FLY_DEBUG_END("ASTBuilder", "CreateVarRef");
	return Member;
}

ASTMember *ASTBuilder::CreateMember(const SourceLocation &Loc, llvm::StringRef Name, ASTExpr *Parent) {
	FLY_DEBUG_START("ASTBuilder", "CreateMember");

	ASTMember *Member = new ASTMember(Loc, Name, Parent);

	FLY_DEBUG_END("ASTBuilder", "CreateMember");
	return Member;
}

ASTUnaryOpExpr *ASTBuilder::CreateUnary(const SourceLocation &Loc, ASTUnaryOpExprKind OpKind, ASTExpr *Expr) {
	FLY_DEBUG_MESSAGE(
		"ASTBuilder", "CreateUnaryOpExpr",
		"Loc=" << Loc.getRawEncoding() << ", OpKind=" << static_cast<uint8_t>(OpKind));

	ASTUnaryOpExpr *UnaryOpExpr = new ASTUnaryOpExpr(Loc, OpKind, Expr);

	FLY_DEBUG_END("ASTBuilder", "CreateUnaryOpExpr");
	return UnaryOpExpr;
}

ASTBinaryOpExpr *ASTBuilder::CreateBinary(
	const SourceLocation &OpLocation, ASTBinaryOpExprKind OpKind,
	ASTExpr *LeftExpr, ASTExpr *RightExpr) {
	FLY_DEBUG_MESSAGE(
		"ASTBuilder", "CreateBinaryOpExpr",
		"OpLocation=" << OpLocation.getRawEncoding() << ", OpKind=" << static_cast<uint8_t>(OpKind));

	ASTBinaryOpExpr *BinaryOpExpr = new ASTBinaryOpExpr(OpKind, OpLocation, LeftExpr, RightExpr);

	FLY_DEBUG_END("ASTBuilder", "CreateBinaryOpExpr");
	return BinaryOpExpr;
}

ASTTernaryOpExpr *ASTBuilder::CreateTernary(
	ASTExpr *ConditionExpr,
	const SourceLocation &TrueOpLocation, ASTExpr *TrueExpr,
	const SourceLocation &FalseOpLocation, ASTExpr *FalseExpr) {
	FLY_DEBUG_MESSAGE(
		"ASTBuilder", "CreateBinaryOpExpr",
		"TrueOpLocation=" << TrueOpLocation.getRawEncoding() << "FalseOpLocation=" << FalseOpLocation.getRawEncoding());

	ASTTernaryOpExpr *TernaryExpr = new ASTTernaryOpExpr(
		ConditionExpr,
		TrueOpLocation, TrueExpr,
		FalseOpLocation, FalseExpr);

	FLY_DEBUG_END("ASTBuilder", "CreateTernaryOpExpr");
	return TernaryExpr;
}


/**
 * Creates a ASTVarStmt
 * @param Parent
 * @param VarRef
 * @return
 */
ASTVarStmt *ASTBuilder::CreateAssignmentStmt(ASTBlockStmt *Parent, ASTIdentifier *VarRef, ASTAssignOperatorKind Kind) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateAssignmentStmt", "Kind=" << static_cast<uint8_t>(Kind));

	ASTVarStmt *Stmt = new ASTVarStmt(VarRef->getLocation(), VarRef, Kind);
	Stmt->Parent = Parent;

	FLY_DEBUG_END("ASTBuilder", "CreateAssignmentStmt");
	return Stmt;
}

/**
 * Creates a ASTReturnStmt
 * @param Parent
 * @param Loc
 * @return
 */
ASTReturnStmt *ASTBuilder::CreateReturnStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateReturnStmt", "Loc=" << Loc.getRawEncoding());

	ASTReturnStmt *Stmt = new ASTReturnStmt(Loc);
	Stmt->Parent = Parent;

	FLY_DEBUG_END("ASTBuilder", "CreateReturnStmt");
	return Stmt;
}

/**
 * Creates an ASTExprStmt
 * @param Loc
 * @return
 */
ASTExprStmt *ASTBuilder::CreateExprStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateExprStmt", "Loc=" << Loc.getRawEncoding());

	ASTExprStmt *Stmt = new ASTExprStmt(Loc);

	FLY_DEBUG_END("ASTBuilder", "CreateExprStmt");
	return Stmt;
}

/**
 * Creates an ASTFailStmt
 * @param Loc
 * @param ErrorHandler
 * @return
 */
ASTFailStmt *ASTBuilder::CreateFailStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateFailStmt", "Loc=" << Loc.getRawEncoding());

	ASTFailStmt *Stmt = new ASTFailStmt(Loc);
	Stmt->Parent = Parent;

	FLY_DEBUG_END("ASTBuilder", "CreateFailStmt");
	return Stmt;
}

ASTHandleStmt *ASTBuilder::CreateHandleStmt(
	ASTBlockStmt *Parent, const SourceLocation &Loc,
	ASTBlockStmt *BlockStmt, ASTIdentifier *ErrorRef) {
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

ASTDeleteStmt *ASTBuilder::CreateDeleteStmt(ASTBlockStmt *Parent, const SourceLocation &Loc, ASTIdentifier *VarRef) {
	FLY_DEBUG_MESSAGE("ASTBuilder", "CreateDeleteStmt", "Loc=" << Loc.getRawEncoding());

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