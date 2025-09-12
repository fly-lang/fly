//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaBuilder.cpp - The Symbolic Table Builder
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/Sema.h"
#include "Sema/SymTable.h"
#include "Sema/SemaModule.h"
#include "Sema/SemaNameSpace.h"
#include "Sema/SemaBuilder.h"
#include "Basic/Debug.h"
#include "Basic/Diagnostic.h"
#include "Sema/SemaClassType.h"
#include "Sema/SemaEnumType.h"
#include "Sema/SemaFunction.h"
#include "Sema/SemaGlobalVar.h"
#include "Sema/SemaType.h"
#include "Sema/SemaMemberVar.h"
#include "AST/ASTAlias.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
#include "AST/ASTImport.h"
#include "AST/ASTFunction.h"
#include "AST/ASTVar.h"
#include "AST/ASTModule.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTModifier.h"
#include "Sema/SemaClassAttribute.h"
#include "Sema/SemaClassMethod.h"
#include "Sema/SemaEnumEntry.h"
#include "Sema/SemaComment.h"
#include "Sema/SemaVisibilityKind.h"
#include "Sema/SemaValue.h"
#include "Sema/SemaCall.h"
#include "llvm/Support/Regex.h"

#include <Sema/SemaBuilderModifiers.h>
#include <Sema/SemaClassInstance.h>

using namespace fly;

SemaBuilder::SemaBuilder(Sema &S) : S(S) {

}

