//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTNode.cpp - AST Base implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTNode.h"

using namespace fly;

ASTNode::ASTNode(const SourceLocation &Loc, ASTKind Kind) : ASTBase(Loc, Kind) {

}

bool ASTNode::isVisited() const {
	return Visited;
}

void ASTNode::setVisited(bool Visited) {
	this->Visited = Visited;
}