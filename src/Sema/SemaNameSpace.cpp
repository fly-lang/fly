//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/SemaNameSpace.cpp - AST Namespace implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaNameSpace.h"
#include "llvm/ADT/StringRef.h"

#include <AST/ASTNameSpace.h>
#include <Sema/SymbolTable.h>

using namespace fly;

SemaNameSpace::SemaNameSpace(llvm::StringRef Name, SemaNameSpace *Parent) : SemaNode(SemaKind::NAMESPACE),
	Name(Name), Parent(Parent), Symbols(new SymbolTable(Parent ? Parent->getSymbols() : nullptr)) {
}

SymbolTable * SemaNameSpace::getSymbols() const {
	return Symbols;
}

SemaNameSpace::~SemaNameSpace() {
	// Delete all child namespaces
	for (auto &Pair : Children) {
		delete Pair.second;
	}

	// Delete Symbols
	delete Symbols;
}

llvm::StringRef SemaNameSpace::getName() const {
	return Name;
}

SemaNameSpace * SemaNameSpace::getParent() const {
	return Parent;
}

const llvm::StringMap<SemaNameSpace *> &SemaNameSpace::getChildren() const {
	return Children;
}

const llvm::SmallVector<ASTModule *, 8> &SemaNameSpace::getModules() const {
	return Modules;
}

const llvm::StringMap<SemaFunction *> &SemaNameSpace::getFunctions() const {
	return Functions;
}

const llvm::StringMap<SemaType *> &SemaNameSpace::getTypes() const {
	return Types;
}
