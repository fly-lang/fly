//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaNode.cpp - The Sema Node
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "Sema/SemaNode.h"

using namespace fly;

SemaNode::SemaNode(SemaKind Kind) : Kind(Kind) {
}

SemaKind SemaNode::getKind() const {
	return Kind;
}