void SemaBuilder::CreateTable() {
	S.Table = new SymTable();

	// Create Builtin Types
	S.Table->BoolType = S.getSemaBuilder().CreateType(SemaTypeKind::TYPE_BOOL, "bool");
	S.Table->BoolType->DefaultValue = new SemaBoolValue(false);
	S.Table->BoolType->DefaultValue->Type = S.Table->BoolType;

	S.Table->ByteType = S.getSemaBuilder().CreateIntType(SemaIntTypeKind::TYPE_BYTE, "byte");
	S.Table->ByteType->DefaultValue = new SemaIntValue("0", 10);
	S.Table->ByteType->DefaultValue->Type = S.Table->ByteType;

	S.Table->UShortType = S.getSemaBuilder().CreateIntType(SemaIntTypeKind::TYPE_USHORT, "ushort");
	S.Table->UShortType->DefaultValue = new SemaIntValue("0", 10);
	S.Table->UShortType->DefaultValue->Type = S.Table->UShortType;

	S.Table->ShortType = S.getSemaBuilder().CreateIntType(SemaIntTypeKind::TYPE_SHORT, "short");
	S.Table->ShortType->DefaultValue = new SemaIntValue("0", 10);
	S.Table->ShortType->DefaultValue->Type = S.Table->ShortType;

	S.Table->UIntType = S.getSemaBuilder().CreateIntType(SemaIntTypeKind::TYPE_UINT, "uint");
	S.Table->UIntType->DefaultValue = new SemaIntValue("0", 10);
	S.Table->UIntType->DefaultValue->Type = S.Table->UIntType;

	S.Table->IntType = S.getSemaBuilder().CreateIntType(SemaIntTypeKind::TYPE_INT, "int");
	S.Table->IntType->DefaultValue = new SemaIntValue("0", 10);
	S.Table->IntType->DefaultValue->Type = S.Table->IntType;

	S.Table->ULongType = S.getSemaBuilder().CreateIntType(SemaIntTypeKind::TYPE_ULONG, "ulong");
	S.Table->ULongType->DefaultValue = new SemaIntValue("0", 10);
	S.Table->ULongType->DefaultValue->Type = S.Table->ULongType;

	S.Table->LongType = S.getSemaBuilder().CreateIntType(SemaIntTypeKind::TYPE_LONG, "long");
	S.Table->LongType->DefaultValue = new SemaIntValue("0", 10);
	S.Table->LongType->DefaultValue->Type = S.Table->LongType;

	S.Table->FloatType = S.getSemaBuilder().CreateFPType(SemaFloatTypeKind::TYPE_FLOAT, "float");
	S.Table->FloatType->DefaultValue = new SemaFloatValue("0.0");
	S.Table->FloatType->DefaultValue->Type = S.Table->FloatType;

	S.Table->DoubleType = S.getSemaBuilder().CreateFPType(SemaFloatTypeKind::TYPE_DOUBLE, "double");
	S.Table->DoubleType->DefaultValue = new SemaFloatValue("0.0");
	S.Table->DoubleType->DefaultValue->Type = S.Table->DoubleType;

	S.Table->StringType = S.getSemaBuilder().CreateType(SemaTypeKind::TYPE_STRING, "string");
	S.Table->StringType->DefaultValue = new SemaStringValue("");
	S.Table->StringType->DefaultValue->Type = S.Table->StringType;

	S.Table->VoidType = S.getSemaBuilder().CreateType(SemaTypeKind::TYPE_VOID, "void");
	
	S.Table->ErrorType = S.getSemaBuilder().CreateType(SemaTypeKind::TYPE_ERROR, "error");

	// Create the Default NameSpace
	S.Table->DefaultNameSpace = S.SBuilder->CreateDefaultNameSpace();

	// Add built-in types to the Default NameSpace
	S.Table->DefaultNameSpace->Types.insert(std::make_pair<>(S.Table->BoolType->getName(), S.Table->BoolType));
	S.Table->DefaultNameSpace->Types.insert(std::make_pair<>(S.Table->ByteType->getName(), S.Table->ByteType));
	S.Table->DefaultNameSpace->Types.insert(std::make_pair<>(S.Table->UShortType->getName(), S.Table->UShortType));
	S.Table->DefaultNameSpace->Types.insert(std::make_pair<>(S.Table->ShortType->getName(), S.Table->ShortType));
	S.Table->DefaultNameSpace->Types.insert(std::make_pair<>(S.Table->UIntType->getName(), S.Table->UIntType));
	S.Table->DefaultNameSpace->Types.insert(std::make_pair<>(S.Table->IntType->getName(), S.Table->IntType));
	S.Table->DefaultNameSpace->Types.insert(std::make_pair<>(S.Table->ULongType->getName(), S.Table->ULongType));
	S.Table->DefaultNameSpace->Types.insert(std::make_pair<>(S.Table->LongType->getName(), S.Table->LongType));
	S.Table->DefaultNameSpace->Types.insert(std::make_pair<>(S.Table->FloatType->getName(), S.Table->FloatType));
	S.Table->DefaultNameSpace->Types.insert(std::make_pair<>(S.Table->DoubleType->getName(), S.Table->DoubleType));
	S.Table->DefaultNameSpace->Types.insert(std::make_pair<>(S.Table->VoidType->getName(), S.Table->VoidType));
	S.Table->DefaultNameSpace->Types.insert(std::make_pair<>(S.Table->StringType->getName(), S.Table->StringType));
	S.Table->DefaultNameSpace->Types.insert(std::make_pair<>(S.Table->ErrorType->getName(), S.Table->ErrorType));
}

SemaNameSpace *SemaBuilder::CreateDefaultNameSpace() {
	FLY_DEBUG_START("SemaBuilder", "CreateNameSpace");

	SemaNameSpace *NameSpace = new SemaNameSpace(Sema::DEFAULT_NAMESPACE);
	S.Table->NameSpaces.insert(std::make_pair<>(NameSpace->getName(), NameSpace));
	S.Table->DefaultNameSpace = NameSpace;

	FLY_DEBUG_END("SemaBuilder", "CreateNameSpace");
	return NameSpace;
}

SemaNameSpace * SemaBuilder::CreateOrGetNameSpace(ASTNameSpace *AST) {
	// When the AST is null, return the Default NameSpace
	if (AST == nullptr) {
		return S.getSymTable().DefaultNameSpace;
	}

	// Build the NameSpace
	SemaNameSpace *Parent = nullptr;
	SemaNameSpace *NameSpace = nullptr;
	std::string FullName = "";
	for (auto It = AST->getNames().begin(); It != AST->getNames().end(); ++It) {
		// Generate the full name
		FullName += It == AST->getNames().begin() ? std::string(*It) : "." + FullName;

		// Create the NameSpace if not exists yet in the Context
		NameSpace = S.getSymTable().getNameSpaces().lookup(FullName);
		if (NameSpace == nullptr) {
			NameSpace = new SemaNameSpace(FullName);
			S.Table->NameSpaces.insert(std::make_pair<>(NameSpace->getName(), NameSpace));
			NameSpace->Parent = Parent;
			Parent = NameSpace;
		}
	}

	return NameSpace;
}

