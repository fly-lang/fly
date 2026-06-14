//===--------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTBuilder.cpp - AST builder factory methods
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
#include "AST/ASTCast.h"
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
#include <AST/ASTCaseStmt.h>
#include <AST/ASTRuleStmt.h>
#include <AST/ASTTestStmt.h>
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
#include <utility>

using namespace fly;

/**
 * Private constructor used only from AST constructor
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
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "GenerateModule", "Name=" << F->getName());

	ASTModule *Module = new ASTModule(F);

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
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateHeader", "Name=" << F->getName());

	ASTModule *Module = new ASTModule(F);
	return Module;
}

ASTComment *ASTBuilder::CreateComment(ASTModule *Module, const SourceLocation &Loc, llvm::StringRef Content) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateComment", "Loc" << Loc.getRawEncoding() << " Content=" << Content);
	ASTComment *Comment = new ASTComment(Loc, Content);

	// Add Comment to Module
	Module->addNode(Comment);
	return Comment;
}

ASTName *ASTBuilder::CreateName(llvm::StringRef Name, const SourceLocation &Loc) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateName", "Loc" << Loc.getRawEncoding() << " Name=" << Name);
	ASTName *AST = new ASTName(Loc, Name);
	return AST;
}

ASTNameSpace *ASTBuilder::CreateNameSpace(
	ASTModule *Module, const SourceLocation &Loc, llvm::SmallVector<ASTName *, 4> Names) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateNameSpace", "Loc=" << Loc.getRawEncoding());
	Module->NameSpace = new ASTNameSpace(Loc, Names);
	return Module->NameSpace;
}

ASTImport *ASTBuilder::CreateImport(
	ASTModule *Module, const SourceLocation &Loc, llvm::SmallVector<ASTName *, 4> Names,
	llvm::SmallVector<ASTName *, 4> Alias, bool Wildcard) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateImport", "Loc=" << Loc.getRawEncoding());

	ASTImport *Import = new ASTImport(Loc, Names, Alias, Wildcard);

	// Add Import to Module
	Module->addNode(Import);
	return Import;
}

/**
 * Creates an ASTFunction
 * @param Module
 * @param Loc
 * @param Name
 * @param Modifiers
 * @return
 */
ASTFunction *ASTBuilder::CreateFunction(
	ASTModule *Module, const SourceLocation &Loc,
	llvm::StringRef Name, llvm::SmallVector<ASTModifier *, 8> &Modifiers,
	SmallVector<ASTParam *, 8> &Params, ASTBlockStmt *Body) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateFunction", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTFunction *Function = new ASTFunction(Loc, Modifiers, Name, Params);

	// Create Body
	if (Body)
		CreateBody(Function, Body);

	// Add Function to Module
	Module->addNode(Function);
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
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateClass", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTClass *Class = new ASTClass(ClassKind, Modifiers, Loc, Name, Bases);

	// Add Class to Module
	Module->addNode(Class);
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
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateClassAttribute", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTAttribute *Attribute = new ASTAttribute(Loc, TypeRef, Name, Modifiers);
	Attribute->Expr = Expr;
	Class->Nodes.push_back(Attribute);
	return Attribute;
}

ASTMethod *ASTBuilder::CreateDefaultConstructor(ASTClass *Class) {
	SourceLocation Loc = SourceLocation();

	// Create Modifiers
	llvm::SmallVector<ASTModifier *, 8> Modifiers;
	Modifiers.push_back(CreateModifier(Loc, ASTModifierKind::MOD_PUBLIC));

	// Empty Params
	llvm::SmallVector<ASTParam *, 8> Params;

	// Create the Default Constructor
	ASTMethod *Method = new ASTMethod(Loc, Modifiers, Class->getName(), Params);

	// Add empty body
	ASTBlockStmt *Body = CreateBlockStmt(Loc);
	CreateBody(Method, Body);

	// Note: intentionally NOT added to Class->Nodes here.
	// Resolver::CreateDefaultConstructor() registers this method at the Sema level
	// (symbol table + method list). Adding it to Nodes would cause specializations that
	// share the same template ASTClass to visit it again, producing duplicate symbols.

	return Method;
}

