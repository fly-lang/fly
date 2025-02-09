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
#include <AST/ASTScopes.h>
#include <Sema/SymBuilder.h>
#include <Sym/SymClassAttribute.h>
#include <Sym/SymClassMethod.h>
#include <Sym/SymEnumEntry.h>
#include <Sym/SymComment.h>
#include <Sym/SymVisibilityKind.h>

using namespace fly;

SymBuilder::SymBuilder(Sema &S) : S(S) {

}

SymTable * SymBuilder::CreateTable() {
	SymTable *Table = new SymTable();

	// Create the Default NameSpace
	S.SymBuildr->CreateNameSpace();

	// Create Builtin Types
	Table->BoolType = S.SymBuildr->CreateType(SymTypeKind::TYPE_BOOL);
	Table->ByteType = S.SymBuildr->CreateIntType(SymIntTypeKind::TYPE_INT);
	Table->UShortType = S.SymBuildr->CreateIntType(SymIntTypeKind::TYPE_USHORT);
	Table->ShortType = S.SymBuildr->CreateIntType(SymIntTypeKind::TYPE_SHORT);
	Table->UIntType = S.SymBuildr->CreateIntType(SymIntTypeKind::TYPE_UINT);
	Table->IntType = S.SymBuildr->CreateIntType(SymIntTypeKind::TYPE_INT);
	Table->ULongType = S.SymBuildr->CreateIntType(SymIntTypeKind::TYPE_ULONG);
	Table->LongType = S.SymBuildr->CreateIntType(SymIntTypeKind::TYPE_LONG);
	Table->FloatType = S.SymBuildr->CreateFPType(SymFPTypeKind::TYPE_FLOAT);
	Table->DoubleType = S.SymBuildr->CreateFPType(SymFPTypeKind::TYPE_DOUBLE);
	Table->VoidType = S.SymBuildr->CreateType(SymTypeKind::TYPE_VOID);
	Table->StringType = S.SymBuildr->CreateType(SymTypeKind::TYPE_STRING);
	Table->CharType = S.SymBuildr->CreateType(SymTypeKind::TYPE_CHAR);
	Table->ErrorType = S.SymBuildr->CreateType(SymTypeKind::TYPE_ERROR);

	// Add built-in types to the Default NameSpace
	Table->DefaultNameSpace->Types.insert(std::make_pair<>(Table->BoolType->getName(), Table->BoolType));
	Table->DefaultNameSpace->Types.insert(std::make_pair<>(Table->ByteType->getName(), Table->ByteType));
	Table->DefaultNameSpace->Types.insert(std::make_pair<>(Table->UShortType->getName(), Table->UShortType));
	Table->DefaultNameSpace->Types.insert(std::make_pair<>(Table->ShortType->getName(), Table->ShortType));
	Table->DefaultNameSpace->Types.insert(std::make_pair<>(Table->UIntType->getName(), Table->UIntType));
	Table->DefaultNameSpace->Types.insert(std::make_pair<>(Table->IntType->getName(), Table->IntType));
	Table->DefaultNameSpace->Types.insert(std::make_pair<>(Table->ULongType->getName(), Table->ULongType));
	Table->DefaultNameSpace->Types.insert(std::make_pair<>(Table->LongType->getName(), Table->LongType));
	Table->DefaultNameSpace->Types.insert(std::make_pair<>(Table->FloatType->getName(), Table->FloatType));
	Table->DefaultNameSpace->Types.insert(std::make_pair<>(Table->DoubleType->getName(), Table->DoubleType));
	Table->DefaultNameSpace->Types.insert(std::make_pair<>(Table->VoidType->getName(), Table->VoidType));
	Table->DefaultNameSpace->Types.insert(std::make_pair<>(Table->StringType->getName(), Table->StringType));
	Table->DefaultNameSpace->Types.insert(std::make_pair<>(Table->CharType->getName(), Table->CharType));
	Table->DefaultNameSpace->Types.insert(std::make_pair<>(Table->ErrorType->getName(), Table->ErrorType));

	return Table;
}

SymNameSpace *SymBuilder::CreateNameSpace() {
	FLY_DEBUG_START("SymBuilder", "CreateNameSpace");

	SymNameSpace *NameSpace = new SymNameSpace();
	S.Table->NameSpaces.insert(std::make_pair<>(NameSpace->getName(), NameSpace));
	S.Table->DefaultNameSpace = NameSpace;

	FLY_DEBUG_END("SymBuilder", "CreateNameSpace");
	return NameSpace;
}