SemaModule * SemaBuilder::CreateModule(SemaNameSpace *NameSpace, ASTModule *AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateModule");

	SemaModule *Module = new SemaModule(AST);
	Module->NameSpace = NameSpace;
	S.Table->Modules.insert(std::make_pair(AST->getId(), Module));

	FLY_DEBUG_END("SemaBuilder", "CreateModule");
	return Module;
}

void SemaBuilder::CreateImport(SemaModule *Module, ASTImport *AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateImport");

	// Error: Empty Import
	if (AST->getName().empty()) {
		S.Diag(AST->getLocation(), diag::err_sema_import_undefined);
		return;
	}

	// Error: name is equals to the current ASTModule namespace
	if (AST->getName() == Module->getNameSpace()->getName()) {
		S.Diag(AST->getLocation(), diag::err_import_conflict_namespace) << AST->getName();
		return;
	}

	// Replace with alias name if exists
	llvm::StringRef Name = AST->getName();

	// Error: alias is equals to the current ASTModule namespace
	if (AST->getAlias()) {

		// Set Import Name
		Name = AST->getAlias()->getName();

		// Check Alias
		if (Module->getImports().lookup(Name) != nullptr) {
			S.Diag(AST->getLocation(), diag::err_conflict_import_alias) << Name;
			return;
		}

		if (AST->getAlias()->getName() == Module->getNameSpace()->getName()) {
			S.Diag(AST->getAlias()->getLocation(), diag::err_alias_conflict_namespace) << AST->getAlias()->getName();
			return;
		}
	}

	// Search Namespace in Symbol Table
	SemaNameSpace *ImportNameSpace = S.getSymTable().getNameSpaces().lookup(AST->getName());
	if (!ImportNameSpace) {
		// Error: NameSpace not found
		S.Diag(AST->getLocation(), diag::err_namespace_notfound) << AST->getName();
		return;
	}

	// Add NameSpace to the Imports for next symbols resolution
	Module->Imports.insert(std::make_pair(Name, ImportNameSpace));

	FLY_DEBUG_END("SemaBuilder", "CreateImport");
}

// TODO: remove GlobalVar
// SemaGlobalVar * SemaBuilder::CreateGlobalVar(SemaModule *Module, ASTVar *AST) {
// 	FLY_DEBUG_START("SemaBuilder", "CreateGlobalVar");
//
// 	SemaGlobalVar *GlobalVar = new SemaGlobalVar(AST);
//
// 	// Check Duplicates in Module
// 	if (Module->GlobalVars.lookup(AST->getName()) != nullptr) {
// 		// Error
// 		S.Diag(AST->getLocation(), diag::err_syntax_error) << AST->getName();
// 		return GlobalVar;
// 	}
//
// 	GlobalVar->Module = Module;
// 	Module->GlobalVars.insert(std::make_pair(AST->getName(), GlobalVar));
// 	AST->Sym = GlobalVar;
//
// 	// Check and set GlobalVar Modifiers
// 	for (auto Scope : AST->getModifiers()) {
// 		if (Scope == nullptr) {
// 			// Error:
// 			S.Diag(AST->getLocation(), diag::err_sema_visibility_error) << AST->getName();
// 		}
// 		if (Scope->getModifierKind() == ASTScopeKind::SCOPE_VISIBILITY) {
// 			if (Scope->getVisibility() == ASTModifierKind::MOD_PUBLIC ||
// 				Scope->getVisibility() == ASTModifierKind::V_DEFAULT) {
//
// 				// Check Duplicates in NameSpace
// 				GlobalVar->Visibility = Scope->getVisibility() == ASTModifierKind::MOD_PUBLIC ?
//                     SemaVisibilityKind::PUBLIC : SemaVisibilityKind::DEFAULT;
// 				if (Module->NameSpace->GlobalVars.lookup(AST->getName()) != nullptr) {
// 					// Error
// 					S.Diag(AST->getLocation(), diag::err_syntax_error) << AST->getName();
// 				}
// 				Module->NameSpace->GlobalVars.insert(std::make_pair(AST->getName(), GlobalVar));
// 			} else if (Scope->getVisibility() == ASTModifierKind::MOD_PRIVATE) {
// 				GlobalVar->Visibility = SemaVisibilityKind::PRIVATE;
// 			} else {
// 				// Error
// 				S.Diag(AST->getLocation(), diag::err_sema_visibility_error) << AST->getName();
// 			}
// 		} else if (Scope->getModifierKind() == ASTScopeKind::SCOPE_CONSTANT) {
// 			GlobalVar->Constant = Scope->isConstant();
// 		}
// 	}
//
// 	FLY_DEBUG_END("SemaBuilder", "CreateGlobalVar");
// 	return GlobalVar;
// }

