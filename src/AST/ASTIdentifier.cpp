//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTIdentifier.cpp - AST Identifier implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTIdentifier.h"
#include "Basic/Logger.h"

using namespace fly;

ASTIdentifier::ASTIdentifier(const SourceLocation &Loc, llvm::StringRef Name) :
		ASTExpr(Loc, ASTExprKind::EXPR_IDENTIFIER), Name(Name), Var(nullptr) {
}

ASTIdentifier::~ASTIdentifier() {
    delete Parent;
}

llvm::StringRef ASTIdentifier::getName() const {
    return Name;
}

ASTVar * ASTIdentifier::getVar() {
    return Var;
}

void ASTIdentifier::setSema(SemaVar *Sema) {
	this->Sema = Sema;
}

SemaVar *ASTIdentifier::getSema() const {
	return Sema;
}

std::string ASTIdentifier::str() const {
    return Logger("ASTIdentifier").
	Attr("Location", getLocation()).
Attr("Kind", static_cast<size_t>(getKind())).
            Attr("Name", Name).
            Attr("Child", Child).
            End();
}

