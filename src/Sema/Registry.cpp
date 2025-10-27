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

llvm::SmallVector<ASTBlockStmt *, 4> Registry::getBodies() {
	return Bodies;
}
