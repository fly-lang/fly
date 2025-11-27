//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaBuilder.cpp - The Symbolic Table Builder
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/Sema.h"
#include "Sema/SymbolTable.h"
#include "Sema/SemaModule.h"
#include "Sema/SemaBuilder.h"
#include "Basic/Debug.h"
#include "Basic/Diagnostic.h"
#include "Sema/SemaClassType.h"
#include "Sema/SemaEnumType.h"
#include "Sema/SemaFunction.h"
#include "Sema/SemaGlobalVar.h"
#include "Sema/SemaType.h"
#include "Sema/SemaMemberVar.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
#include "AST/ASTFunction.h"
#include "AST/ASTVar.h"
#include "Sema/SemaClassAttribute.h"
#include "Sema/SemaClassMethod.h"
#include "Sema/SemaEnumEntry.h"
#include "Sema/SemaComment.h"
#include "Sema/SemaValue.h"
#include "Sema/SemaCall.h"
#include "llvm/Support/Regex.h"

#include <AST/ASTBuilder.h>
#include <AST/ASTBuilderStmt.h>
#include <AST/ASTNameSpace.h>
#include <AST/ASTValue.h>
#include <Sema/Helper.h>
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
	Module.Imports.push_back(Import);

	FLY_DEBUG_END("SemaBuilder", "CreateImport");
	return Import;
}

SemaFunction *SemaBuilder::CreateFunction(SemaModule &Module, SymbolTable *Symbols, ASTFunction &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateFunction");

	// Create the Function
	SemaFunction *Function = new SemaFunction(AST, Symbols);

	// Set Symbol Table
	Function->Symbols = Symbols;

	SemaBuilderModifiers *BuilderModifiers = SemaBuilderModifiers::Build(AST.getModifiers());
	Function->Visibility = BuilderModifiers->getVisibility();

	// Add to Module
	Module.Nodes.push_back(Function);

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
	Module.Nodes.push_back(Class);

	FLY_DEBUG_END("SemaBuilder", "CreateClass");
	return Class;
}

SemaClassInstance *SemaBuilder::CreateThisInstance(SemaClassType &Class) {
	// Create the 'this' attribute for the current class
	return new SemaClassInstance(&Class);
}

SemaClassAttribute * SemaBuilder::CreateClassAttribute(SemaClassType &Class, ASTVar &AST, SemaComment &Comment) {
	FLY_DEBUG_START("SemaBuilder", "CreateClassAttribute");

	SemaClassAttribute *Attribute = new SemaClassAttribute(AST, Class);
	Attribute->setParent(*Class.getThis());
	// Attribute->Type = AST->TypeRef->getSema(); // TODO add resolved symbol in the scope

	// Set Modifiers
	SemaBuilderModifiers *Builder = SemaBuilderModifiers::Build(AST.getModifiers());
	Attribute->Visibility = Builder->getVisibility();
	Attribute->Static = Builder->isStatic();
	Attribute->Constant = Builder->isConstant();

	Attribute->Comment = &Comment;

	FLY_DEBUG_END("SemaBuilder", "CreateClassAttribute");
	return Attribute;
}

SemaClassMethod * SemaBuilder::CreateClassMethod(SemaClassType *Class, ASTFunction &AST, SemaComment &Comment) {
	FLY_DEBUG_START("SemaBuilder", "CreateClassFunction");

	SemaClassMethod *Method;
	// When the Class Name is Equals to the Function Name this is a Constructor
	if (AST.getName() == Class->getName()) {
		Method = new SemaClassMethod(AST, Class, Class->getThis(), SemaClassMethodKind::METHOD_CONSTRUCTOR);
		Method->ReturnType = SemaBuiltin::getVoidType();
	} else {
		SemaClassMethodKind MethodKind = Class->getClassKind() == SemaClassKind::INTERFACE ?
			                 SemaClassMethodKind::METHOD_ABSTRACT : SemaClassMethodKind::METHOD;
		Method = new SemaClassMethod(AST, Class, Class->getThis(), MethodKind);

		// ClassDefinition Return Type
		Method->ReturnType = AST.getReturnTypeRef()->getSema();
	}

	// Set Modifiers
	SemaBuilderModifiers *Builder = SemaBuilderModifiers::Build(AST.getModifiers());
	Method->Visibility = Builder->getVisibility();
	Method->Static = Builder->isStatic();
	Method->Comment = &Comment;

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
	Module.Nodes.push_back(Enum);

	FLY_DEBUG_END("SemaBuilder", "CreateEnum");
	return Enum;
}

SemaEnumEntry * SemaBuilder::CreateEnumEntry(SemaEnumType &Enum, ASTVar &AST, SemaComment *Comment) {
	FLY_DEBUG_START("SemaBuilder", "CreateEnumEntry");

	SemaEnumEntry *Entry = new SemaEnumEntry(AST);
	// EnumEntry->Index = Enum.Entries.size() + 1; TODO
	Enum.Entries.insert(std::make_pair(AST.getName(), Entry));
	Entry->Comment = Comment;

	FLY_DEBUG_END("SemaBuilder", "CreateEnumEntry");
	return Entry;
}

SemaComment * SemaBuilder::CreateComment(ASTComment &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateComment");

	SemaComment * Comment = new SemaComment(AST);

	FLY_DEBUG_END("SemaBuilder", "CreateComment");
	return Comment;
}

SemaLocalVar * SemaBuilder::CreateLocalVar(ASTVar &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateLocalVar");

	// Create LocalVar Symbol
	SemaLocalVar *Sema = new SemaLocalVar(AST);
	SemaBuilderModifiers *Builder = SemaBuilderModifiers::Build(AST.getModifiers());
	Sema->Constant = Builder->isConstant();

	// Assign Symbol to AST
	AST.setSema(Sema);

	FLY_DEBUG_END("SemaBuilder", "CreateLocalVar");
	return Sema;
}

