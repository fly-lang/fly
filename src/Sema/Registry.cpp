//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/Registry.cpp - The Namespace Registry
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "Sema/Registry.h"
#include "Sema/SemaNameSpace.h"

#include <AST/ASTNameSpace.h>

using namespace fly;

std::string Registry::DEFAULT_NAMESPACE = "default";

Registry::Registry() : DefaultNameSpace(new SemaNameSpace(DEFAULT_NAMESPACE)) {
	NameSpaces.insert(std::make_pair<>(DefaultNameSpace->getName(), DefaultNameSpace));
}

void Registry::addModule(SemaModule *Module) {
	Modules.push_back(Module);
}

llvm::SmallVector<SemaModule *, 8> &Registry::getModules() {
	return Modules;
}

SemaNameSpace * Registry::getDefaultNameSpace() {
	return DefaultNameSpace;
}

SemaNameSpace * Registry::getOrAddFQNameSpace(const std::string &Name, SemaNameSpace *Parent) {
	// Create the CurrentNameSpace if not exists yet in the Context
	auto I = NameSpaces.find(Name);
	if (I != NameSpaces.end()) {
		return I->second;
	} else {
		auto NameSpace = new SemaNameSpace(Name, Parent);
		NameSpaces.insert(std::make_pair<>(NameSpace->getName(), NameSpace));
		return NameSpace;
	}
}

SemaNameSpace * Registry::getFQNameSpace(const std::string &FQName) {
	auto I = NameSpaces.find(FQName);
	if (I != NameSpaces.end()) {
		return I->second;
	}
	return nullptr;
}

llvm::SmallVector<LocalScope, 4> Registry::getBodies() const {
	return Bodies;
}

void Registry::addBody(SymbolTable* Symbols, ASTBlockStmt *Body) {
	Bodies.push_back(LocalScope{Symbols, Body});
}

SemaNameSpace* Registry::getOrAddNameSpace(const ASTNameSpace& AST) {
	SemaNameSpace *NameSpace = nullptr;
	std::string FQName = "";
	for (auto It = AST.getNames().begin(); It != AST.getNames().end(); ++It) {
		// Generate the full name
		FQName += (It == AST.getNames().begin()) ? std::string(*It) : "." + std::string(*It);

		// Add as Parent the previous NameSpace
		NameSpace = getOrAddFQNameSpace(FQName, NameSpace);
	}
	return NameSpace;
}
