//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTCall.cpp - AST Function Call implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTCall.h"
#include "AST/ASTArg.h"
#include "Basic/Logger.h"

#include <AST/ASTVisitor.h>


using namespace fly;

ASTCall::ASTCall(const SourceLocation &Loc, llvm::StringRef Name, ASTCallKind CallKind) :
	ASTExpr(Loc, ASTExprKind::EXPR_CALL), CallKind(CallKind), Name(Name) {

}

void ASTCall::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

llvm::StringRef ASTCall::getName() const {
	return Name;
}

llvm::SmallVector<ASTArg *, 8> ASTCall::getArgs() const {
    return Args;
}

ASTCallKind ASTCall::getCallKind() const {
    return CallKind;
}

SemaCall *ASTCall::getSema() const {
	return Sema;
}

void ASTCall::setSema(SemaCall *Sema) {
	this->Sema = Sema;
}

std::string ASTCall::str() const {
    return Logger("ASTCall").
	Attr("Location", getLocation()).
		Attr("Kind", static_cast<size_t>(getKind())).
            Attr("Args", ASTNode::str(Args)).
            Attr("Sym", Sema).
            End();
}
