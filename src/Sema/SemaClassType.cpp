//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaClassType.cpp - The Sema identity Symbols
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaClassType.h"
#include "llvm/ADT/StringMap.h"

using namespace fly;

SemaClassType::SemaClassType(ASTClass *AST) : SemaType(SemaTypeKind::TYPE_CLASS, AST->getName().data()),
	AST(AST), ClassKind(toClassKind(AST->getClassKind())) {

}

SemaClassKind SemaClassType::toClassKind(ASTClassKind Kind) {
	switch (Kind) {

		case ASTClassKind::CLASS:
			return SemaClassKind::CLASS;
		case ASTClassKind::INTERFACE:
			return SemaClassKind::INTERFACE;
		case ASTClassKind::STRUCT:
			return SemaClassKind::STRUCT;
	}

	assert(false && "unknown ASTClassKind");
}

ASTClass *SemaClassType::getAST() {
    return AST;
}

SemaModule * SemaClassType::getModule() const {
	return Module;
}

SemaVisibilityKind SemaClassType::getVisibility() const {
	return Visibility;
}

bool SemaClassType::isConstant() const {
	return Constant;
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

const llvm::StringMap<SemaClassAttribute *> &SemaClassType::getAttributes() const {
    return Attributes;
}

const llvm::StringMap<SemaClassMethod *> &SemaClassType::getMethods() const {
    return Methods;
}

const llvm::StringMap<SemaClassMethod *> &SemaClassType::getConstructors() const {
	return Constructors;
}

SemaComment * SemaClassType::getComment() const {
	return Comment;
}

bool SemaClassType::isDerivedOrEquals(const SemaClassType *BaseClassType) const {
	if (this->isEquals(BaseClassType)) {
		return true;
	}

	// Check if this->ClassType is a derived of BaseClassType
	for (auto &Base : this->getBaseClasses()) {
		if (Base->isEquals(BaseClassType)) {
			return true;
		}
	}

	return false;
}

bool SemaClassType::isDerived(const SemaClassType *BaseClassType) const {
	// Check if this->ClassType is a derived of BaseClassType
	for (auto &Base : this->getBaseClasses()) {
		if (Base->isEquals(BaseClassType)) {
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
bool SemaClassType::isBaseOrEquals(const SemaClassType *Derived) const {
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

bool SemaClassType::isBase(const SemaClassType *Derived) const {
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
