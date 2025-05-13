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

const llvm::StringMap<SemaClassType *> &SemaClassType::getSuperClasses() const {
	return SuperClasses;
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

CodeGenClass *SemaClassType::getCodeGen() const {
	return CodeGen;
}

void SemaClassType::setCodeGen(CodeGenClass *CGC) {
	CodeGen = CGC;
}
