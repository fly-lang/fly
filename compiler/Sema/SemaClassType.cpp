//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaClassType.cpp - class type semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaClassType.h"

#include "AST/ASTVar.h"
#include "Basic/Logger.h"
#include "Sema/SemaType.h"
#include "Sema/SemaVisitor.h"
#include "Sema/SymbolTable.h"

#include "llvm/ADT/StringMap.h"

#include <Sema/SemaClassAttribute.h>
#include <Sema/SemaClassMethod.h>

using namespace fly;

SemaClassType::SemaClassType(ASTClass &AST, SemaModule &Module, SymbolTable *Symbols) :
	SemaType(SemaKind::TYPE_CLASS, AST.getName().data()), Module(Module),
	AST(AST), Symbols(Symbols), ClassKind(toClassKind(AST.getClassKind())) {

}

SemaClassType::~SemaClassType() {
	// Delete all owned Nodes (Attributes, Methods, Constructors)
	for (auto *Node : Nodes) {
		delete Node;
	}

	// Delete the 'this' instance
	delete This;

	// Delete Comment if present
	delete Comment;

	// CodeGen is deleted by parent SemaType destructor
}

SemaClassKind SemaClassType::toClassKind(ASTClassKind Kind) {
	switch (Kind) {

		case ASTClassKind::CLASS:
			return SemaClassKind::CLASS;

		case ASTClassKind::INTERFACE:
			return SemaClassKind::INTERFACE;

		case ASTClassKind::STRUCT:
			return SemaClassKind::STRUCT;

		case ASTClassKind::SUITE:
			return SemaClassKind::SUITE;
	}

	assert(false && "unknown ASTClassKind");
}

ASTClass &SemaClassType::getAST() {
    return AST;
}

SemaModule &SemaClassType::getModule() const {
	return Module;
}

SymbolTable *SemaClassType::getSymbols() const {
	return Symbols;
}

SemaComment * SemaClassType::getComment() const {
	return Comment;
}

llvm::SmallVector<SemaNode *, 8> & SemaClassType::getNodes() {
	return Nodes;
}

SemaVisibilityKind SemaClassType::getVisibility() const {
	return Visibility;
}

bool SemaClassType::isConstant() const {
	return Constant;
}

bool SemaClassType::isAbstract() const {
	return Abstract;
}

bool SemaClassType::isFinal() const {
	return Final;
}

SemaClassKind SemaClassType::getClassKind() const {
	return ClassKind;
}

SemaClassInstance *SemaClassType::getThis() const {
	return This;
}

const llvm::SmallVector<SemaClassType *, 4> &SemaClassType::getBaseClasses() const {
	return BaseClasses;
}

SemaClassMethod *SemaClassType::getDefaultConstructor() const {
	return DefaultConstructor;
}

void SemaClassType::setDefaultConstructor(SemaClassMethod *Constructor) {
	DefaultConstructor = Constructor;
}

SemaClassAttribute * SemaClassType::LookupAttribute(llvm::StringRef Name) const {
	// 1. Search in current class
	auto It = Attributes.find(Name);
	if (It != Attributes.end())
		return It->second;

	// 2. Search in base classes
	for (auto *Base : BaseClasses) {
		if (auto *A = Base->LookupAttribute(Name))
			return A;
	}

	return nullptr;
}

SemaClassMethod * SemaClassType::LookupMethod(llvm::StringRef Name) const {
	// 1. Search in current class
	auto It = Methods.find(Name);
	if (It != Methods.end())
		return It->second;

	// 2. Search in base classes
	for (auto *Base : BaseClasses) {
		if (auto *M = Base->LookupMethod(Name))
			return M;
	}

	return nullptr;
}

SemaClassMethod * SemaClassType::LookupConstructor(llvm::StringRef Name) const {
	// 1. Search in current class
	auto It = Constructors.find(Name);
	if (It != Constructors.end())
		return It->second;

	return nullptr;
}

const llvm::StringMap<SemaClassAttribute *> &SemaClassType::getAttributes() const {
    return Attributes;
}

const llvm::StringMap<SemaClassMethod *> &SemaClassType::getMethods() const {
    return Methods;
}

const llvm::StringMap<SemaClassMethod *> &SemaClassType::getConstructors() const {
	return Constructors;
}

void SemaClassType::addAttribute(SemaClassAttribute *Attribute) {
	Nodes.emplace_back(Attribute);
	Attributes.insert({Attribute->getAST()->getName(), Attribute});
}

void SemaClassType::addMethod(SemaClassMethod *Method) {
	Nodes.emplace_back(Method);
	if (Method->isConstructor()) {
		Constructors.insert({Method->getName(), Method});
	} else {
		Methods.insert({Method->getName(), Method});
	}
}

bool SemaClassType::isDerivedOrEquals(SemaClassType *BaseClassType) const {
	if (this->isEquals(BaseClassType)) {
		return true;
	}

	// Walk the FULL base chain, not just direct bases: a class is derived from any
	// ancestor (ASTMethod → ASTFunction → ASTNode → ASTBase), so an upcast to a base
	// several levels up is valid for assignment and argument compatibility.
	for (auto &Base : this->getBaseClasses()) {
		if (Base->isDerivedOrEquals(BaseClassType)) {
			return true;
		}
	}

	return false;
}

bool SemaClassType::isDerived(SemaClassType *BaseClassType) const {
	// Strictly derived (excludes equality): some base equals or is itself derived
	// from BaseClassType. Recurses the full chain (see isDerivedOrEquals).
	for (auto &Base : this->getBaseClasses()) {
		if (Base->isDerivedOrEquals(BaseClassType)) {
			return true;
		}
	}

	return false;
}

/**
 * Check if this->ClassType is a base class of ClassType
 * @param Derived
 * @return true if this ClassType is a base class of ClassType
 */
bool SemaClassType::isBaseOrEquals(SemaClassType *Derived) const {
	if (this->isEquals(Derived)) {
		return true;
	}

	// Check if ClassType is a base class of derived class
	for (auto &Base : Derived->getBaseClasses()) {
		if (this->isBaseOrEquals(Base)) {
			return true;
		}
	}

	return false;
}

bool SemaClassType::isBase(SemaClassType *Derived) const {
	// Check if ClassType is a base class of derived class
	for (auto &Base : Derived->getBaseClasses()) {
		if (this->isEquals(Base)) {
			return true;
		}
	}

	return false;
}

CodeGenClass *SemaClassType::getCodeGen() const {
	return CodeGen;
}

void SemaClassType::setCodeGen(CodeGenClass *CGC) {
	CodeGen = CGC;
}

void SemaClassType::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string SemaClassType::str() const {
	return Logger("SemaClassType")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Name", getName())
		.Attr("Visibility", static_cast<uint64_t>(getVisibility()))
		.Attr("Abstract", isAbstract())
		.Attr("Final", isFinal())
		.Attr("Constant", isConstant())
		.End();
}

const llvm::SmallVector<SemaTypeParam *, 4> &SemaClassType::getTypeParams() const {
	return TypeParams;
}

bool SemaClassType::isGeneric() const {
	return !TypeParams.empty();
}

SemaClassType *SemaClassType::getGenericTemplate() const {
	return GenericTemplate;
}

llvm::StringMap<SemaClassType *> &SemaClassType::getSpecializations() {
	return Specializations;
}

const std::string &SemaClassType::getMangledName() const {
	return MangledName;
}

