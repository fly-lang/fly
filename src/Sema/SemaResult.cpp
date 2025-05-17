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

SemaResult::SemaResult(SemaResultKind Kind) : Kind(Kind) {
}

SemaResultKind SemaResult::getKind() const {
	return Kind;
}

SemaResult *SemaResult::getParent() const {
	return Parent;
}
