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

#include <AST/ASTValue.h>
#include <Sema/SemaBuilderModifiers.h>
#include <Sema/SemaBuiltin.h>
#include <Sema/SemaClassInstance.h>
#include <Sema/SemaLocalVar.h>
#include <Sema/SemaParam.h>

using namespace fly;

SemaFunction * SemaBuilder::CreateFunction(ASTFunction &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateFunction");

	SemaFunction *Function = new SemaFunction(AST);

	SemaBuilderModifiers *BuilderModifiers = SemaBuilderModifiers::Build(AST.getModifiers());
	Function->Visibility = BuilderModifiers->getVisibility();

	FLY_DEBUG_END("SemaBuilder", "CreateFunction");
	return Function;
}

SemaClassType * SemaBuilder::CreateClass(ASTClass &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateClass");

	// Create the Class Type
	SemaClassType *Class = new SemaClassType(AST);

	// Create the 'this' attribute for the current class
	Class->This = CreateThisInstance(Class);

	// Set Modifiers
	SemaBuilderModifiers *BuilderModifiers = SemaBuilderModifiers::Build(AST.getModifiers());
	Class->Constant = BuilderModifiers->isConstant();
	Class->Visibility = BuilderModifiers->getVisibility();

	FLY_DEBUG_END("SemaBuilder", "CreateClass");
	return Class;
}

SemaClassInstance *SemaBuilder::CreateThisInstance(SemaClassType *Class) {
	// Create the 'this' attribute for the current class
	return new SemaClassInstance(Class);
}

SemaClassAttribute * SemaBuilder::CreateClassAttribute(SemaClassType *Class, SemaClassInstance *This, ASTVar *AST, SemaComment *Comment) {
	FLY_DEBUG_START("SemaBuilder", "CreateClassAttribute");

	SemaClassAttribute *Attribute = new SemaClassAttribute(AST, Class);
	Attribute->setParent(This);
	Attribute->Type = AST->TypeRef->getSema();

	// Set Modifiers
	SemaBuilderModifiers *Builder = SemaBuilderModifiers::Build(AST->getModifiers());
	Attribute->Visibility = Builder->getVisibility();
	Attribute->Static = Builder->isStatic();
	Attribute->Constant = Builder->isConstant();

	AST->Sema = Attribute;
	Attribute->Comment = Comment;

	FLY_DEBUG_END("SemaBuilder", "CreateClassAttribute");
	return Attribute;
}

SemaClassMethod * SemaBuilder::CreateClassMethod(SemaClassType *Class, SemaClassInstance *This, ASTFunction *AST, SemaComment *Comment) {
	FLY_DEBUG_START("SemaBuilder", "CreateClassFunction");

	SemaClassMethod *Method;
	// When the Class Name is Equals to the Function Name this is a Constructor
	if (AST->getName() == Class->getName()) {
		Method = new SemaClassMethod(AST, Class, This, SemaClassMethodKind::METHOD_CONSTRUCTOR);
		Method->ReturnType = SemaBuiltin::getVoidType();
	} else {
		SemaClassMethodKind MethodKind = Class->getClassKind() == SemaClassKind::INTERFACE ?
			                 SemaClassMethodKind::METHOD_ABSTRACT : SemaClassMethodKind::METHOD;
		Method = new SemaClassMethod(AST, Class, This, MethodKind);

		// ClassDefinition Return Type
		Method->ReturnType = AST->ReturnTypeRef->getSema();
	}

	// Set Modifiers
	SemaBuilderModifiers *Builder = SemaBuilderModifiers::Build(AST->getModifiers());
	Method->Visibility = Builder->getVisibility();
	Method->Static = Builder->isStatic();

	AST->Sema = Method;
	Method->Comment = Comment;

	FLY_DEBUG_END("SemaBuilder", "CreateClassFunction");
	return Method;
}

SemaEnumType * SemaBuilder::CreateEnum(ASTEnum &AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateEnum");

	SemaEnumType *Enum = new SemaEnumType(AST);

	// Set Modifiers
	SemaBuilderModifiers *Builder = SemaBuilderModifiers::Build(AST.getModifiers());
	Enum->Visibility = Builder->getVisibility();
	Enum->Constant = Builder->isConstant();

	FLY_DEBUG_END("SemaBuilder", "CreateEnum");
	return Enum;
}

SemaEnumEntry * SemaBuilder::CreateEnumEntry(SemaEnumType *Enum, ASTVar *AST, SemaComment *Comment) {
	FLY_DEBUG_START("SemaBuilder", "CreateEnumEntry");

	SemaEnumEntry *Entry = new SemaEnumEntry(AST);
	// EnumEntry->Index = Enum.Entries.size() + 1; TODO
	Enum->Entries.insert(std::make_pair(AST->getName(), Entry));
	Entry->Comment = Comment;

	FLY_DEBUG_END("SemaBuilder", "CreateEnumEntry");
	return Entry;
}

SemaComment * SemaBuilder::CreateComment(ASTComment *AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateComment");

	SemaComment * Comment = new SemaComment(AST);

	FLY_DEBUG_END("SemaBuilder", "CreateComment");
	return Comment;
}

