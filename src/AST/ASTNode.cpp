//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTNode.cpp - AST Base implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTNode.h"
#include "Basic/Logger.h"

using namespace fly;

ASTNode::ASTNode(const SourceLocation &Loc, ASTKind Kind) : Location(Loc), Kind(Kind) {

}

const SourceLocation &ASTNode::getLocation() const {
    return Location;
}

ASTKind ASTNode::getKind() const {
	return Kind;
}

std::string ASTNode::str() const {
	return Logger()
	.Attr("Location", &Location)
	.Attr("Kind", static_cast<size_t>(Kind))
	.End();
}
