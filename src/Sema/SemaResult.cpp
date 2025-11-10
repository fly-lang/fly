//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaResult.cpp - The Symbolic Table for Var or Call
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaResult.h"

using namespace fly;

SemaResult::SemaResult(SemaKind Kind) : SemaNode(Kind) {
}

void SemaResult::setType(SemaType *Type) {
	this->Type = Type;
}

bool SemaResult::isCall() const {
	return Kind == SemaKind::CALL;
}

SemaResult *SemaResult::getParent() const {
	return Parent;
}

void SemaResult::setParent(SemaResult &Parent) {
	Parent.Child = this;
}

SemaResult * SemaResult::getChild() const {
	return Child;
}

SemaType *SemaResult::getType() const {
	return Type;
}
