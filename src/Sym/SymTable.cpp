//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymTable.cpp - Sym table implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sym/SymTable.h"

using namespace fly;

/**
 * Symbol table constructor
 * @param Diags
 */
SymTable::SymTable() = default;

/**
 * Symbol table destructor
 */
SymTable::~SymTable() = default;

llvm::StringMap<SymModule *> SymTable::getModules() const {
	return Modules;
}

SymNameSpace * SymTable::getDefaultNameSpace() const {
	return DefaultNameSpace;
}

llvm::StringMap<SymNameSpace *> SymTable::getNameSpaces() const {
	return NameSpaces;
}