ASTMethod *ASTBuilder::CreateClassMethod(
	const SourceLocation &Loc, ASTClass *Class,
	llvm::StringRef Name, llvm::SmallVector<ASTModifier *, 8> &Modifiers,
	llvm::SmallVector<ASTParam *, 8> &Params, ASTBlockStmt *Body) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateClassMethod", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTMethod *Method = new ASTMethod(Loc, Modifiers, Name, Params);

	if (Body)
		CreateBody(Method, Body);

	// Add to Class Methods
	Class->Nodes.push_back(Method);
	return Method;
}

ASTEnum *ASTBuilder::CreateEnum(
	ASTModule *Module, const SourceLocation &Loc, const llvm::StringRef Name,
	llvm::SmallVector<ASTModifier *, 8> &Modifiers,
	llvm::SmallVector<ASTType *, 4> EnumTypes) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateEnum", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTEnum *Enum = new ASTEnum(Loc, Name, Modifiers, EnumTypes);

	// Add Enum to Module
	Module->addNode(Enum);
	return Enum;
}

ASTEnumEntry *ASTBuilder::CreateEnumEntry(
	const SourceLocation &Loc, ASTEnum *Enum, llvm::StringRef Name,
	llvm::SmallVector<ASTModifier *, 8> &Modifiers) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateEnumEntry", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTEnumEntry *EnumEntry = new ASTEnumEntry(Loc, Enum, Name);
	// Index starts from 1 since 0 is reserved for 'unset' keyword value
	EnumEntry->setIndex(Enum->Nodes.size() + 1);
	Enum->Nodes.push_back(EnumEntry);
	return EnumEntry;
}

/**
 * Creates a Modifier
 * @param Loc
 * @param Kind
 * @return
 */
ASTModifier *ASTBuilder::CreateModifier(const SourceLocation &Loc, ASTModifierKind Kind) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateModifier", "Loc=" << Loc.getRawEncoding() << ", Kind=" << static_cast<size_t>(Kind));

	ASTModifier *Modifier = new ASTModifier(Loc, Kind);
	return Modifier;
}

/**
 * Creates a bool type
 * @param Loc
 * @return
 */
ASTType *ASTBuilder::CreateBoolType(const SourceLocation &Loc) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateBoolTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_BOOL);
	return TypeRef;
}

/**
 * Creates an byte type
 * @param Loc
 * @return
 */
ASTType *ASTBuilder::CreateByteType(const SourceLocation &Loc) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateByteTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_BYTE);
	return TypeRef;
}

/**
 * Creates an unsigned short type
 * @param Loc
 * @return
 */
ASTType *ASTBuilder::CreateUShortType(const SourceLocation &Loc) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateUShortTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_USHORT);
	return TypeRef;
}

/**
 * Create a short type
 * @param Loc
 * @return
 */
ASTType *ASTBuilder::CreateShortType(const SourceLocation &Loc) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateShortTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_SHORT);
	return TypeRef;
}

/**
 * Creates an unsigned int type
 * @param Loc
 * @return
 */
ASTType *ASTBuilder::CreateUIntType(const SourceLocation &Loc) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateUIntTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_UINT);
	return TypeRef;
}

/**
 * Creates an int type
 * @param Loc
 * @return
 */
ASTType *ASTBuilder::CreateIntType(const SourceLocation &Loc) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateIntTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_INT);
	return TypeRef;
}

/**
 * Creates an unsigned long type
 * @param Loc
 * @return
 */
ASTType *ASTBuilder::CreateULongType(const SourceLocation &Loc) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateULongTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_ULONG);
	return TypeRef;
}

/**
 * Creates a long type
 * @param Loc
 * @return
 */
ASTType *ASTBuilder::CreateLongType(const SourceLocation &Loc) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateLongTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_LONG);
	return TypeRef;
}

/**
 * Creates a float type
 * @param Loc
 * @return
 */
ASTType *ASTBuilder::CreateFloatType(const SourceLocation &Loc) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateFloatTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_FLOAT);
	return TypeRef;
}

