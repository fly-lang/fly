//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymbolTable.cpp - Sym table implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SymbolTable.h"
#include "Sema/SemaFunction.h"
#include "Sema/SemaParam.h"
#include "Sema/SemaType.h"

using namespace fly;

/**
 * Symbol table constructor
 */
SymbolTable::SymbolTable(SymbolTable *Parent) : Parent(Parent) {

}

SymbolTable::~SymbolTable() {
	for (auto &Entry : Table)
		for (Symbol *Sym : Entry.second)
			delete Sym;
	Table.clear();
}

bool SymbolTable::insert(Symbol *Sym) {
	// Check for existing symbols with the same name
	llvm::SmallVector<Symbol *, 8> *ExistingSymbols = lookup(Sym->Name);

	if (ExistingSymbols) {
		// For functions and methods, check for duplicates based on parameter types
		if (Sym->getKind() == SymbolKind::FUNCTION) {
			SemaFunctionBase *NewFunction = static_cast<SemaFunctionBase *>(Sym->getRef());
			llvm::SmallVector<SemaParam *, 8> &NewParams = NewFunction->getParams();

			// Check each existing symbol
			for (Symbol *ExistingSym : *ExistingSymbols) {
				// Only check symbols of the same kind
				if (ExistingSym->getKind() != Sym->getKind()) {
					continue;
				}

				SemaFunctionBase *ExistingFunction = static_cast<SemaFunctionBase *>(ExistingSym->getRef());
				llvm::SmallVector<SemaParam *, 8> &ExistingParams = ExistingFunction->getParams();

				// Check if the number of parameters matches
				if (NewParams.size() != ExistingParams.size()) {
					continue;
				}

				// Check if all parameter types match exactly
				bool AllTypesMatch = true;
				for (size_t i = 0; i < NewParams.size(); i++) {
					SemaType *NewParamType = NewParams[i]->getType();
					SemaType *ExistingParamType = ExistingParams[i]->getType();

					// Direct type match using isEquals method
					if (!NewParamType->isEquals(ExistingParamType)) {
						// For duplicate checking, we use exact type matching (not inheritance)
						// Two functions are duplicates only if parameter types are exactly the same
						AllTypesMatch = false;
						break;
					}
				}

				// If all types match exactly, it's a duplicate
				if (AllTypesMatch) {
					return false; // Duplicate found, insertion failed
				}
			}
		} else {
			// For non-function symbols, any symbol with the same name is a duplicate
			// (regardless of kind) to avoid ambiguity
			return false; // Duplicate found, insertion failed
		}
	}

	// No duplicate, insert the symbol
	Table[Sym->Name].push_back(Sym);
	return true; // Insertion successful
}

void SymbolTable::addChild(SymbolTable *Child) {
	Children.push_back(Child);
}

size_t SymbolTable::size() const {
	return Table.size();
}

const llvm::StringMap<llvm::SmallVector<Symbol *, 8>> &SymbolTable::getAll() const {
	return Table;
}

llvm::SmallVector<Symbol *, 8> *SymbolTable::lookup(llvm::StringRef Name) {
	auto it = Table.find(Name);
	if (it != Table.end())
		return &it->second;
	return nullptr;
}

llvm::SmallVector<Symbol *, 8> *SymbolTable::lookupInParents(llvm::StringRef Name) {
	// First check in the current table
	auto it = Table.find(Name);
	if (it != Table.end())
		return &it->second;

	if (Parent) return Parent->lookupInParents(Name);
	return nullptr;
}

llvm::SmallVector<Symbol *, 8> *SymbolTable::lookupInChildren(llvm::StringRef Name) {
	// First check in the current table
	auto it = Table.find(Name);
	if (it != Table.end())
		return &it->second;

	// Then recursively search in all children
	for (SymbolTable* Child : Children) {
		if (Child) {
			auto Found = Child->lookupInChildren(Name);
			if (Found) return Found;
		}
	}

	return nullptr;
}

SymbolTable * SymbolTable::pushScope() {
	auto* Child = new SymbolTable(this);
	Children.push_back(Child);
	return Child;
}

SymbolTable * SymbolTable::getParent() {
	return Parent;
}

llvm::SmallVector<SymbolTable*, 8> SymbolTable::getChildren() {
	return Children;
}

void SymbolTable::deleteChildren() {
	// Recursively delete all child scopes
	for (SymbolTable* Child : Children) {
		if (Child) {
			Child->deleteChildren(); // Recursively delete children's children first
			delete Child;
		}
	}
	Children.clear();
}

