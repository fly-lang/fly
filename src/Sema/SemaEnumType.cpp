//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaEnumType.cpp - The Sema identity Symbols
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaEnumType.h"

#include "Sema/SemaVisitor.h"

#include "llvm/ADT/StringMap.h"

#include <AST/ASTEnum.h>
#include <Sema/SemaEnumValue.h>

using namespace fly;

SemaEnumType::SemaEnumType(ASTEnum &AST, SymbolTable *Symbols) : SemaType(SemaKind::ENUM ,SemaTypeKind::TYPE_ENUM, AST.getName().data()),
	AST(AST), Symbols(Symbols) {

}

SemaEnumType::~SemaEnumType() {
	// Delete all owned Nodes (Enum Entries)
	for (auto *Node : Nodes) {
		delete Node;
	}

	// Delete Comment if present
	delete Comment;
}

ASTEnum &SemaEnumType::getAST() {
    return AST;
}

SemaModule * SemaEnumType::getModule() const {
	return Module;
}

SymbolTable *SemaEnumType::getSymbols() const {
	return Symbols;
}

llvm::SmallVector<SemaNode *, 8> &SemaEnumType::getNodes() {
	return Nodes;
}

const llvm::StringMap<SemaEnumType *> &SemaEnumType::getBaseEnums() const {
	return SuperEnums;
}

SemaVisibilityKind SemaEnumType::getVisibility() const {
	return Visibility;
}

bool SemaEnumType::isConstant() const {
	return Constant;
}

const llvm::StringMap<SemaEnumValue *> &SemaEnumType::getEntries() const {
    return Entries;
}

SemaEnumValue * SemaEnumType::LookupEntry(llvm::StringRef Name) const {
	return Entries.lookup(Name);
}

void SemaEnumType::addEntry(SemaEnumValue *Value) {
	Nodes.push_back(Value);
}

SemaComment * SemaEnumType::getComment() const {
	return Comment;
}

bool SemaEnumType::isDerivedOrEquals(const SemaEnumType *BaseEnumType) const {
	if (this->isEquals(BaseEnumType)) {
		return true;
	}

	// Check if ClassType is a subclass of SuperClassType
	for (auto &Base : this->getBaseEnums()) {
		if (Base.getValue()->isEquals(BaseEnumType)) {
			return true;
		}
	}

	return false;
}

bool SemaEnumType::isDerived(const SemaEnumType *BaseEnumType) const {

	// Check if ClassType is a subclass of SuperClassType
	for (auto &Base : this->getBaseEnums()) {
		if (Base.getValue()->isEquals(BaseEnumType))
			return true;
	}

	return false;
}

bool SemaEnumType::isBaseOrEquals(const SemaEnumType *Derived) const {
	if (this->isEquals(Derived)) {
		return true;
	}

	// Check if this->ClassType is a derived of BaseClassType
	for (auto &Base : Derived->getBaseEnums()) {
		if (this->isBaseOrEquals(Base.getValue())) {
			return true;
		}
	}

	return false;
}

bool SemaEnumType::isBase(const SemaEnumType *Derived) const {
	// Check if this->ClassType is a derived of BaseClassType
	for (auto &Base : Derived->getBaseEnums()) {
		if (this->isEquals(Base.getValue())) {
			return true;
		}
	}

	return false;
}

void SemaEnumType::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