/**
 * Creates a double type
 * @param Loc
 * @return
 */
ASTType *ASTBuilder::CreateDoubleType(const SourceLocation &Loc) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateDoubleTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_DOUBLE);
	return TypeRef;
}

ASTType *ASTBuilder::CreateComplexType(const SourceLocation &Loc) {
	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_COMPLEX);
	return TypeRef;
}

/**
 * Creates a void type
 * @param Loc
 * @return
 */
ASTType *ASTBuilder::CreateVoidType(const SourceLocation &Loc) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateVoidTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_VOID);
	return TypeRef;
}

ASTType *ASTBuilder::CreateStringType(const SourceLocation &Loc) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateStringTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_STRING);
	return TypeRef;
}

ASTType *ASTBuilder::CreateErrorType(const SourceLocation &Loc) {
	FLY_DEBUG_SCOPE("ASTBuilder", "CreateErrorTypeRef");

	ASTType *TypeRef = new ASTBuiltinType(Loc, ASTBuiltinTypeKind::TYPE_ERROR);
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
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateArrayTypeRef", "Loc=" << Loc.getRawEncoding());

	ASTArrayType *ArrayTypeRef = new ASTArrayType(Loc, ElementType, SizeExpr);
	return ArrayTypeRef;
}

/**
 * Creates a Type
 * @param Loc
 * @param Name
 * @return
 */
ASTType *ASTBuilder::CreateType(const SourceLocation &Loc, llvm::SmallVector<ASTName *, 4> Names) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateType", "Loc=" << Loc.getRawEncoding());

	ASTType *T = new ASTNamedType(Loc, Names);
	return T;
}

/**
 * Creates a null value
 * @param Loc
 * @return
 */
ASTNullValue *ASTBuilder::CreateNullValue(const SourceLocation &Loc) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateNullValue", "Loc=" << Loc.getRawEncoding());

	ASTNullValue *Value = new ASTNullValue(Loc);
	return Value;
}

/**
 * Creates an unset value (for uninitialized enums)
 * @param Loc
 * @return
 */
ASTUnsetValue *ASTBuilder::CreateUnsetValue(const SourceLocation &Loc) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateUnsetValue", "Loc=" << Loc.getRawEncoding());

	ASTUnsetValue *Value = new ASTUnsetValue(Loc);
	return Value;
}

/**
 * Creates a bool value
 * @param Loc
 * @param Val
 * @return
 */
ASTBoolValue *ASTBuilder::CreateBoolValue(const SourceLocation &Loc, bool Val) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateBoolValue", "Loc=" << Loc.getRawEncoding() << ", Val=" << Val);

	ASTBoolValue *Value = new ASTBoolValue(Loc, Val);
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
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateIntegerValue", "Loc=" << Loc.getRawEncoding() << ", Val=" << Val);

	ASTNumberValue *Value = new ASTNumberValue(Loc, Val);
	return Value;
}

ASTStringValue *ASTBuilder::CreateStringValue(const SourceLocation &Loc, llvm::StringRef Val) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateStringValue", "Loc=" << Loc.getRawEncoding() << ", Val=" << Val);

	ASTStringValue *Value = new ASTStringValue(Loc, Val);
	return Value;
}

/**
 * Create an array value
 * @param Loc
 * @return
 */
ASTArrayValue *ASTBuilder::CreateArrayValue(const SourceLocation &Loc, llvm::SmallVector<ASTValue *, 8> Values) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateArrayValue", "Loc=" << Loc.getRawEncoding());

	ASTArrayValue *Array = new ASTArrayValue(Loc);
	Array->Values = std::move(Values);
	return Array;
}

