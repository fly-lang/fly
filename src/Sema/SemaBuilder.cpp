//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaBuilder.cpp - The Symbolic Table Builder
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaBuilder.h"

#include "AST/ASTBinary.h"
#include "AST/ASTCall.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
#include "AST/ASTEnumValue.h"
#include "AST/ASTFunction.h"
#include "AST/ASTTernary.h"
#include "AST/ASTUnary.h"
#include "AST/ASTVar.h"
#include "Basic/Debug.h"
#include "Basic/Diagnostic.h"
#include "Sema/SemaBinary.h"
#include "Sema/SemaCall.h"
#include "Sema/SemaClassAttribute.h"
#include "Sema/SemaClassMethod.h"
#include "Sema/SemaClassType.h"
#include "Sema/SemaComment.h"
#include "Sema/SemaEnumType.h"
#include "Sema/SemaEnumValue.h"
#include "Sema/SemaFunction.h"
#include "Sema/SemaMemberVar.h"
#include "Sema/SemaModule.h"
#include "Sema/SemaTernary.h"
#include "Sema/SemaType.h"
#include "Sema/SemaUnary.h"
#include "Sema/SemaValue.h"
#include "Sema/SymbolTable.h"

#include "llvm/Support/Regex.h"

#include <AST/ASTAttribute.h>
#include <AST/ASTBuilder.h>
#include <AST/ASTLocalVar.h>
#include <AST/ASTMethod.h>
#include <AST/ASTParam.h>
#include <AST/ASTValue.h>
#include <Sema/SemaBuilderModifiers.h>
#include <Sema/SemaBuiltin.h>
#include <Sema/SemaClassInstance.h>
#include <Sema/SemaLocalVar.h>
#include <Sema/SemaNameSpace.h>
#include <Sema/SemaParam.h>

using namespace fly;

SemaImport *SemaBuilder::CreateImport(SemaModule &Module, ASTImport &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateImport");

	// Search Namespace in Symbol Table
	SemaImport *Import = new SemaImport(AST);

	// Add Import to the Module Imports for next symbols resolution
	Module.addImport(Import);

	FLY_DEBUG_END("SemaBuilder", "CreateImport");
	return Import;
}

SemaFunction *SemaBuilder::CreateFunction(SemaModule &Module, SymbolTable *Symbols, ASTFunction &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateFunction");

	// Create the Function
	SemaFunction *Function = new SemaFunction(AST, Symbols);

	SemaBuilderModifiers *BuilderModifiers = SemaBuilderModifiers::Build(AST.getModifiers());
	Function->setVisibility(BuilderModifiers->getVisibility());

	// Add to Module
	Module.addNode(Function);

	FLY_DEBUG_END("SemaBuilder", "CreateFunction");
	return Function;
}

SemaClassType * SemaBuilder::CreateClass(SemaModule &Module, SymbolTable *Symbols, ASTClass &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateClass");

	// Create the Class Type
	SemaClassType *Class = new SemaClassType(AST, Symbols);

	// Set Symbol Table
	Class->Symbols = Symbols;

	// Create the 'this' attribute for the current class
	Class->This = CreateThisInstance(*Class);

	// Set Modifiers
	SemaBuilderModifiers *BuilderModifiers = SemaBuilderModifiers::Build(AST.getModifiers());
	Class->Constant = BuilderModifiers->isConstant();
	Class->Visibility = BuilderModifiers->getVisibility();

	// Add to Module
	Module.addNode(Class);

	FLY_DEBUG_END("SemaBuilder", "CreateClass");
	return Class;
}

SemaClassInstance *SemaBuilder::CreateThisInstance(SemaClassType &Class) {
	// Create the 'this' attribute for the current class
	return new SemaClassInstance(&Class);
}

SemaClassAttribute * SemaBuilder::CreateClassAttribute(SemaClassType &Class, ASTAttribute &AST, SemaType *Type) {
	FLY_DEBUG_START("SemaBuilder", "CreateClassAttribute");

	SemaClassAttribute *Attribute = new SemaClassAttribute(AST, Class, Type);
	Attribute->setParent(*Class.getThis());
	// Attribute->Type = AST->TypeRef->getSema(); // TODO add resolved symbol in the scope

	// Set Modifiers
	SemaBuilderModifiers *Builder = SemaBuilderModifiers::Build(AST.getModifiers());
	Attribute->Visibility = Builder->getVisibility();
	Attribute->Static = Builder->isStatic();
	Attribute->Constant = Builder->isConstant();

	AST.setSema(Attribute);

	FLY_DEBUG_END("SemaBuilder", "CreateClassAttribute");
	return Attribute;
}

