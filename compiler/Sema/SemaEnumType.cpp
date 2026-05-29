//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaEnumType.cpp - enum type semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaEnumType.h"

#include "Basic/Logger.h"
#include "Sema/SemaVisitor.h"

#include "llvm/ADT/StringMap.h"

#include <AST/ASTEnum.h>
#include <Sema/SemaEnumEntry.h>

using namespace fly;

SemaEnumType::SemaEnumType(ASTEnum &AST, SymbolTable *Symbols) : SemaType(SemaKind::TYPE_ENUM, AST.getName().data()),
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

const llvm::StringMap<SemaEnumEntry *> &SemaEnumType::getEntries() const {
    return Entries;
}

SemaEnumEntry * SemaEnumType::LookupEntry(llvm::StringRef Name) const {
	return Entries.lookup(Name);
}

void SemaEnumType::addEntry(SemaEnumEntry *Entry) {
	Nodes.push_back(Entry);
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

std::string SemaEnumType::str() const {
	std::string Entries = Logger::OPEN_LIST;
	bool first = true;
	for (auto &KV : getEntries()) {
		if (!first) Entries += Logger::SEP;
		Entries += KV.getKey().str();
		first = false;
	}
	Entries += Logger::CLOSE_LIST;
	return Logger("SemaEnumType")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Name", getName())
		.Attr("Visibility", static_cast<uint64_t>(getVisibility()))
		.Attr("Constant", isConstant())
		.Attr("Entries", Entries)
		.End();
}

