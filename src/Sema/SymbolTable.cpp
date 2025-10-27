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
 */
SymbolTable::~SymbolTable() = default;

void SymbolTable::insert(Symbol *Sym) {
	Table[Sym->Name] = Sym;
}

Symbol * SymbolTable::lookup(const std::string &Name) {
	auto it = Table.find(Name);
	if (it != Table.end()) return it->second;
	if (Parent) return Parent->lookup(Name);
	return nullptr;
}

SymbolTable * SymbolTable::pushScope() {
	return new SymbolTable(this);
}

SymbolTable * SymbolTable::getParent() {
	return Parent;
}
