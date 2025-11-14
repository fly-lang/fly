//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTNameSpace.cpp - AST Namespace implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTNameSpace.h"
#include "Basic/Logger.h"

#include "AST/ASTVisitor.h"

using namespace fly;

ASTNameSpace::ASTNameSpace(const SourceLocation &Loc, ASTIdentifier *Identifier) :
    ASTNode(Loc, ASTKind::AST_NAMESPACE), Identifier(Identifier) {
}

ASTNameSpace::~ASTNameSpace() = default;

void ASTNameSpace::accept(ASTVisitor &Visitor) {
	return Visitor.visit(*this);
}

ASTIdentifier *ASTNameSpace::getIdentifier() const {
	return Identifier;
}

std::string ASTNameSpace::str() const {
    return Logger("ASTNameSpace").
	Attr("Location", getLocation()).
	Attr("Kind", static_cast<size_t>(getKind())).
           End();
}
