//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaNode.cpp - AST node semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "Sema/SemaNode.h"
#include "Basic/Logger.h"

using namespace fly;

SemaNode::SemaNode(SemaKind Kind) : Kind(Kind) {
}

SemaKind SemaNode::getKind() const {
	return Kind;
}

std::string SemaNode::str() const {
	return Logger("SemaNode")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.End();
}