SymNameSpace *SymBuilder::CreateNameSpace(llvm::StringRef Name) {
	FLY_DEBUG_MESSAGE("SemaBuilder", "CreateNameSpace", "Name=" << Name);

	SymNameSpace *NameSpace = new SymNameSpace(Name);
	S.Table->NameSpaces.insert(std::make_pair<>(NameSpace->getName(), NameSpace));

	FLY_DEBUG_END("SymBuilder", "CreateNameSpace");
	return NameSpace;
}

SymNameSpace * SymBuilder::AddNameSpace(llvm::StringRef Name) {
	// Create the NameSpace if not exists yet in the Context
	SymNameSpace *NameSpace = S.getSymTable().getNameSpaces().lookup(Name);
	if (NameSpace == nullptr) {
		S.getSymBuilder().CreateNameSpace(Name);
	}
	return NameSpace;
}

SymModule * SymBuilder::CreateModule(ASTModule *AST) {
	FLY_DEBUG_START("SymBuilder", "CreateModule");

	SymModule *Module = new SymModule(AST);
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

SymGlobalVar * SymBuilder::CreateGlobalVar(SymModule *Module, ASTVar *AST) {
	FLY_DEBUG_START("SymBuilder", "CreateGlobalVar");

	SymGlobalVar *GlobalVar = new SymGlobalVar(AST);

	// Check Duplicates in Module
	if (Module->GlobalVars.lookup(AST->getName()) != nullptr) {
		// Error
		S.Diag(AST->getLocation(), diag::err_syntax_error) << AST->getName();
		return GlobalVar;
	}

	Module->GlobalVars.insert(std::make_pair(AST->getName(), GlobalVar));
	AST->Def = GlobalVar;

	// Check and set GlobalVar Scopes
	for (auto Scope : AST->getScopes()) {
		if (Scope == nullptr) {
			// Error:
			S.Diag(AST->getLocation(), diag::err_sema_visibility_error) << AST->getName();
		}
		if (Scope->getKind() == ASTScopeKind::SCOPE_VISIBILITY) {
			if (Scope->getVisibility() == ASTVisibilityKind::V_PUBLIC) {

				// Check Duplicates in NameSpace
				GlobalVar->Visibility = SymVisibilityKind::PUBLIC;
				if (Module->NameSpace->GlobalVars.lookup(AST->getName()) != nullptr) {
					// Error
					S.Diag(AST->getLocation(), diag::err_syntax_error) << AST->getName();
				}
				Module->NameSpace->GlobalVars.insert(std::make_pair(AST->getName(), GlobalVar));
			} else if (Scope->getVisibility() == ASTVisibilityKind::V_PRIVATE) {
				GlobalVar->Visibility = SymVisibilityKind::PRIVATE;
			} else {
				// Error
				S.Diag(AST->getLocation(), diag::err_sema_visibility_error) << AST->getName();
			}
		} else if (Scope->getKind() == ASTScopeKind::SCOPE_CONSTANT) {
			GlobalVar->Constant = Scope->isConstant();
		}
	}

	FLY_DEBUG_END("SymBuilder", "CreateGlobalVar");
	return GlobalVar;
}

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

	Module->Functions.insert(std::make_pair(MangledName, Function));
	AST->Def = Function;

	// Check and set Function Scopes
	for (auto Scope : AST->getScopes()) {
		if (Scope == nullptr) {
			// Error:
			S.Diag(AST->getLocation(), diag::err_sema_visibility_error) << AST->getName();
		}
		if (Scope->getKind() == ASTScopeKind::SCOPE_VISIBILITY) {
			if (Scope->getVisibility() == ASTVisibilityKind::V_PUBLIC) {
				Function->Visibility = SymVisibilityKind::PUBLIC;

				// Check Duplicates in NameSpace
				if (Module->NameSpace->Functions.lookup(MangledName) != nullptr) {
					// Error
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
	if (Module->Classes.lookup(AST->getName()) != nullptr) {
		// Error
		S.Diag(AST->getLocation(), diag::err_syntax_error) << AST->getName();
		return Class;
	}
	Module->Classes.insert(std::make_pair(AST->getName(), Class));

	// Check and set Function Scopes
	for (auto Scope : AST->getScopes()) {
		if (Scope == nullptr) {
			// Error:
			S.Diag(AST->getLocation(), diag::err_sema_visibility_error) << AST->getName();
		}
		if (Scope->getKind() == ASTScopeKind::SCOPE_VISIBILITY) {
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
		} else if (Scope->getKind() == ASTScopeKind::SCOPE_CONSTANT) {
			Class->Constant = Scope->isConstant();
		}
	}

	FLY_DEBUG_END("SymBuilder", "CreateClass");
	return Class;
}

SymClassAttribute * SymBuilder::CreateClassAttribute(SymClass *Class, ASTVar *AST, SymComment *Comment) {
	FLY_DEBUG_START("SymBuilder", "CreateClassAttribute");

	SymClassAttribute *Attribute = new SymClassAttribute(AST);

	Class->Attributes.insert(std::make_pair(AST->getName(), Attribute));
	AST->Def = Attribute;
	Attribute->Comment = Comment;

	FLY_DEBUG_END("SymBuilder", "CreateClassAttribute");
	return Attribute;
}

SymClassMethod * SymBuilder::CreateClassFunction(SymClass *Class, ASTFunction *AST, SymComment *Comment) {
	FLY_DEBUG_START("SymBuilder", "CreateClassFunction");

	SymClassMethod *Method = new SymClassMethod(AST);
	//Class->Methods.insert(std::make_pair(AST->getName(), Method)); // FIXME

	// Set Constructor Class
	// Constructor->Class = &Class;// Remove Default Constructor
	// if (Class.DefaultConstructor == nullptr) {
	// 	delete Class.DefaultConstructor;
	// 	Class.DefaultConstructor = nullptr;
	// 	Class.Constructors.clear();
	// }

	AST->Def = Method;
	Method->Comment = Comment;

	FLY_DEBUG_END("SymBuilder", "CreateClassFunction");
	return Method;
}

SymEnum * SymBuilder::CreateEnum(SymModule *Module, ASTEnum *AST) {
	FLY_DEBUG_START("SymBuilder", "CreateEnum");

	SymEnum *Enum = new SymEnum(AST);

	// Check Duplicates in Module
	if (Module->Enums.lookup(AST->getName()) != nullptr) {
		// Error
		S.Diag(AST->getLocation(), diag::err_syntax_error) << AST->getName();
		return Enum;
	}
	Module->Enums.insert(std::make_pair(AST->getName(), Enum));

	// Check and set Function Scopes
	for (auto Scope : AST->getScopes()) {
		if (Scope == nullptr) {
			// Error:
			S.Diag(AST->getLocation(), diag::err_sema_visibility_error) << AST->getName();
		}
		if (Scope->getKind() == ASTScopeKind::SCOPE_VISIBILITY) {
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
		} else if (Scope->getKind() == ASTScopeKind::SCOPE_CONSTANT) {
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

SymType * SymBuilder::CreateType(SymTypeKind Kind) {
	FLY_DEBUG_START("SymBuilder", "CreateType");

	SymType *Type = new SymType(Kind);

	FLY_DEBUG_END("SymBuilder", "CreateType");
	return Type;
}

SymTypeInt * SymBuilder::CreateIntType(SymIntTypeKind IntKind) {
	FLY_DEBUG_START("SymBuilder", "CreateIntType");

	SymTypeInt *Type = new SymTypeInt(IntKind);

	FLY_DEBUG_END("SymBuilder", "CreateIntType");
	return Type;
}

SymTypeFP * SymBuilder::CreateFPType(SymFPTypeKind FPKind) {
	FLY_DEBUG_START("SymBuilder", "CreateFPType");

	SymTypeFP *Type = new SymTypeFP(FPKind);

	FLY_DEBUG_END("SymBuilder", "CreateFPType");
	return Type;
}

SymTypeArray * SymBuilder::CreateArrayType(SymType *Type, uint64_t Size) {
	FLY_DEBUG_START("SymBuilder", "CreateArrayType");

	SymTypeArray * TypeArray = new SymTypeArray(Type, Size);

	FLY_DEBUG_END("SymBuilder", "CreateArrayType");
	return TypeArray;
}

SymComment * SymBuilder::CreateComment(ASTComment *AST) {
	FLY_DEBUG_START("SymBuilder", "CreateComment");

	SymComment * Comment = new SymComment(AST);

	FLY_DEBUG_END("SymBuilder", "CreateComment");
	return Comment;
}