SemaFunction * SemaBuilder::CreateFunction(SemaModule *Module, ASTFunction *AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateFunction");

	SemaFunction *Function = new SemaFunction(AST);

	// Check Duplicates in Module
	std::string MangledName = Function->getMangledName();
	if (Module->Functions.lookup(MangledName) != nullptr) {
		// Error: function already exists
		S.Diag(AST->getLocation(), diag::err_syntax_error) << AST->getName();
		return Function;
	}

	Function->Module = Module;
	Module->Functions.insert(std::make_pair(MangledName, Function));
	AST->Sema = Function;

	// Check and set Function Modifiers
	for (auto Modifier : AST->getModifiers()) {
		if (Modifier == nullptr) {
			// Error:
			S.Diag(AST->getLocation(), diag::err_sema_visibility_error) << AST->getName();
		}
		if (Modifier->getModifierKind() == ASTModifierKind::MOD_PUBLIC) {
			Function->Visibility = SemaVisibilityKind::PUBLIC;

			// Check Duplicates in NameSpace
			if (Module->NameSpace->Functions.lookup(MangledName) != nullptr) {
				// Error: duplicated function
				S.Diag(AST->getLocation(), diag::err_syntax_error) << AST->getName();
			}
			Module->NameSpace->Functions.insert(std::make_pair(MangledName, Function));
		} else if (Modifier->getModifierKind() == ASTModifierKind::MOD_DEFAULT) {
			Function->Visibility = SemaVisibilityKind::DEFAULT;

			// Check Duplicates in NameSpace
			if (Module->NameSpace->Functions.lookup(MangledName) != nullptr) {
				// Error: duplicated function
				S.Diag(AST->getLocation(), diag::err_syntax_error) << AST->getName();
			}
			Module->NameSpace->Functions.insert(std::make_pair(MangledName, Function));
		} else if (Modifier->getModifierKind() == ASTModifierKind::MOD_PRIVATE) {
			Function->Visibility = SemaVisibilityKind::PRIVATE;
		} else {
			// Error
			S.Diag(AST->getLocation(), diag::err_sema_visibility_error) << AST->getName();
		}
	}

	FLY_DEBUG_END("SemaBuilder", "CreateFunction");
	return Function;
}