SemaClassMethod * SemaBuilder::CreateDefaultConstructor(SemaClassType *Class) {
	// Create AST
	ASTMethod *AST = ASTBuilder::CreateDefaultConstructor(&Class->getAST());

	// Create Sema
	SemaClassMethod *Method = new SemaClassMethod(*AST, Class, Class->getThis(), SemaClassMethodKind::METHOD_CONSTRUCTOR);
	Method->setReturnType(SemaBuiltin::getVoidType());

	return Method;
}

SemaClassMethod * SemaBuilder::CreateClassMethod(SemaClassType *Class, ASTMethod &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateClassFunction");
	SemaClassMethod *Method;

	// When the Class Name is Equals to the Function Name this is a Constructor
	if (AST.getName() == Class->getName()) {
		Method = new SemaClassMethod(AST, Class, Class->getThis(), SemaClassMethodKind::METHOD_CONSTRUCTOR);
		Method->setReturnType(SemaBuiltin::getVoidType());
	} else {
		SemaClassMethodKind MethodKind = Class->getClassKind() == SemaClassKind::INTERFACE ?
			                 SemaClassMethodKind::METHOD_ABSTRACT : SemaClassMethodKind::METHOD;
		Method = new SemaClassMethod(AST, Class, Class->getThis(), MethodKind);

		// ClassDefinition Return Type
		Method->setReturnType(AST.getReturnType()->getSema());
	}

	// Set Modifiers
	SemaBuilderModifiers *Builder = SemaBuilderModifiers::Build(AST.getModifiers());
	Method->Visibility = Builder->getVisibility();
	Method->Static = Builder->isStatic();

	FLY_DEBUG_END("SemaBuilder", "CreateClassFunction");
	return Method;
}

SemaEnumType * SemaBuilder::CreateEnum(SemaModule &Module, SymbolTable *Symbols, ASTEnum &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateEnum");

	SemaEnumType *Enum = new SemaEnumType(AST, Symbols);

	// Set Symbol Table
	Enum->Symbols = Symbols;

	// Set Modifiers
	SemaBuilderModifiers *Builder = SemaBuilderModifiers::Build(AST.getModifiers());
	Enum->Visibility = Builder->getVisibility();
	Enum->Constant = Builder->isConstant();
	Enum->Comment = nullptr;

	// Add to Module
	Module.addNode(Enum);

	FLY_DEBUG_END("SemaBuilder", "CreateEnum");
	return Enum;
}

SemaEnumValue * SemaBuilder::CreateEnumValue(SemaEnumType *Enum, ASTEnumValue &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateEnumEntry");

	SemaEnumValue *Entry = new SemaEnumValue(AST, Enum);
	// EnumEntry->Index = Enum.Entries.size() + 1; TODO
	Enum->Entries.insert(std::make_pair(AST.getName(), Entry));

	FLY_DEBUG_END("SemaBuilder", "CreateEnumEntry");
	return Entry;
}

SemaComment * SemaBuilder::CreateComment(ASTComment &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateComment");

	SemaComment * Comment = new SemaComment(AST);

	FLY_DEBUG_END("SemaBuilder", "CreateComment");
	return Comment;
}

SemaLocalVar * SemaBuilder::CreateLocalVar(ASTLocalVar &AST, SemaType *Type) {
	FLY_DEBUG_START("SemaBuilder", "CreateLocalVar");

	// Create LocalVar Symbol
	SemaLocalVar *Sema = new SemaLocalVar(AST, Type);
	SemaBuilderModifiers *Builder = SemaBuilderModifiers::Build(AST.getModifiers());
	Sema->Constant = Builder->isConstant();

	// Assign Symbol to AST
	AST.setSema(Sema);

	FLY_DEBUG_END("SemaBuilder", "CreateLocalVar");
	return Sema;
}

SemaParam *SemaBuilder::CreateParam(ASTParam &AST, SemaType *Type) {
	FLY_DEBUG_START("SemaBuilder", "CreateParam");

	// Create LocalVar Symbol
	SemaParam *Sema = new SemaParam(AST, Type);

	// Assign Symbol to AST
	AST.setSema(Sema);

	FLY_DEBUG_END("SemaBuilder", "CreateParam");
	return Sema;
}

