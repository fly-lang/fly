//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaClassInstance.cpp - The Symbolic Table for Class Attribute
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaClassInstance.h"
#include "CodeGen/CodeGenVar.h"

using namespace fly;

SemaClassInstance::SemaClassInstance(SemaClassType *Class) :
	SemaVar(nullptr, SemaVarKind::CLASS_INSTANCE), Class(Class) {
		Type = Class;
}

SemaClassInstance::~SemaClassInstance() {
	// Delete all base instances
	for (auto &Pair : BaseInstances) {
		delete Pair.second;
	}
}

SemaClassInstance *SemaClassInstance::getParent() const {
    return static_cast<SemaClassInstance *>(Parent);
}

llvm::StringRef SemaClassInstance::getName() const {
	return Name;
}

const llvm::DenseMap<size_t, SemaClassInstance *> &SemaClassInstance::getBaseInstances() const {
	return BaseInstances;
}

SemaClassInstance * SemaClassInstance::getBaseInstance(size_t TypeId) const {
	// Check if the TypeId exists in the current instance
	auto It = BaseInstances.find(TypeId);
	if (It != BaseInstances.end()) {
		return It->second;
	}

	// If not found, search in the base classes
	for (auto Entry : BaseInstances) {
		return Entry.second->getBaseInstance(TypeId);
	}

	// If still not found, return nullptr
	return nullptr;
}

SemaClassType * SemaClassInstance::getClass() const {
	return Class;
}

uint64_t SemaClassInstance::getIndex() {
	return Index;
}

CodeGenVarBase * SemaClassInstance::getCodeGen() const {
	return CodeGen;
}

void SemaClassInstance::setCodeGen(CodeGenVarBase *CodeGen) {
	this->CodeGen = static_cast<CodeGenVar *>(CodeGen);
}