SemaClassType * SemaBuilder::CreateClass(SemaModule *Module, ASTClass *AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateClass");

	// Create the Class Type
	SemaClassType *Class = new SemaClassType(AST);

	// Check Duplicates in Module
	if (Module->Types.lookup(AST->getName()) != nullptr) {
		// Error
		S.Diag(AST->getLocation(), diag::err_syntax_error) << AST->getName();
		return Class;
	}

	// Set Class Module
	Class->Module = Module;
	Module->Types.insert(std::make_pair(AST->getName(), Class));

	// Create the 'this' attribute for the current class
	Class->This = CreateThisInstance(Class);

	// Set Modifiers
	SemaBuilderModifiers *BuilderModifiers = SemaBuilderModifiers::Build(AST->getModifiers());
	Class->Constant = BuilderModifiers->isConstant();
	Class->Visibility = BuilderModifiers->getVisibility();

	// Check and set Function Modifiers
	if (Class->getVisibility() == SemaVisibilityKind::PUBLIC) {

		// Check Duplicates in NameSpace
		if (Module->NameSpace->Types.lookup(AST->getName()) != nullptr) {
			// Error
			S.Diag(AST->getLocation(), diag::err_syntax_error) << AST->getName();
		}
		Module->NameSpace->Types.insert(std::make_pair(AST->getName(), Class));
	} else if (Class->getVisibility() == SemaVisibilityKind::PROTECTED) {
		// TODO: Protected Visibility

	} else if (Class->getVisibility() == SemaVisibilityKind::PRIVATE) {
		// TODO: Private Visibility
	} else { // Default Visibility
		// Check Duplicates in NameSpace
		if (Module->NameSpace->Types.lookup(AST->getName()) != nullptr) {
			// Error
			S.Diag(AST->getLocation(), diag::err_syntax_error) << AST->getName();
		}
		Module->NameSpace->Types.insert(std::make_pair(AST->getName(), Class));
	}

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
		Method->ReturnType = S.getSymTable().getVoidType();
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

SemaEnumType * SemaBuilder::CreateEnum(SemaModule *Module, ASTEnum *AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateEnum");

	SemaEnumType *Enum = new SemaEnumType(AST);

	// Check Duplicates in Module
	if (Module->Types.lookup(AST->getName()) != nullptr) {
		// Error
		S.Diag(AST->getLocation(), diag::err_syntax_error) << AST->getName();
		return Enum;
	}

	Enum->Module = Module;
	Module->Types.insert(std::make_pair(AST->getName(), Enum));

	// Set Modifiers
	SemaBuilderModifiers *Builder = SemaBuilderModifiers::Build(AST->getModifiers());
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

SemaType * SemaBuilder::CreateType(SemaTypeKind Kind, std::string Name) {
	FLY_DEBUG_START("SemaBuilder", "CreateType");

	SemaType *Type = new SemaType(Kind, Name);

	FLY_DEBUG_END("SemaBuilder", "CreateType");
	return Type;
}

SemaIntType * SemaBuilder::CreateIntType(SemaIntTypeKind IntKind, std::string Name) {
	FLY_DEBUG_START("SemaBuilder", "CreateIntType");

	SemaIntType *Type = new SemaIntType(IntKind, Name);

	FLY_DEBUG_END("SemaBuilder", "CreateIntType");
	return Type;
}

SemaFloatType * SemaBuilder::CreateFPType(SemaFloatTypeKind FPKind, std::string Name) {
	FLY_DEBUG_START("SemaBuilder", "CreateFPType");

	SemaFloatType *Type = new SemaFloatType(FPKind, Name);

	FLY_DEBUG_END("SemaBuilder", "CreateFPType");
	return Type;
}

SemaArrayType * SemaBuilder::CreateArrayType(SemaType *Type) {
	FLY_DEBUG_START("SemaBuilder", "CreateArrayType");

	SemaArrayType * TypeArray = new SemaArrayType(Type);

	FLY_DEBUG_END("SemaBuilder", "CreateArrayType");
	return TypeArray;
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
	V->Type = S.getSymTable().BoolType;
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
		V->Type = S.getSymTable().DoubleType;
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
			if (MinBits <= 16) IntValue->Type = S.getSymTable().ShortType;
			else if (MinBits <= 32) IntValue->Type = S.getSymTable().IntType;
			else if (MinBits <= 64) IntValue->Type = S.getSymTable().LongType;
		} else {
			unsigned MinBits = 1 + I.getBitWidth() - I.countLeadingZeros();
			if (MinBits <= 8) IntValue->Type = S.getSymTable().ByteType;
			else if (MinBits <= 16) IntValue->Type = S.getSymTable().UShortType;
			else if (MinBits <= 32) IntValue->Type = S.getSymTable().UIntType;
			else if (MinBits <= 64) IntValue->Type = S.getSymTable().ULongType;
		}
		IntValue->Type = S.getSymTable().IntType;
		V = IntValue;
	}

	AST->Sema = V;

	FLY_DEBUG_END("SemaBuilder", "CreateNumberValue");
	return V;
}

SemaStringValue * SemaBuilder::CreateStringValue(ASTStringValue *AST) {
	FLY_DEBUG_START("SemaBuilder", "CreateStringValue");

	SemaStringValue * V = new SemaStringValue(AST->getValue());
	V->Type = S.getSymTable().StringType;
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