SemaParam *SemaBuilder::CreateParam(ASTVar &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateParam");

	// Create LocalVar Symbol
	SemaParam *Sema = new SemaParam(AST);

	// Assign Symbol to AST
	AST.setSema(Sema);

	FLY_DEBUG_END("SemaBuilder", "CreateParam");
	return Sema;
}

SemaMemberVar * SemaBuilder::CreateMemberVar(ASTMember &AST, SemaResult &Parent) {
	SemaMemberVar *Sema = new SemaMemberVar(AST, Parent);

	return Sema;
}

SemaValue * SemaBuilder::CreateDefaultValue(SemaType &Type) {
	FLY_DEBUG_START("ASTBuilder", "CreateDefaultValue");

	if (Type.isBool()) {
		ASTBoolValue * AST = ASTBuilder::CreateBoolValue(SourceLocation(), false);
		return CreateBoolValue(*AST);
	}

	if (Type.isInteger()) {
		ASTNumberValue *AST = ASTBuilder::CreateNumberValue(SourceLocation(), "0");
		return CreateNumberValue(*AST);
	}

	if (Type.isFloatingPoint()) {
		ASTNumberValue *AST = ASTBuilder::CreateNumberValue(SourceLocation(), "0.0");
		return CreateNumberValue(*AST);
	}

	if (Type.isString()) {
		ASTStringValue *AST = ASTBuilder::CreateStringValue(SourceLocation(), "");
		return CreateStringValue(*AST);
	}

	if (Type.isArray()) {
		llvm::SmallVector<ASTValue *, 8> Values;
		ASTArrayValue *AST = ASTBuilder::CreateArrayValue(SourceLocation(), Values);
		return CreateArrayValue(*AST);
	}

	if (Type.isClass()) {
		ASTNullValue * AST = ASTBuilder::CreateNullValue(SourceLocation());
		return CreateNullValue(*AST);
	}

	FLY_DEBUG_END("ASTBuilder", "CreateDefaultValue");

	assert("Unknown type");
	return nullptr;
}

SemaCall * SemaBuilder::CreateCall(ASTCall &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateParam");

	// Create Call Symbol
	SemaCall *Call = new SemaCall(AST);

	// Assign Symbol to AST
	// AST->setSema(Call); // TODO add resolved symbol in the scope

	FLY_DEBUG_END("SemaBuilder", "CreateParam");
	return Call;
}

SemaBoolValue * SemaBuilder::CreateBoolValue(ASTBoolValue &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateBoolValue");

	SemaBoolValue * V = new SemaBoolValue(AST);
	V->Type = SemaBuiltin::getBoolType();

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
		SemaFloatValue * SemaFloat = new SemaFloatValue(AST);
		SemaFloat->Value = llvm::APFloat(llvm::APFloat::IEEEdouble(), AST.getValue());
		SemaFloat->Type = SemaBuiltin::getDoubleType();

		Sema = SemaFloat;
	} else {

		SemaIntValue *SemaInt = new SemaIntValue(AST);

		llvm::StringRef ValStr = AST.getValue();
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
		llvm::APInt I(llvm::APInt::getBitsNeeded(ValStr, Radix), ValStr, Radix);
		if (IsNegative)
			I = -I;

		// Compute MinBits
		unsigned MinBits = IsNegative
			? 1 + I.getBitWidth() - I.countLeadingOnes()
			: 1 + I.getBitWidth() - I.countLeadingZeros();

		// Infer Type
		if (IsNegative) {
			if (MinBits <= 16) SemaInt->Type = SemaBuiltin::getShortType();
			else if (MinBits <= 32) SemaInt->Type = SemaBuiltin::getIntType();
			else SemaInt->Type = SemaBuiltin::getLongType();
		} else {
			if (MinBits <= 8) SemaInt->Type = SemaBuiltin::getByteType();
			else if (MinBits <= 16) SemaInt->Type = SemaBuiltin::getUShortType();
			else if (MinBits <= 32) SemaInt->Type = SemaBuiltin::getUIntType();
			else SemaInt->Type = SemaBuiltin::getULongType();
		}

		// Final normalized value
		SemaInt->Value = I.zextOrTrunc(MinBits);

		Sema = SemaInt;
	}

	FLY_DEBUG_END("SemaBuilder", "CreateNumberValue");
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

SemaArrayValue * SemaBuilder::CreateArrayValue(ASTArrayValue &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateArrayValue");

	SemaArrayValue * V = new SemaArrayValue(AST);
	V->Type = AST.getValues().empty() ? nullptr : AST.getValues()[0]->getSema()->getType();

	FLY_DEBUG_END("SemaBuilder", "CreateArrayValue");
	return V;
}

SemaStructValue * SemaBuilder::CreateStructValue(ASTStructValue &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateStructValue");

	const llvm::StringMap<SemaValue *> Values;
	SemaStructValue * V = new SemaStructValue(AST);
	if (AST.getValues().empty()) {
		V->Type = nullptr;
	} else {
		// TODO
		for (auto &Entry : AST.getValues()) {

		}
		// V->Type = S.getSemaBuilder().CreateClass();
	}

	FLY_DEBUG_END("SemaBuilder", "CreateStructValue");
	return V;
}

SemaValue * SemaBuilder:: CreateNullValue(ASTNullValue &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateNullValue");

	SemaValue * V = new SemaNullValue(AST);
	return V;
}