ASTStructValue *ASTBuilder::CreateStructValue(const SourceLocation &Loc, llvm::StringMap<ASTValue *> Values) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateArrayValue", "Loc=" << Loc.getRawEncoding());

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
ASTParam *ASTBuilder::CreateParam(
	const SourceLocation &Loc, ASTType *TypeRef, llvm::StringRef Name,
	llvm::SmallVector<ASTModifier *, 8> &Modifiers, ASTValue *DefaultValue) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateParam", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTParam *Param = new ASTParam(Loc, TypeRef, Name, Modifiers);
	Param->setExpr(DefaultValue);
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
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateLocalVar", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

	ASTLocalVar *Var = new ASTLocalVar(Loc, Type, Name, Modifiers);
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
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateCall", "Loc=" << Loc.getRawEncoding() << ", Name=" << Name);

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
	return Call;
}

ASTIdentifier *ASTBuilder::CreateIdentifier(ASTVar *Var) {
	FLY_DEBUG_SCOPE("ASTBuilder", "CreateVarRef");

	ASTIdentifier *Identifier = new ASTIdentifier(Var->getLocation(), Var->getName());
	Identifier->Var = Var;
	return Identifier;
}

ASTIdentifier *ASTBuilder::CreateIdentifier(const SourceLocation &Loc, llvm::StringRef Name, ASTExpr *Parent) {
	FLY_DEBUG_SCOPE("ASTBuilder", "CreateIdentifier");

	ASTIdentifier *Identifier = new ASTIdentifier(Loc, Name);
	Identifier->Parent = Parent;
	return Identifier;
}

ASTMember *ASTBuilder::CreateMember(const SourceLocation &Loc, llvm::StringRef Name, ASTExpr *Parent) {
	FLY_DEBUG_SCOPE("ASTBuilder", "CreateMember");

	ASTMember *Member = new ASTMember(Loc, Name, Parent);
	return Member;
}

ASTUnary *ASTBuilder::CreateUnary(const SourceLocation &Loc, ASTUnaryKind OpKind, ASTExpr *Expr) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateUnaryOpExpr", "Loc=" << Loc.getRawEncoding() << ", OpKind=" << static_cast<uint8_t>(OpKind));

	ASTUnary *UnaryOpExpr = new ASTUnary(Loc, OpKind, Expr);
	return UnaryOpExpr;
}

ASTCast *ASTBuilder::CreateCast(ASTExpr *Expr, ASTType *ToType) {
	FLY_DEBUG_SCOPE("ASTBuilder", "CreateCast");

	ASTCast *Cast = new ASTCast(Expr, ToType);
	return Cast;
}

ASTBinary *ASTBuilder::CreateBinary(
	const SourceLocation &OpLocation, ASTBinaryKind OpKind,
	ASTExpr *LeftExpr, ASTExpr *RightExpr) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateBinaryOpExpr", "OpLocation=" << OpLocation.getRawEncoding() << ", OpKind=" << static_cast<uint8_t>(OpKind));

	ASTBinary *BinaryOpExpr = new ASTBinary(OpKind, OpLocation, LeftExpr, RightExpr);
	return BinaryOpExpr;
}

ASTTernary *ASTBuilder::CreateTernary(
	ASTExpr *ConditionExpr,
	const SourceLocation &TrueOpLocation, ASTExpr *TrueExpr,
	const SourceLocation &FalseOpLocation, ASTExpr *FalseExpr) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateBinaryOpExpr", "TrueOpLocation=" << TrueOpLocation.getRawEncoding() << "FalseOpLocation=" << FalseOpLocation.getRawEncoding());

	ASTTernary *TernaryExpr = new ASTTernary(
		ConditionExpr,
		TrueOpLocation, TrueExpr,
		FalseOpLocation, FalseExpr);
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
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateDeclStmt", "Loc=" << Loc.getRawEncoding());
	ASTDeclStmt *Stmt = new ASTDeclStmt(Loc, Var);
	Parent->addContent(Stmt);
	return Stmt;
}

/**
 * Creates a ASTReturnStmt
 * @param Parent
 * @param Loc
 * @return
 */
ASTReturnStmt *ASTBuilder::CreateReturnStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateReturnStmt", "Loc=" << Loc.getRawEncoding());

	ASTReturnStmt *Stmt = new ASTReturnStmt(Loc);
	Parent->addContent(Stmt);
	return Stmt;
}

