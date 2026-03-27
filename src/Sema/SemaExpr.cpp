//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaExpr.cpp - The Symbolic Table for Var or Call
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaExpr.h"

using namespace fly;

SemaExpr::SemaExpr(SemaKind Kind, SemaType *Type) : SemaNode(Kind), Type(Type) {
}

SemaExpr::~SemaExpr() {
	// Delete CodeGen if present
	delete CodeGen;
}

SemaExpr *SemaExpr::getParent() const {
	return Parent;
}

void SemaExpr::setParent(SemaExpr &Parent) {
	this->Parent = &Parent;
	Parent.Child = this;
}

SemaExpr * SemaExpr::getChild() const {
	return Child;
}

SemaType *SemaExpr::getType() const {
	return Type;
}

void SemaExpr::setType(SemaType *Type) {
	this->Type = Type;
}

CodeGenExpr * SemaExpr::getCodeGen() const {
	return CodeGen;
}

void SemaExpr::setCodeGen(CodeGenExpr *CG) {
	this->CodeGen = CG;
}

