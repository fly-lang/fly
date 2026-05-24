//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaClassInstance.cpp - class instance semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaClassInstance.h"

#include "Basic/Logger.h"
#include "CodeGen/CodeGenVar.h"
#include "Sema/SemaClassType.h"
#include "Sema/SemaVisitor.h"

using namespace fly;

SemaClassInstance::SemaClassInstance(SemaClassType *Class) :
	SemaVar(nullptr, SemaKind::INSTANCE_VAR, Class), Class(Class) {
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

void SemaClassInstance::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string SemaClassInstance::str() const {
	return Logger("SemaClassInstance")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Name", getName())
		.Attr("Index", const_cast<SemaClassInstance*>(this)->getIndex())
		.Attr("Type", Type)
		.End();
}


