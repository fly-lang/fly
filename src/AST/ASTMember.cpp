//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTCastExpr.cpp - AST Cast Expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTMember.h"

#include <AST/ASTVisitor.h>

using namespace fly;

ASTMember::ASTMember(const SourceLocation &Loc, llvm::StringRef Name, ASTExpr *Parent) :
	ASTExpr(Loc, ASTExprKind::EXPR_MEMBER, Parent), Name(Name), Sema(nullptr) {
}

ASTMember::~ASTMember() {
}

void ASTMember::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

llvm::StringRef ASTMember::getName() const {
	return Name;
}

void ASTMember::setSema(SemaVar *Sema) {
	this->Sema = Sema;
}

SemaVar *ASTMember::getSema() const {
	return Sema;
}

std::string ASTMember::str() const {
	return ASTExpr::str();
}