SemaLocalVar * SemaBuilder::CreateLocalVar(ASTVar *AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateLocalVar");

	// Create LocalVar Symbol
	SemaLocalVar *LocalVar = new SemaLocalVar(AST);
	SemaBuilderModifiers *Builder = SemaBuilderModifiers::Build(AST->getModifiers());
	LocalVar->Constant = Builder->isConstant();

	// Assign Symbol to AST
	AST->Sema = LocalVar;

	FLY_DEBUG_END("SemaBuilder", "CreateLocalVar");
	return LocalVar;
}

SemaParam *SemaBuilder::CreateParam(fly::ASTVar *AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateParam");

	// Create LocalVar Symbol
	SemaParam *Param = new SemaParam(AST);

	// Assign Symbol to AST
	AST->Sema = Param;

	FLY_DEBUG_END("SemaBuilder", "CreateParam");
	return Param;
}

SemaMemberVar * SemaBuilder::CreateMemberVar(ASTVar *AST, SemaResult *Parent) {
	SemaMemberVar *Sema = new SemaMemberVar(AST, Parent);

	SemaBuilderModifiers *Builder = SemaBuilderModifiers::Build(AST->getModifiers());
	Sema->Constant = Builder->isConstant();

	return Sema;
}

SemaCall * SemaBuilder::CreateCall(ASTCall *AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateParam");

	// Create Call Symbol
	SemaCall *Call = new SemaCall(AST);

	// Assign Symbol to AST
	AST->Sema = Call;

	FLY_DEBUG_END("SemaBuilder", "CreateParam");
	return Call;
}

SemaBoolValue * SemaBuilder::CreateBoolValue(ASTBoolValue *AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateBoolValue");

	SemaBoolValue * V = new SemaBoolValue(AST->getValue());
	V->Type = SemaBuiltin::getBoolType();
	AST->Sema = V;

	FLY_DEBUG_END("SemaBuilder", "CreateBoolValue");
	return V;
}

SemaValue * SemaBuilder::CreateNumberValue(ASTNumberValue *AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateNumberValue");

	SemaValue *V;

	// Floating point number
	llvm::Regex FloatRegex(R"(^[-+]?[0-9]*\.[0-9]+([eE][-+]?[0-9]+)?$)");
	if (FloatRegex.match(AST->getValue())) {
		// Floating point
		V = new SemaFloatValue(AST->getValue());
		V->Type = SemaBuiltin::getDoubleType();
	} else {

		// Integer number
		uint8_t Radix = 10;
		if (AST->getValue().substr(0, 2) == "0b" || AST->getValue().substr(0, 2) == "0B") {
			// Binary
			Radix = 2;
		} else if (AST->getValue().substr(0, 2) == "0x" || AST->getValue().substr(0, 2) == "0X") {
			// Hexadecimal
			Radix = 16;
		} else if (AST->getValue()[0] == '0' && AST->getValue().size() > 1) {
			// Octal
			Radix = 8;
		}
		SemaIntValue *IntValue = new SemaIntValue(AST->getValue(), Radix);
		llvm::APInt I = IntValue->getValue();
		if (I.isNegative()) {
			unsigned MinBits = 1 + I.getBitWidth() - I.countLeadingOnes();
			if (MinBits <= 16) IntValue->Type = SemaBuiltin::getShortType();
			else if (MinBits <= 32) IntValue->Type = SemaBuiltin::getIntType();
			else if (MinBits <= 64) IntValue->Type = SemaBuiltin::getLongType();
		} else {
			unsigned MinBits = 1 + I.getBitWidth() - I.countLeadingZeros();
			if (MinBits <= 8) IntValue->Type = SemaBuiltin::getByteType();
			else if (MinBits <= 16) IntValue->Type = SemaBuiltin::getUShortType();
			else if (MinBits <= 32) IntValue->Type = SemaBuiltin::getUIntType();
			else if (MinBits <= 64) IntValue->Type = SemaBuiltin::getULongType();
		}
		IntValue->Type = SemaBuiltin::getIntType();
		V = IntValue;
	}

	AST->Sema = V;

	FLY_DEBUG_END("SemaBuilder", "CreateNumberValue");
	return V;
}

SemaStringValue * SemaBuilder::CreateStringValue(ASTStringValue *AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateStringValue");

	SemaStringValue * V = new SemaStringValue(AST->getValue());
	V->Type = SemaBuiltin::getStringType();
	AST->Sema = V;

	FLY_DEBUG_END("SemaBuilder", "CreateStringValue");
	return V;
}

SemaArrayValue * SemaBuilder::CreateArrayValue(ASTArrayValue *AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateArrayValue");

	SemaArrayValue * V = new SemaArrayValue();
	V->Type = AST->getValues().empty() ? nullptr : AST->getValues()[0]->getSema()->getType();
	AST->Sema = V;

	FLY_DEBUG_END("SemaBuilder", "CreateArrayValue");
	return V;
}

SemaStructValue * SemaBuilder::CreateStructValue(ASTStructValue *AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateStructValue");

	const llvm::StringMap<SemaValue *> Values;
	SemaStructValue * V = new SemaStructValue();
	if (AST->getValues().empty()) {
		V->Type = nullptr;
	} else {
		// TODO
		for (auto &Entry : AST->getValues()) {

		}
		// V->Type = S.getSemaBuilder().CreateClass();
	}
	AST->Sema = V;

	FLY_DEBUG_END("SemaBuilder", "CreateStructValue");
	return V;
}

