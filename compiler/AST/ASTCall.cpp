//===--------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTCall.cpp - AST function call expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTCall.h"
#include "AST/ASTArg.h"
#include "Sema/Symbol.h"
#include "Basic/Logger.h"
#include <AST/ASTVisitor.h>

using namespace fly;

ASTCall::ASTCall(const SourceLocation &Loc, llvm::StringRef Name, ASTCallKind CallKind) :
	ASTExpr(Loc, ASTExprKind::EXPR_CALL), CallKind(CallKind), Name(Name) {
}

void ASTCall::accept(ASTVisitor &Visitor) { Visitor.visit(*this); }

llvm::StringRef ASTCall::getName() const { return Name; }

llvm::SmallVector<ASTArg *, 8> ASTCall::getArgs() const { return Args; }

ASTCallKind ASTCall::getCallKind() const { return CallKind; }

Symbol *ASTCall::getSymbol() const { return ResolvedSymbol; }

void ASTCall::setSymbol(Symbol *Sym) { ResolvedSymbol = Sym; }

std::string ASTCall::str() const {
    return Logger("ASTCall")
        .Attr("Location", getLocation())
        .Attr("Kind", static_cast<size_t>(getKind()))
        .Attr("CallKind", static_cast<uint64_t>(CallKind))
        .Attr("Name", Name)
        .Attr("Symbol", ResolvedSymbol ? ResolvedSymbol->getName() : std::string("null"))
        .Attr("Args", Args)
        .End();
}
