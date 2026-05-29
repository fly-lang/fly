//===-------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTIdentifier.cpp - AST identifier expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTIdentifier.h"
#include "Sema/Symbol.h"
#include "Basic/Logger.h"
#include <AST/ASTVisitor.h>

using namespace fly;

ASTIdentifier::ASTIdentifier(const SourceLocation &Loc, llvm::StringRef Name) :
		ASTExpr(Loc, ASTExprKind::EXPR_IDENTIFIER), Name(Name) {
}

void ASTIdentifier::accept(ASTVisitor &Visitor) { Visitor.visit(*this); }

ASTIdentifier::~ASTIdentifier() {}

llvm::StringRef ASTIdentifier::getName() const { return Name; }

ASTVar *ASTIdentifier::getVar() { return Var; }

Symbol *ASTIdentifier::getSymbol() const { return ResolvedSymbol; }

void ASTIdentifier::setSymbol(Symbol *Sym) { ResolvedSymbol = Sym; }

std::string ASTIdentifier::str() const {
    return Logger("ASTIdentifier").
	Attr("Location", getLocation()).
	Attr("Kind", static_cast<size_t>(getKind())).
        Attr("Name", Name).
        Attr("Child", Child).
        End();
}