SemaMemberVar * SemaBuilder::CreateMemberVar(ASTVar &AST, SemaExpr &Parent, SemaClassAttribute *Attribute) {
	SemaMemberVar *Sema = new SemaMemberVar(AST, Parent, Attribute);

	return Sema;
}

SemaUnary *SemaBuilder::CreateUnary(ASTUnary &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateUnary");

	// Create Unary Symbol
	SemaUnary *Sema = new SemaUnary(AST);

	// Assign Symbol to AST
	AST.setSema(Sema);

	FLY_DEBUG_END("SemaBuilder", "CreateUnary");
	return Sema;
}

SemaBinary *SemaBuilder::CreateBinary(ASTBinary &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateBinary");

	// Create Unary Symbol
	SemaBinary *Sema = new SemaBinary(AST);

	// Assign Symbol to AST
	AST.setSema(Sema);

	FLY_DEBUG_END("SemaBuilder", "CreateBinary");
	return Sema;
}

SemaTernary *SemaBuilder::CreateTernary(ASTTernary &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateTernary");

	// Create Unary Symbol
	SemaTernary *Sema = new SemaTernary(AST);

	// Assign Symbol to AST
	AST.setSema(Sema);

	FLY_DEBUG_END("SemaBuilder", "CreateTernary");
	return Sema;
}

SemaValue * SemaBuilder::CreateDefaultValue(SemaType &Type) {
	FLY_DEBUG_START("ASTBuilder", "CreateDefaultValue");
	SemaValue *Sema = nullptr;

	if (Type.isBool()) {
		ASTBoolValue * AST = ASTBuilder::CreateBoolValue(SourceLocation(), false);
		Sema = CreateBoolValue(*AST);
		AST->setSema(Sema);
	}

	else if (Type.isInteger()) {
		SemaIntType *IntType = static_cast<SemaIntType *>(&Type);
		ASTNumberValue *AST = ASTBuilder::CreateNumberValue(SourceLocation(), "0");
		Sema =  CreateIntValue(*AST, IntType);
		AST->setSema(Sema);
	}

	else if (Type.isFloatingPoint()) {
		SemaFloatType *FloatType = static_cast<SemaFloatType *>(&Type);
		ASTNumberValue *AST = ASTBuilder::CreateNumberValue(SourceLocation(), "0.0");
		Sema =  CreateFloatValue(*AST, FloatType);
		AST->setSema(Sema);
	}

	else if (Type.isString()) {
		ASTStringValue *AST = ASTBuilder::CreateStringValue(SourceLocation(), "");
		Sema =  CreateStringValue(*AST);
		AST->setSema(Sema);
	}

	else if (Type.isArray()) {
		llvm::SmallVector<ASTValue *, 8> ASTValues;
		ASTArrayValue *AST = ASTBuilder::CreateArrayValue(SourceLocation(), ASTValues);
		llvm::SmallVector<SemaValue *, 8> Values;
		Sema =  CreateArrayValue(*AST, Values);
		AST->setSema(Sema);
	}

	else if (Type.isClass()) {
		ASTNullValue * AST = ASTBuilder::CreateNullValue(SourceLocation());
		Sema =  CreateNullValue(*AST);
		AST->setSema(Sema);
	}

	FLY_DEBUG_END("ASTBuilder", "CreateDefaultValue");
	return Sema;
}

SemaCall * SemaBuilder::CreateCall(ASTCall &AST, SemaType *Type, SemaFunctionBase *Function) {
	FLY_DEBUG_START("SemaBuilder", "CreateParam");

	// Create Call Symbol
	SemaCall *Call = new SemaCall(AST, Type);
	Call->Function = Function;

	// Assign Symbol to AST
	// AST.setSema(Call); // TODO add resolved symbol in the scope

	FLY_DEBUG_END("SemaBuilder", "CreateParam");
	return Call;
}

SemaBoolValue * SemaBuilder::CreateBoolValue(ASTBoolValue &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateBoolValue");

	SemaBoolValue * V = new SemaBoolValue(AST);

	FLY_DEBUG_END("SemaBuilder", "CreateBoolValue");
	return V;
}

