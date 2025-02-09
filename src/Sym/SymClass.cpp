//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymClass.cpp - The Sema identity Symbols
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sym/SymClass.h"
#include "llvm/ADT/StringMap.h"

using namespace fly;

SymClass::SymClass(ASTClass *AST) : SymType(SymTypeKind::TYPE_CLASS), AST(AST) {

}

ASTClass *SymClass::getAST() {
    return AST;
}

SymModule * SymClass::getModule() const {
	return Module;
}

SymVisibilityKind SymClass::getVisibility() const {
	return Visibility;
}

bool SymClass::isConstant() const {
	return Constant;
}

SymClassKind SymClass::getClassKind() const {
	return ClassKind;
}

const llvm::StringMap<SymClass *> &SymClass::getSuperClasses() const {
	return SuperClasses;
}

const llvm::StringMap<SymClassAttribute *> &SymClass::getAttributes() const {
    return Attributes;
}

const llvm::DenseMap<size_t, SymClassMethod *> &SymClass::getMethods() const {
    return Methods;
}

const llvm::DenseMap<size_t, SymClassMethod *> &SymClass::getConstructors() const {
	return Constructors;
}

SymComment * SymClass::getComment() const {
	return Comment;
}

CodeGenClass *SymClass::getCodeGen() const {
	return CodeGen;
}

void SymClass::setCodeGen(CodeGenClass *CGC) {
	CodeGen = CGC;
}