/**
 * Creates an ASTExprStmt
 * @param Loc
 * @return
 */
ASTExprStmt *ASTBuilder::CreateExprStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateExprStmt", "Loc=" << Loc.getRawEncoding());
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
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateFailStmt", "Loc=" << Loc.getRawEncoding());

	ASTFailStmt *Stmt = new ASTFailStmt(Loc);
	Parent->addContent(Stmt);
	return Stmt;
}

ASTHandleStmt *ASTBuilder::CreateHandleStmt(
	ASTBlockStmt *Parent, const SourceLocation &Loc,
	ASTBlockStmt *BlockStmt) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateHandleStmt", "Loc=" << Loc.getRawEncoding());

	ASTHandleStmt *HandleStmt = new ASTHandleStmt(Loc);
	Parent->addContent(HandleStmt);
	HandleStmt->Handle = BlockStmt;

	// set Handle Block
	BlockStmt->Parent = HandleStmt;
	return HandleStmt;
}

ASTBreakStmt *ASTBuilder::CreateBreakStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateLocalVar", "Loc=" << Loc.getRawEncoding());

	ASTBreakStmt *Break = new ASTBreakStmt(Loc);
	// Inner Stmt
	Parent->Content.push_back(Break);
	Break->Parent = Parent;
	return Break;
}

ASTContinueStmt *ASTBuilder::CreateContinueStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateContinueStmt", "Loc=" << Loc.getRawEncoding());

	ASTContinueStmt *Continue = new ASTContinueStmt(Loc);
	// Inner Stmt
	Parent->Content.push_back(Continue);
	Continue->Parent = Parent;
	return Continue;
}

ASTDeleteStmt *ASTBuilder::CreateDeleteStmt(ASTBlockStmt *Parent, const SourceLocation &Loc, ASTIdentifier *VarRef) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateDeleteStmt", "Loc=" << Loc.getRawEncoding());

	ASTDeleteStmt *Delete = new ASTDeleteStmt(Loc, VarRef);
	// Inner Stmt
	Parent->Content.push_back(Delete);
	Delete->Parent = Parent;
	return Delete;
}

ASTBlockStmt *
ASTBuilder::CreateBody(ASTFunction *Function, ASTBlockStmt *Body) {
	FLY_DEBUG_SCOPE("ASTBuilder", "CreateBody");

	Body->Parent = nullptr; // body cannot have a parent stmt
	Function->Body = Body;
	return Function->Body;
}

ASTBlockStmt *ASTBuilder::CreateBlockStmt(const SourceLocation &Loc) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateBlockStmt", "Loc=" << Loc.getRawEncoding());

	ASTBlockStmt *Block = new ASTBlockStmt(Loc);
	return Block;
}

ASTBlockStmt *ASTBuilder::CreateBlockStmt(ASTStmt *Parent, const SourceLocation &Loc) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateBlockStmt", "Loc=" << Loc.getRawEncoding());

	ASTBlockStmt *Block = new ASTBlockStmt(Loc);
	Block->Parent = Parent;
	return Block;
}

ASTCaseStmt *ASTBuilder::CreateCaseStmt(ASTBlockStmt *Parent, const SourceLocation &Loc,
                                          ASTExpr *Expr, ASTBlockStmt *Body) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateCaseStmt", "Loc=" << Loc.getRawEncoding());
	ASTCaseStmt *Case = new ASTCaseStmt(Loc);
	Case->Expr = Expr;
	Case->Stmt = Body;
	Parent->addContent(Case);
	return Case;
}

ASTTestStmt *ASTBuilder::CreateTestStmt(ASTBlockStmt *Parent, const SourceLocation &Loc) {
	FLY_DEBUG_SCOPE_MSG("ASTBuilder", "CreateTestStmt", "Loc=" << Loc.getRawEncoding());
	ASTTestStmt *Stmt = new ASTTestStmt(Loc);
	ASTBlockStmt *Body = new ASTBlockStmt(Loc);
	Stmt->Body = Body;
	Body->Parent = Stmt;
	Parent->addContent(Stmt);
	return Stmt;
}

