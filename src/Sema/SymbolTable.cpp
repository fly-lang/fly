//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymbolTable.cpp - Sym table implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SymbolTable.h"

using namespace fly;

/**
 * Symbol table constructor
 */
SymbolTable::SymbolTable(SymbolTable *Parent) : Parent(Parent) {

}

/**
 * Symbol table destructor
 * Currently only clears the table structure
 *
 * TODO: Memory Management Issue
 * Symbol objects inserted into the table are currently NOT deleted here due to
 * a segfault that occurs when deleting them (possibly double-deletion somewhere).
 * This is a known memory leak that needs to be investigated and fixed.
 *
 * Child scopes created via pushScope() are tracked but not automatically deleted.
 * Call deleteChildren() before destruction if you want to recursively clean them up.
 */
SymbolTable::~SymbolTable() {
	// Clear the table structure (does not delete Symbol* values)
	Table.clear();
	// Note: Parent is not deleted - it's managed externally
	// Note: Children are not deleted - call deleteChildren() explicitly if needed
}

void SymbolTable::insert(Symbol *Sym) {
	Table[Sym->Name] = Sym;
}

size_t SymbolTable::size() const {
	return Table.size();
}


Symbol * SymbolTable::lookup(llvm::StringRef Name) {
	auto it = Table.find(Name);
	if (it != Table.end()) return it->second;
	if (Parent) return Parent->lookup(Name);
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