SemaValue * SemaBuilder::CreateNumberValue(ASTNumberValue &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateNumberValue");

	SemaValue *Sema;

	// Floating point number
	llvm::Regex FloatRegex(R"(^[-+]?[0-9]*\.[0-9]+([eE][-+]?[0-9]+)?$)");
	if (FloatRegex.match(AST.getValue())) {
		// Floating point
		llvm::APFloat Value = llvm::APFloat(llvm::APFloat::IEEEdouble(), AST.getValue());
		Sema = new SemaFloatValue(AST, SemaBuiltin::getDoubleType(), Value);
	} else {
		llvm::APInt Value = CreateAPIntValue(AST.getValue());

		// Compute MinBits
		unsigned MinBits = Value.isNegative()
			? 1 + Value.getBitWidth() - Value.countLeadingOnes()
			: 1 + Value.getBitWidth() - Value.countLeadingZeros();

		// Infer Type
		SemaIntType *Type = nullptr;
		if (Value.isNegative()) {
			if (MinBits <= 16) Type = SemaBuiltin::getShortType();
			else if (MinBits <= 32) Type = SemaBuiltin::getIntType();
			else Type = SemaBuiltin::getLongType();
			Value = Value.sextOrTrunc(MinBits);
		} else {
			if (MinBits <= 8) Type = SemaBuiltin::getByteType();
			else if (MinBits <= 16) Type = SemaBuiltin::getUShortType();
			else if (MinBits <= 32) Type = SemaBuiltin::getUIntType();
			else Type = SemaBuiltin::getULongType();
			Value = Value.zextOrTrunc(MinBits);
		}

		// Final normalized value
		Sema = new SemaIntValue(AST, Type, Value);
	}

	FLY_DEBUG_END("SemaBuilder", "CreateNumberValue");
	return Sema;
}

llvm::APInt SemaBuilder::CreateAPIntValue(StringRef ValStr) {
	bool IsNegative = ValStr.startswith("-");
	if (IsNegative)
		ValStr = ValStr.drop_front(1);

	// Detect radix
	unsigned Radix = 10;
	if (ValStr.startswith("0x") || ValStr.startswith("0X")) {
		Radix = 16;
		ValStr = ValStr.drop_front(2);
	} else if (ValStr.startswith("0b") || ValStr.startswith("0B")) {
		Radix = 2;
		ValStr = ValStr.drop_front(2);
	}

	// Parse
	unsigned SrcBits = llvm::APInt::getBitsNeeded(ValStr, Radix);
	llvm::APInt Value(SrcBits, ValStr, Radix);
	return IsNegative ? -Value : Value;
}

SemaIntValue * SemaBuilder::CreateIntValue(ASTNumberValue &AST, SemaIntType *IntType) {
	llvm::APInt Value = CreateAPIntValue(AST.getValue());
	SemaIntValue *Sema = new SemaIntValue(AST, IntType, Value);
	return Sema;
}

SemaFloatValue * SemaBuilder::CreateFloatValue(ASTNumberValue &AST, SemaFloatType *FloatType) {
	llvm::APFloat Value = llvm::APFloat(llvm::APFloat::IEEEdouble(), AST.getValue());
	SemaFloatValue *Sema = new SemaFloatValue(AST, FloatType, Value);
	return Sema;
}

SemaStringValue * SemaBuilder::CreateStringValue(ASTStringValue &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateStringValue");

	SemaStringValue * V = new SemaStringValue(AST);
	V->Value = AST.getValue();
	V->Type = SemaBuiltin::getStringType();

	FLY_DEBUG_END("SemaBuilder", "CreateStringValue");
	return V;
}

SemaArrayValue * SemaBuilder::CreateArrayValue(ASTArrayValue &AST, llvm::SmallVector<SemaValue *, 8> &Values) {
	FLY_DEBUG_START("SemaBuilder", "CreateArrayValue");

	SemaType * Type = Values[0] ? Values[0]->getType() : nullptr;
	SemaArrayValue * V = new SemaArrayValue(AST, Type);
    V->Values = std::move(Values);

	FLY_DEBUG_END("SemaBuilder", "CreateArrayValue");
	return V;
}

SemaStructValue * SemaBuilder::CreateStructValue(ASTStructValue &AST, llvm::StringMap<SemaValue *> Values) {
	FLY_DEBUG_START("SemaBuilder", "CreateStructValue");

	llvm::SmallVector<SemaType *, 8> Types;
	for (auto &Entry : Values) {
		Types.push_back(Entry.second->getType());
	}

	//TODO: Create Struct Type from Types
	SemaStructValue * V = new SemaStructValue(AST, nullptr);

	FLY_DEBUG_END("SemaBuilder", "CreateStructValue");
	return V;
}

SemaValue * SemaBuilder:: CreateNullValue(ASTNullValue &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateNullValue");

	SemaValue * V = new SemaNullValue(AST);
	return V;
}
