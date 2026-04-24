//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTBase.cpp - AST Base implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTBase.h"

#include <Basic/Logger.h>

using namespace fly;

ASTBase::ASTBase(const SourceLocation &Loc, ASTKind Kind) : Loc(Loc), Kind(Kind) {

}
const SourceLocation &ASTBase::getLocation() const {
	return Loc;
}

ASTKind ASTBase::getKind() const {
	return Kind;
}

void ASTBase::setKind(ASTKind Kind) {
	this->Kind = Kind;
}

std::string ASTBase::str() const {
	return Logger()
	.Attr("Location", &Loc)
	.Attr("Kind", static_cast<size_t>(Kind))
	.End();
}

