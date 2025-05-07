//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymBuilder.cpp - The Symbolic Table Builder
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/Sema.h"
#include "Sym/SymTable.h"
#include "Sym/SymModule.h"
#include "Sym/SymNameSpace.h"
#include "Sema/SymBuilder.h"
#include "Basic/Debug.h"
#include "Basic/Diagnostic.h"
#include "Sym/SymClass.h"
#include "Sym/SymEnum.h"
#include "Sym/SymFunction.h"
#include "Sym/SymGlobalVar.h"
#include "Sym/SymType.h"

#include <AST/ASTAlias.h>
#include <AST/ASTClass.h>
#include <AST/ASTEnum.h>
#include <AST/ASTImport.h>
#include <AST/ASTFunction.h>
#include <AST/ASTVar.h>
#include <AST/ASTModule.h>
#include <AST/ASTNameSpace.h>
#include <AST/ASTScopes.h>
#include <Sym/SymClassAttribute.h>
#include <Sym/SymClassMethod.h>
#include <Sym/SymEnumEntry.h>
#include <Sym/SymComment.h>
#include <Sym/SymVisibilityKind.h>
#include <llvm/Support/Regex.h>

using namespace fly;

SymBuilder::SymBuilder(Sema &S) : S(S) {

}

void SymBuilder::CreateTable() {
	S.Table = new SymTable();

	// Create Builtin Types
	S.Table->BoolType = S.getSymBuilder().CreateType(SymTypeKind::TYPE_BOOL, "bool");
	S.Table->ByteType = S.getSymBuilder().CreateIntType(SymIntTypeKind::TYPE_BYTE, "byte");
	S.Table->UShortType = S.getSymBuilder().CreateIntType(SymIntTypeKind::TYPE_USHORT, "ushort");
	S.Table->ShortType = S.getSymBuilder().CreateIntType(SymIntTypeKind::TYPE_SHORT, "short");
	S.Table->UIntType = S.getSymBuilder().CreateIntType(SymIntTypeKind::TYPE_UINT, "uint");
	S.Table->IntType = S.getSymBuilder().CreateIntType(SymIntTypeKind::TYPE_INT, "int");
	S.Table->ULongType = S.getSymBuilder().CreateIntType(SymIntTypeKind::TYPE_ULONG, "ulong");
	S.Table->LongType = S.getSymBuilder().CreateIntType(SymIntTypeKind::TYPE_LONG, "long");
	S.Table->FloatType = S.getSymBuilder().CreateFPType(SymFPTypeKind::TYPE_FLOAT, "float");
	S.Table->DoubleType = S.getSymBuilder().CreateFPType(SymFPTypeKind::TYPE_DOUBLE, "double");
	S.Table->VoidType = S.getSymBuilder().CreateType(SymTypeKind::TYPE_VOID, "void");
	S.Table->StringType = S.getSymBuilder().CreateType(SymTypeKind::TYPE_STRING, "string");
	S.Table->ErrorType = S.getSymBuilder().CreateType(SymTypeKind::TYPE_ERROR, "error");

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

SymNameSpace *SymBuilder::CreateDefaultNameSpace() {
	FLY_DEBUG_START("SymBuilder", "CreateNameSpace");

	SymNameSpace *NameSpace = new SymNameSpace(Sema::DEFAULT_NAMESPACE);
	S.Table->NameSpaces.insert(std::make_pair<>(NameSpace->getName(), NameSpace));
	S.Table->DefaultNameSpace = NameSpace;

	FLY_DEBUG_END("SymBuilder", "CreateNameSpace");
	return NameSpace;
}

SymNameSpace * SymBuilder::CreateOrGetNameSpace(ASTNameSpace *AST) {
	// When the AST is null, return the Default NameSpace
	if (AST == nullptr) {
		return S.getSymTable().DefaultNameSpace;
	}

	// Build the NameSpace
	SymNameSpace *Parent = nullptr;
	SymNameSpace *NameSpace = nullptr;
	std::string FullName = "";
	for (auto It = AST->getNames().begin(); It != AST->getNames().end(); ++It) {
		// Generate the full name
		FullName += It == AST->getNames().begin() ? std::string(*It) : "." + FullName;

		// Create the NameSpace if not exists yet in the Context
		NameSpace = S.getSymTable().getNameSpaces().lookup(FullName);
		if (NameSpace == nullptr) {
			NameSpace = new SymNameSpace(FullName);
			S.Table->NameSpaces.insert(std::make_pair<>(NameSpace->getName(), NameSpace));
			NameSpace->Parent = Parent;
			Parent = NameSpace;
		}
	}

	return NameSpace;
}

SymModule * SymBuilder::CreateModule(SymNameSpace *NameSpace, ASTModule *AST) {
	FLY_DEBUG_START("SymBuilder", "CreateModule");

	SymModule *Module = new SymModule(AST);
	Module->NameSpace = NameSpace;
	S.Table->Modules.insert(std::make_pair(AST->getId(), Module));

	FLY_DEBUG_END("SymBuilder", "CreateModule");
	return Module;
}

void SymBuilder::CreateImport(SymModule *Module, ASTImport *AST) {
	FLY_DEBUG_START("SymBuilder", "CreateImport");

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
	SymNameSpace *ImportNameSpace = S.getSymTable().getNameSpaces().lookup(AST->getName());
	if (!ImportNameSpace) {
		// Error: NameSpace not found
		S.Diag(AST->getLocation(), diag::err_namespace_notfound) << AST->getName();
		return;
	}

	// Add NameSpace to the Imports for next symbols resolution
	Module->Imports.insert(std::make_pair(Name, ImportNameSpace));

	FLY_DEBUG_END("SymBuilder", "CreateImport");
}

// TODO: remove GlobalVar
// SymGlobalVar * SymBuilder::CreateGlobalVar(SymModule *Module, ASTVar *AST) {
// 	FLY_DEBUG_START("SymBuilder", "CreateGlobalVar");
//
// 	SymGlobalVar *GlobalVar = new SymGlobalVar(AST);
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
// 	// Check and set GlobalVar Scopes
// 	for (auto Scope : AST->getScopes()) {
// 		if (Scope == nullptr) {
// 			// Error:
// 			S.Diag(AST->getLocation(), diag::err_sema_visibility_error) << AST->getName();
// 		}
// 		if (Scope->getScopeKind() == ASTScopeKind::SCOPE_VISIBILITY) {
// 			if (Scope->getVisibility() == ASTVisibilityKind::V_PUBLIC ||
// 				Scope->getVisibility() == ASTVisibilityKind::V_DEFAULT) {
//
// 				// Check Duplicates in NameSpace
// 				GlobalVar->Visibility = Scope->getVisibility() == ASTVisibilityKind::V_PUBLIC ?
//                     SymVisibilityKind::PUBLIC : SymVisibilityKind::DEFAULT;
// 				if (Module->NameSpace->GlobalVars.lookup(AST->getName()) != nullptr) {
// 					// Error
// 					S.Diag(AST->getLocation(), diag::err_syntax_error) << AST->getName();
// 				}
// 				Module->NameSpace->GlobalVars.insert(std::make_pair(AST->getName(), GlobalVar));
// 			} else if (Scope->getVisibility() == ASTVisibilityKind::V_PRIVATE) {
// 				GlobalVar->Visibility = SymVisibilityKind::PRIVATE;
// 			} else {
// 				// Error
// 				S.Diag(AST->getLocation(), diag::err_sema_visibility_error) << AST->getName();
// 			}
// 		} else if (Scope->getScopeKind() == ASTScopeKind::SCOPE_CONSTANT) {
// 			GlobalVar->Constant = Scope->isConstant();
// 		}
// 	}
//
// 	FLY_DEBUG_END("SymBuilder", "CreateGlobalVar");
// 	return GlobalVar;
// }

SymFunction * SymBuilder::CreateFunction(SymModule *Module, ASTFunction *AST) {
	FLY_DEBUG_START("SymBuilder", "CreateFunction");

	SymFunction *Function = new SymFunction(AST);

	// Check Duplicates in Module
	std::string MangledName = Function->getMangledName();
	if (Module->Functions.lookup(MangledName) != nullptr) {
		// Error: function already exists
		S.Diag(AST->getLocation(), diag::err_syntax_error) << AST->getName();
		return Function;
	}

	Function->Module = Module;
	Module->Functions.insert(std::make_pair(MangledName, Function));
	AST->Sym = Function;

	// Check and set Function Scopes
	for (auto Scope : AST->getScopes()) {
		if (Scope == nullptr) {
			// Error:
			S.Diag(AST->getLocation(), diag::err_sema_visibility_error) << AST->getName();
		}
		if (Scope->getScopeKind() == ASTScopeKind::SCOPE_VISIBILITY) {
			if (Scope->getVisibility() == ASTVisibilityKind::V_PUBLIC) {
				Function->Visibility = SymVisibilityKind::PUBLIC;

				// Check Duplicates in NameSpace
				if (Module->NameSpace->Functions.lookup(MangledName) != nullptr) {
					// Error: duplicated function
					S.Diag(AST->getLocation(), diag::err_syntax_error) << AST->getName();
				}
				Module->NameSpace->Functions.insert(std::make_pair(MangledName, Function));
			} else if (Scope->getVisibility() == ASTVisibilityKind::V_DEFAULT) {
				Function->Visibility = SymVisibilityKind::DEFAULT;

				// Check Duplicates in NameSpace
				if (Module->NameSpace->Functions.lookup(MangledName) != nullptr) {
					// Error: duplicated function
					S.Diag(AST->getLocation(), diag::err_syntax_error) << AST->getName();
				}
				Module->NameSpace->Functions.insert(std::make_pair(MangledName, Function));
			} else if (Scope->getVisibility() == ASTVisibilityKind::V_PRIVATE) {
				Function->Visibility = SymVisibilityKind::PRIVATE;
			} else {
				// Error
				S.Diag(AST->getLocation(), diag::err_sema_visibility_error) << AST->getName();
			}
		}
	}

	FLY_DEBUG_END("SymBuilder", "CreateFunction");
	return Function;
}

SymClass * SymBuilder::CreateClass(SymModule *Module, ASTClass *AST) {
	FLY_DEBUG_START("SymBuilder", "CreateClass");

	SymClass *Class = new SymClass(AST);

	// Check Duplicates in Module
	if (Module->Types.lookup(AST->getName()) != nullptr) {
		// Error
		S.Diag(AST->getLocation(), diag::err_syntax_error) << AST->getName();
		return Class;
	}

	Class->Module = Module;
	Module->Types.insert(std::make_pair(AST->getName(), Class));

	// Check and set Function Scopes
	for (auto Scope : AST->getScopes()) {
		if (Scope == nullptr) {
			// Error:
			S.Diag(AST->getLocation(), diag::err_sema_visibility_error) << AST->getName();
		}
		if (Scope->getScopeKind() == ASTScopeKind::SCOPE_VISIBILITY) {
			if (Scope->getVisibility() == ASTVisibilityKind::V_PUBLIC) {
				Class->Visibility = SymVisibilityKind::PUBLIC;

				// Check Duplicates in NameSpace
				if (Module->NameSpace->Types.lookup(AST->getName()) != nullptr) {
					// Error
					S.Diag(AST->getLocation(), diag::err_syntax_error) << AST->getName();
				}
				Module->NameSpace->Types.insert(std::make_pair(AST->getName(), Class));
			} else if (Scope->getVisibility() == ASTVisibilityKind::V_PRIVATE) {
				Class->Visibility = SymVisibilityKind::PRIVATE;
			} else {
				// Error
				S.Diag(AST->getLocation(), diag::err_sema_visibility_error) << AST->getName();
			}
		} else if (Scope->getScopeKind() == ASTScopeKind::SCOPE_CONSTANT) {
			Class->Constant = Scope->isConstant();
		}
	}

	FLY_DEBUG_END("SymBuilder", "CreateClass");
	return Class;
}

SymClassAttribute * SymBuilder::CreateClassAttribute(SymClass *Class, ASTVar *AST, SymComment *Comment) {
	FLY_DEBUG_START("SymBuilder", "CreateClassAttribute");

	SymClassAttribute *Attribute = new SymClassAttribute(AST, Class);

	Class->Attributes.insert(std::make_pair(AST->getName(), Attribute));
	AST->Sym = Attribute;
	Attribute->Comment = Comment;

	FLY_DEBUG_END("SymBuilder", "CreateClassAttribute");
	return Attribute;
}

SymClassMethod * SymBuilder::CreateClassMethod(SymClass *Class, ASTFunction *AST, SymComment *Comment) {
	FLY_DEBUG_START("SymBuilder", "CreateClassFunction");

	SymClassMethod *Method = new SymClassMethod(AST, Class);
	std::string MangledName = Method->getMangledName();

	if (AST->getName() == Class->getName()) {
		Method->MethodKind = SymClassMethodKind::METHOD_CONSTRUCTOR;
		Class->Constructors.insert(std::make_pair(MangledName, Method));
	} else {
		Method->MethodKind = SymClassMethodKind::METHOD;
		Class->Methods.insert(std::make_pair(MangledName, Method));
	}

	AST->Sym = Method;
	Method->Comment = Comment;

	FLY_DEBUG_END("SymBuilder", "CreateClassFunction");
	return Method;
}

SymEnum * SymBuilder::CreateEnum(SymModule *Module, ASTEnum *AST) {
	FLY_DEBUG_START("SymBuilder", "CreateEnum");

	SymEnum *Enum = new SymEnum(AST);

	// Check Duplicates in Module
	if (Module->Types.lookup(AST->getName()) != nullptr) {
		// Error
		S.Diag(AST->getLocation(), diag::err_syntax_error) << AST->getName();
		return Enum;
	}

	Enum->Module = Module;
	Module->Types.insert(std::make_pair(AST->getName(), Enum));

	// Check and set Function Scopes
	for (auto Scope : AST->getScopes()) {
		if (Scope == nullptr) {
			// Error:
			S.Diag(AST->getLocation(), diag::err_sema_visibility_error) << AST->getName();
		}
		if (Scope->getScopeKind() == ASTScopeKind::SCOPE_VISIBILITY) {
			if (Scope->getVisibility() == ASTVisibilityKind::V_PUBLIC) {
				Enum->Visibility = SymVisibilityKind::PUBLIC;

				// Check Duplicates in NameSpace
				if (Module->NameSpace->Types.lookup(AST->getName()) != nullptr) {
					// Error
					S.Diag(AST->getLocation(), diag::err_syntax_error) << AST->getName();
				}
				Module->NameSpace->Types.insert(std::make_pair(AST->getName(), Enum));
			} else if (Scope->getVisibility() == ASTVisibilityKind::V_PRIVATE) {
				Enum->Visibility = SymVisibilityKind::PRIVATE;
			} else {
				// Error
				S.Diag(AST->getLocation(), diag::err_sema_visibility_error) << AST->getName();
			}
		} else if (Scope->getScopeKind() == ASTScopeKind::SCOPE_CONSTANT) {
			Enum->Constant = Scope->isConstant();
		}
	}

	FLY_DEBUG_END("SymBuilder", "CreateEnum");
	return Enum;
}

SymEnumEntry * SymBuilder::CreateEnumEntry(SymEnum *Enum, ASTVar *AST, SymComment *Comment) {
	FLY_DEBUG_START("SymBuilder", "CreateEnumEntry");

	SymEnumEntry *Entry = new SymEnumEntry(AST);
	// EnumEntry->Index = Enum.Entries.size() + 1; TODO
	Enum->Entries.insert(std::make_pair(AST->getName(), Entry));
	Entry->Comment = Comment;

	FLY_DEBUG_END("SymBuilder", "CreateEnumEntry");
	return Entry;
}

SymType * SymBuilder::CreateType(SymTypeKind Kind, std::string Name) {
	FLY_DEBUG_START("SymBuilder", "CreateType");

	SymType *Type = new SymType(Kind, Name);

	FLY_DEBUG_END("SymBuilder", "CreateType");
	return Type;
}

SymTypeInt * SymBuilder::CreateIntType(SymIntTypeKind IntKind, std::string Name) {
	FLY_DEBUG_START("SymBuilder", "CreateIntType");

	SymTypeInt *Type = new SymTypeInt(IntKind, Name);

	FLY_DEBUG_END("SymBuilder", "CreateIntType");
	return Type;
}

SymTypeFP * SymBuilder::CreateFPType(SymFPTypeKind FPKind, std::string Name) {
	FLY_DEBUG_START("SymBuilder", "CreateFPType");

	SymTypeFP *Type = new SymTypeFP(FPKind, Name);

	FLY_DEBUG_END("SymBuilder", "CreateFPType");
	return Type;
}

SymTypeArray * SymBuilder::CreateArrayType(SymType *Type) {
	FLY_DEBUG_START("SymBuilder", "CreateArrayType");

	SymTypeArray * TypeArray = new SymTypeArray(Type);

	FLY_DEBUG_END("SymBuilder", "CreateArrayType");
	return TypeArray;
}

SymComment * SymBuilder::CreateComment(ASTComment *AST) {
	FLY_DEBUG_START("SymBuilder", "CreateComment");

	SymComment * Comment = new SymComment(AST);

	FLY_DEBUG_END("SymBuilder", "CreateComment");
	return Comment;
}

SymLocalVar * SymBuilder::CreateLocalVar(ASTVar *AST) {
	FLY_DEBUG_START("SymBuilder", "CreateLocalVar");

	// Create LocalVar Symbol
	SymLocalVar *LocalVar = new SymLocalVar(AST);

	// Assign Symbol to AST
	AST->Sym = LocalVar;

	FLY_DEBUG_END("SymBuilder", "CreateLocalVar");
	return LocalVar;
}

SymParam *SymBuilder::CreateParam(fly::ASTVar *AST) {
	FLY_DEBUG_START("SymBuilder", "CreateParam");

	// Create LocalVar Symbol
	SymParam *Param = new SymParam(AST);

	// Assign Symbol to AST
	AST->Sym = Param;

	FLY_DEBUG_END("SymBuilder", "CreateParam");
	return Param;
}

SymCall * SymBuilder::CreateCall(ASTCall *AST) {
	FLY_DEBUG_START("SymBuilder", "CreateParam");

	// Create Call Symbol
	SymCall *Call = new SymCall(AST);

	// Assign Symbol to AST
	AST->Sym = Call;

	FLY_DEBUG_END("SymBuilder", "CreateParam");
	return Call;
}

SemaBoolValue * SymBuilder::CreateBoolValue(ASTBoolValue *AST) {
	FLY_DEBUG_START("SymBuilder", "CreateBoolValue");

	SemaBoolValue * V = new SemaBoolValue(AST->getValue());
	V->Type = S.getSymTable().BoolType;
	AST->Sema = V;

	FLY_DEBUG_END("SymBuilder", "CreateBoolValue");
	return V;
}

SemaValue * SymBuilder::CreateNumberValue(ASTNumberValue *AST) {
	FLY_DEBUG_START("SymBuilder", "CreateNumberValue");

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

	FLY_DEBUG_END("SymBuilder", "CreateNumberValue");
	return V;
}

SemaStringValue * SymBuilder::CreateStringValue(ASTStringValue *AST) {
	FLY_DEBUG_START("SymBuilder", "CreateStringValue");

	SemaStringValue * V = new SemaStringValue(AST->getValue());
	V->Type = S.getSymTable().StringType;
	AST->Sema = V;

	FLY_DEBUG_END("SymBuilder", "CreateStringValue");
	return V;
}

SemaArrayValue * SymBuilder::CreateArrayValue(ASTArrayValue *AST) {
	FLY_DEBUG_START("SymBuilder", "CreateArrayValue");

	SemaArrayValue * V = new SemaArrayValue();
	V->Type = AST->getValues().empty() ? nullptr : AST->getValues()[0]->getSema()->getType();
	AST->Sema = V;

	FLY_DEBUG_END("SymBuilder", "CreateArrayValue");
	return V;
}

SemaStructValue * SymBuilder::CreateStructValue(ASTStructValue *AST) {
	FLY_DEBUG_START("SymBuilder", "CreateStructValue");

	const llvm::StringMap<SemaValue *> Values;
	SemaStructValue * V = new SemaStructValue();
	if (AST->getValues().empty()) {
		V->Type = nullptr;
	} else {
		// TODO
		for (auto &Entry : AST->getValues()) {

		}
		// V->Type = S.getSymBuilder().CreateClass();
	}
	AST->Sema = V;

	FLY_DEBUG_END("SymBuilder", "CreateStructValue");
	return V;
}

